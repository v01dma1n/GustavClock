#include "anim_slot_machine.h"
#include <Arduino.h>

SlotMachineAnimation::SlotMachineAnimation(std::string targetText,
                                           unsigned long lockDelay,
                                           unsigned long holdTime,
                                           unsigned long spinDelay,
                                           bool dotsWithPreviousChar)
    : _targetText(targetText),
      _lockDelay(lockDelay),
      _holdTime(holdTime),
      _spinDelay(spinDelay),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _lastLockTime(0),
      _lastSpinTime(0),
      _lockedCount(0),
      _lockingCompleteTime(0),
      _finalFrameDrawn(false)
{}

// Destructor is empty because std::vector handles its own memory.
SlotMachineAnimation::~SlotMachineAnimation() {}

void SlotMachineAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    
    parseTextAndDots(_targetText, _dotsWithPreviousChar, _parsedText, _dotStates);

    // Initialize the vector to the correct size, with all values as 0 (false).
    _isLocked.assign(_display->getDisplaySize(), 0);

    // Reset all state variables to ensure the animation can be run multiple times
    _lastLockTime = millis();
    _lastSpinTime = millis();
    _lockedCount = 0;
    _lockingCompleteTime = 0;
    _finalFrameDrawn = false;
    randomSeed(analogRead(0));

    int displaySize = _display->getDisplaySize();
    for (int i = 0; i < displaySize; ++i) {
        // All digits start as "spinning" (random numbers)
        _display->setChar(i, random(0, 10) + '0', false);
    }
}

bool SlotMachineAnimation::isDone() {
    bool lockingComplete = _lockedCount >= _display->getDisplaySize();
    if (!lockingComplete) {
        return false;
    }
    return (millis() - _lockingCompleteTime >= _holdTime);
}

void SlotMachineAnimation::update() {
    if (_finalFrameDrawn) {
        return;
    }

    if (isDone()) {
        // Draw the final, correct text one last time before exiting.
        for (int i = 0; i < _display->getDisplaySize(); ++i) {
            _display->setChar(i, _parsedText[i], _dotStates[i]);
        }
        _finalFrameDrawn = true;
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastSpinTime < _spinDelay) {
        return;
    }
    _lastSpinTime = currentTime;

    // --- State Update Logic ---
    bool isLockingPhase = _lockedCount < _display->getDisplaySize();
    if (isLockingPhase && (currentTime - _lastLockTime >= _lockDelay)) {
        _lastLockTime = currentTime;
        if (_lockedCount < _isLocked.size()) {
            _isLocked[_lockedCount] = 1; // Lock the next digit by setting to 1 (true)
            _lockedCount++;
        }

        if (_lockedCount == _display->getDisplaySize()) {
            _lockingCompleteTime = millis();
        }
    }

    // --- Atomic Drawing Logic (redraw every digit, every frame) ---
    int displaySize = _display->getDisplaySize();
    for (int i = 0; i < displaySize; ++i) {
        if (i < _isLocked.size() && _isLocked[i]) { // C++ treats 0 as false and non-zero as true
            _display->setChar(i, _parsedText[i], _dotStates[i]);
        } else {
            _display->setChar(i, random(0, 10) + '0', false);
        }
    }
}

