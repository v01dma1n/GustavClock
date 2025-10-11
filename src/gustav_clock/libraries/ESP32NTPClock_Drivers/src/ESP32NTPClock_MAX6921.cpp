#include "ESP32NTPClock_MAX6921.h"
#include "enc_debug.h"

// --- Pin and Segment mapping from your disp_phys.h ---
// Grids (Digits)
static const unsigned long GRIDS[] = {
    0b00000000010000000000, // GRD_01
    0b00000000100000000000, // GRD_02
    0b00000010000000000000, // GRD_03
    0b00010000000000000000, // GRD_04
    0b10000000000000000000, // GRD_05
    0b00000000000000000010, // GRD_06
    0b00000000000000000100, // GRD_07
    0b00000000000000100000, // GRD_08
    0b00000000000100000000, // GRD_09
    0b00000000001000000000  // GRD_10
};

//   --     a  
//  |  |  f   b
//   --     g
//  |  |  e   c
//   --     d
//                                       1111111111                              
//                                       98765432109876543210
// Segment                               -d,-ef-g----ab-c.---
static const unsigned long SEG_A     = 0b00000000000010000000;
static const unsigned long SEG_B     = 0b00000000000001000000;
static const unsigned long SEG_C     = 0b00000000000000010000;
static const unsigned long SEG_D     = 0b01000000000000000000;
static const unsigned long SEG_E     = 0b00001000000000000000;
static const unsigned long SEG_F     = 0b00000100000000000000;
static const unsigned long SEG_G     = 0b00000001000000000000;
static const unsigned long SEG_DOT   = 0b00000000000000001000;
static const unsigned long SEG_COMMA = 0b00100000000000000000;

// This table maps ASCII characters (from ' ' to 'Z') to the VFD segment masks.
static const unsigned long VFD_FONT_MAP[] = {
    0,                                                                  // ' ' (space)
    0,                                                                  // !
    SEG_F | SEG_B,                                  // "
    0,                                              // #
    0,                                              // $
    0,                                              // %
    0,                                              // &
    SEG_A,                                          // '
    0,                                              // (
    0,                                              // )
    0,                                              // *
    0,                                              // +
    0,                                              // ,
    SEG_G,                                         // -
    0,                                             // . (handled by dot boolean)
    0,                                                                 // /
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
    SEG_B | SEG_C,                                              // 1
    SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,            // 2
    SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,            // 3
    SEG_F | SEG_G | SEG_B | SEG_C,                        // 4
    SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,            // 5
    SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G, // 6
    SEG_A | SEG_B | SEG_C,                                    // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 8
    SEG_A | SEG_F | SEG_G | SEG_B | SEG_C | SEG_D, // 9
    0,                                                                  // : (colon)
    0,                                                                  // ;
    0,                                                                  // <
    0,                                                                  // =
    0,                                                                  // >
    SEG_A | SEG_B | SEG_G | SEG_E,                        // ?
    0,                                                                  // @
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,            // B (lowercase b)
    SEG_A | SEG_D | SEG_E | SEG_F,                        // C
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,            // D (lowercase d)
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
    SEG_A | SEG_E | SEG_F | SEG_G,                        // F
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F,            // G
    SEG_C | SEG_E | SEG_F | SEG_G,                        // H (lowercase h)
    SEG_E | SEG_F,                                              // I
    SEG_B | SEG_C | SEG_D | SEG_E,                        // J
    0,                                                                  // K
    SEG_D | SEG_E | SEG_F,                                    // L
    0,                                                                  // M
    SEG_C | SEG_E | SEG_G,                                    // N (lowercase n)
    SEG_C | SEG_D | SEG_E | SEG_G,                        // O (lowercase o)
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,            // P
    SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,            // Q
    SEG_E | SEG_G,                                              // R (lowercase r)
    SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,            // S
    SEG_D | SEG_E | SEG_F | SEG_G,                        // T (lowercase t)
    SEG_C | SEG_D | SEG_E,                                    // U (lowercase u)
    0,                                                                  // V
    0,                                                                  // W
    0,                                                                  // X
    SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,            // Y
    0                                                                   // Z
};

/*
// Font map
static const unsigned long FONT_MAP[] = {
    SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,       // 0
    SEG_B|SEG_C,                               // 1
    SEG_A|SEG_B|SEG_G|SEG_E|SEG_D,             // 2
    SEG_A|SEG_B|SEG_G|SEG_C|SEG_D,             // 3
    SEG_F|SEG_G|SEG_B|SEG_C,                   // 4
    SEG_A|SEG_F|SEG_G|SEG_C|SEG_D,             // 5
    SEG_A|SEG_F|SEG_E|SEG_D|SEG_C|SEG_G,       // 6
    SEG_A|SEG_B|SEG_C,                         // 7
    SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, // 8
    SEG_A|SEG_F|SEG_G|SEG_B|SEG_C|SEG_D,       // 9
};
*/
// --- Class Implementation ---

