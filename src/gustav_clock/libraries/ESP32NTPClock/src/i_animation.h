#ifndef I_ANIM_H
#define I_ANIM_H

#include "i_display_driver.h"
#include <string>
#include <vector>

class IAnimation {
public:
    virtual ~IAnimation() {}

    virtual void setup(IDisplayDriver* display) {
        _display = display;
        // Allocate the private buffer to match the display size
        _buffer.assign(_display->getDisplaySize(), 0); 
    }

    virtual void update() = 0;
    virtual bool isDone() = 0;

    // New function to get the completed frame from the animation
    const std::vector<unsigned long>& getFrame() const {
        return _buffer;
    }

protected:
    IDisplayDriver* _display;
    std::vector<unsigned long> _buffer; // The animation's private buffer

    // This now writes to the animation's private buffer, not the display driver
    void setChar(int position, char character, bool dot = false) {
        if (position < 0 || position >= _buffer.size()) return;
        _buffer[position] = _display->mapAsciiToSegment(character, dot);
    }

    void parseTextAndDots(const std::string& inputText, bool dotsWithPrevious,
                          std::string& outParsedText, std::vector<uint8_t>& outDotStates) {
        outParsedText.clear();
        outDotStates.clear();

        for (char c : inputText) {
            if (c == '.' && dotsWithPrevious) {
                // If a dot is found and we have text to attach it to,
                // set the dot flag for the *previous* character.
                if (!outDotStates.empty()) {
                    outDotStates.back() = 1; // 1 means true
                }
            } else {
                // This is a regular character, add it to the text
                // and add a corresponding 'false' dot state.
                outParsedText += c;
                outDotStates.push_back(0); // 0 means false
            }
        }
    }
};

#endif // I_ANIM_H
