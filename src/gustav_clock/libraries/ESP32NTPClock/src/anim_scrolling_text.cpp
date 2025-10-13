#include "anim_scrolling_text.h"
#include <Arduino.h>

ScrollingTextAnimation::ScrollingTextAnimation(std::string text, unsigned long scrollDelay, bool dotsWithPreviousChar)
    : _text(text),
      _scrollDelay(scrollDelay),
      _lastScrollTime(0),
      _currentPosition(0),
      _dotsWithPreviousChar(dotsWithPreviousChar) {}

void ScrollingTextAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);

    // Use the robust parser from the base class
    parseTextAndDots(_text, _dotsWithPreviousChar, _parsedText, _dotStates);
    
    // Start the text completely off-screen to the right
    _currentPosition = -_display->getDisplaySize();
}

bool ScrollingTextAnimation::isDone() {
    // The animation is done when the START of the text has scrolled
    // completely past the LEFT edge of the display.
    return _currentPosition > (int)_parsedText.length();
}

void ScrollingTextAnimation::update() {
    if (isDone()) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastScrollTime >= _scrollDelay) {
        _lastScrollTime = currentTime;
        
        int displaySize = _display->getDisplaySize();

        for (int i = 0; i < displaySize; ++i) {
            int textIndex = _currentPosition + i;
            // Check if the calculated index is within the bounds of the parsed text
            if (textIndex >= 0 && textIndex < (int)_parsedText.length()) {
                _display->setChar(i, _parsedText[textIndex], _dotStates[textIndex]);
            } else {
                // If it's outside the bounds, draw a blank space.
                _display->setChar(i, ' ', false);
            }
        }
        _currentPosition++;
    }
}
