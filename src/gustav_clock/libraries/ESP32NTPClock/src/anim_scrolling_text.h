#ifndef ANIM_SCROLLING_TEXT_H
#define ANIM_SCROLLING_TEXT_H

#include "i_animation.h"
#include <string>
#include <vector>

class ScrollingTextAnimation : public IAnimation {
public:
    ScrollingTextAnimation(std::string text, unsigned long scrollDelay = 250, bool dotsWithPreviousChar = false);
   
    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _text;
    unsigned long _scrollDelay;
    unsigned long _lastScrollTime;
    int _currentPosition;

    bool _dotsWithPreviousChar;  
      
    std::string _parsedText;
    std::vector<uint8_t> _dotStates;    
};

#endif // ANIM_SCROLLING_TEXT_H