DispDriverMAX6921::DispDriverMAX6921(int displaySize, int sclkPin, int misoPin, int mosiPin, int ssPin, int blankPin)
    : _displaySize(displaySize), _blankPin(blankPin), _ssPin(ssPin), _currentDigit(0),
      _multiplexStep(0), _lastMultiplexTime(0) {
    _displayBuffer = new unsigned long[_displaySize](); // Allocate and zero-initialize buffer
    _spi = new SPIClass(VSPI);
    _spi->begin(sclkPin, misoPin, mosiPin, ssPin);
}

void DispDriverMAX6921::begin() {
    pinMode(_ssPin, OUTPUT);
    pinMode(_blankPin, OUTPUT);
    digitalWrite(_blankPin, LOW); // Blank the display initially
}

int DispDriverMAX6921::getDisplaySize() {
    return _displaySize;
}

void DispDriverMAX6921::setBrightness(uint8_t level) {
    // MAX6921 doesn't have brightness control, but we must implement the function.
}

void DispDriverMAX6921::clear() {
    for (int i = 0; i < _displaySize; i++) {
        _displayBuffer[i] = 0;
    }
}

unsigned long DispDriverMAX6921::mapAsciiToSegment(char ascii_char) {
    char c = toupper(ascii_char);
    // The font map starts at ASCII 32 (' ').
    if (c >= ' ' && c <= 'Z') {
        return VFD_FONT_MAP[c - ' '];
    }
    return 0; // Return blank for any character not in the font map.
}


void DispDriverMAX6921::setChar(int position, char character, bool dot) {
    if (position < 0 || position >= _displaySize) return;
    unsigned long segments = mapAsciiToSegment(character);
    if (dot) {
        segments |= SEG_DOT; 
    }
    _displayBuffer[position] = segments;
}

/*
unsigned long DispDriverMAX6921::mapAsciiToSegment(char ascii_char) {
    if (ascii_char >= '0' && ascii_char <= '9') {
        return FONT_MAP[ascii_char - '0'];
    }
    // Add more character mappings here if needed (e.g., A-F for hex)
    switch (toupper(ascii_char)) {
        case '-': return SEG_G;
        case ' ': return 0;
        default: return 0; // Blank for unknown characters
    }
}

void DispDriverMAX6921::setChar(int position, char character, bool dot) {
   
    if (position < 0 || position >= _displaySize) return;
    
    unsigned long segments = mapAsciiToSegment(character);
    if (dot) {
        segments |= SEG_DOT;
    }
    _displayBuffer[position] = segments;
}
*/
void DispDriverMAX6921::setSegments(int position, uint16_t mask) {
    // This function is less relevant for this driver, but we implement it.
    // It maps the standard 7-segment bits to the MAX6921 bits.
    if (position < 0 || position >= _displaySize) return;
    unsigned long vfd_mask = 0;
    if (mask & 0b00000001) vfd_mask |= SEG_A;
    if (mask & 0b00000010) vfd_mask |= SEG_B;
    if (mask & 0b00000100) vfd_mask |= SEG_C;
    if (mask & 0b00001000) vfd_mask |= SEG_D;
    if (mask & 0b00010000) vfd_mask |= SEG_E;
    if (mask & 0b00100000) vfd_mask |= SEG_F;
    if (mask & 0b01000000) vfd_mask |= SEG_G;
    if (mask & 0b10000000) vfd_mask |= SEG_DOT;
    _displayBuffer[position] = vfd_mask;
}

void DispDriverMAX6921::spiCmd(unsigned long data) {
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    digitalWrite(_ssPin, LOW);
    _spi->transfer32(data); // MAX6921 is 20-bit, but sending 32 is fine
    digitalWrite(_ssPin, HIGH);

    _spi->endTransaction();
}

void DispDriverMAX6921::writeDisplay() {
    // This function is called continuously. We use micros() to ensure
    // a consistent 0.5ms interval between steps.
    unsigned long currentTime = micros();
    if ((currentTime - _lastMultiplexTime) < 500) {
        return; // run this every 0.5 millisecond 
    }
    _lastMultiplexTime = currentTime;

    if (_multiplexStep == 0) { // Step 0: Send data while blanked
        digitalWrite(_blankPin, HIGH); // Blank the display
        spiCmd(GRIDS[_currentDigit] | _displayBuffer[_currentDigit]);

        _currentDigit++;
        if (_currentDigit >= _displaySize) {
            _currentDigit = 0;
        }
        _multiplexStep = 1; // Move to the next step

    } else { // Step 1: Un-blank the display to show the digit
        digitalWrite(_blankPin, LOW); // Un-blank the display
        _multiplexStep = 0; // Go back to the send step for the next digit
    }
}

bool DispDriverMAX6921::needsContinuousUpdate() const {
    return true;
}