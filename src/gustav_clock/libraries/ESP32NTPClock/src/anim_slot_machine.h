#ifndef ANIM_SLOT_MACHINE_H
#define ANIM_SLOT_MACHINE_H

#include "i_animation.h"
#include "fast_random.h"
#include <string>
#include <vector>
#include <list>

class SlotMachineAnimation : public IAnimation {
public:
    SlotMachineAnimation(std::string targetText, 
                         unsigned long lockDelay = 200, 
                         unsigned long spinDelay = 50,
                         bool dotsWithPreviousChar = false);
    ~SlotMachineAnimation();
    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    std::string _parsedText;
    std::vector<uint8_t> _dotStates;
    
    // std::vector<uint8_t> _isLocked;
    // it looks like the std::vector did not beahve so it is replaced with std::list
    std::list<int> _lockedIndices;

    unsigned long _lockDelay;
    unsigned long _spinDelay;
    bool _dotsWithPreviousChar;

    unsigned long _lastLockTime;
    unsigned long _lastSpinTime;
    int _lockedCount;
    bool _finalFrameDrawn;

    FastRandom _rng;
};

#endif // ANIM_SLOT_MACHINE_H
