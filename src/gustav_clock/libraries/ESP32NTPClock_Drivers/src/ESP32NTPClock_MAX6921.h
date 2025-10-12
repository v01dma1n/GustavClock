#ifndef ESP32NTPCLOCK_MAX6921_H
#define ESP32NTPCLOCK_MAX6921_H

#include <ESP32NTPClock.h>
#include <SPI.h>

class DispDriverMAX6921 : public IDisplayDriver {
public:
    DispDriverMAX6921(int displaySize, int sclkPin, int misoPin, int mosiPin, int ssPin, int blankPin);
    ~DispDriverMAX6921(); 

    // IDisplayDriver interface implementation
    void begin() override;
    int getDisplaySize() override;
    void setBrightness(uint8_t level) override;
    void clear() override;
    void setChar(int position, char character, bool dot = false) override;
    void setSegments(int position, uint16_t mask) override;
    void writeDisplay() override;
    bool needsContinuousUpdate() const override;

    unsigned long mapAsciiToSegment(char ascii_char, bool dot) override;
    void setBuffer(const std::vector<unsigned long>& newBuffer) override;

private:
    void spiCmd(unsigned long data);

    int _displaySize;
    int _blankPin;
    int _ssPin;
    unsigned long* _displayBuffer; // Dynamically allocated buffer
    
    SPIClass* _spi;
    int _currentDigit;
};

#endif // ESP32NTPCLOCK_MAX6921_H
