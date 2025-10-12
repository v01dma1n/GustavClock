#ifndef I_DISP_DRIVER_H
#define I_DISP_DRIVER_H

#include <Arduino.h> 
#include <vector>

class IDisplayDriver {
public:
    virtual ~IDisplayDriver() {}

    virtual void begin() = 0;
    virtual int getDisplaySize() = 0;
    virtual void setBrightness(uint8_t level) = 0;
    virtual void clear() = 0;
    virtual void setChar(int position, char character, bool dot = false) = 0;
    virtual void setSegments(int position, uint16_t mask) = 0;
    
    virtual unsigned long mapAsciiToSegment(char ascii_char, bool dot) = 0;
    virtual void setBuffer(const std::vector<unsigned long>& newBuffer) = 0;

    void print(const char* text, bool dotsWithPreviousChar = false); 
    virtual void writeDisplay() = 0;
    virtual bool needsContinuousUpdate() const { return false; }
};

#endif // I_DISP_DRIVER_H

