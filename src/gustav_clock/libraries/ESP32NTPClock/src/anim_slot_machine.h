#ifndef ANIM_SLOT_MACHINE_H
#define ANIM_SLOT_MACHINE_H

#include "i_animation.h"
#include <string>
#include <vector>

class SlotMachineAnimation : public IAnimation {
public:
    SlotMachineAnimation(std::string targetText, 
                         unsigned long lockDelay = 200, 
                         unsigned long holdTime = 3000, 
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
    
    // Use a standard, safe vector instead of a raw pointer to prevent memory corruption.
    // Using uint8_t is more robust on embedded systems than the specialized vector<bool>.
    std::vector<uint8_t> _isLocked;

    unsigned long _lockDelay;
    unsigned long _holdTime;
    unsigned long _spinDelay;
    bool _dotsWithPreviousChar;

    unsigned long _lastLockTime;
    unsigned long _lastSpinTime;
    int _lockedCount;
    unsigned long _lockingCompleteTime;
    bool _finalFrameDrawn;
};

#endif // ANIM_SLOT_MACHINE_H
