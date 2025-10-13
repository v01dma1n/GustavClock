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
      _finalFrameDrawn(false),
      _rng(millis())
{}

SlotMachineAnimation::~SlotMachineAnimation() {}

void SlotMachineAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    
    _parsedText = _targetText;
    _dotStates.assign(_targetText.length(), 0); 
    
    // This state vector is no longer used by the simplified update()
    _isLocked.assign(_display->getDisplaySize(), 0); 
    
    _lastLockTime = millis();
    _lastSpinTime = millis();
    _lockedCount = 0;
    _lockingCompleteTime = 0;
    _finalFrameDrawn = false;
    
    // The initial draw of spinning numbers remains the same.
    int displaySize = _display->getDisplaySize();
    for (int i = 0; i < displaySize; ++i) {
        _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
    }
}

bool SlotMachineAnimation::isDone() {
    // This logic is identical to MatrixAnimation and is known to be stable.
    bool lockingComplete = _lockedCount >= _display->getDisplaySize();
    if (!lockingComplete) {
        return false;
    }
    return (millis() - _lockingCompleteTime >= _holdTime);
}

// --- SIMPLIFIED UPDATE FUNCTION ---
void SlotMachineAnimation::update() {
    if (isDone()) {
        // Draw the final frame once when done.
        if (!_finalFrameDrawn) {
            int displaySize = _display->getDisplaySize();
            for (int i = 0; i < displaySize; ++i) {
                if (i < _parsedText.length()) {
                    _display->setChar(i, _parsedText[i], _dotStates[i]);
                } else {
                    _display->setChar(i, ' ', false);
                }
            }
            _finalFrameDrawn = true;
        }
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastSpinTime < _spinDelay) {
        return; // Control spin speed
    }
    _lastSpinTime = currentTime;

    // --- Progression Logic (borrowed directly from MatrixAnimation) ---
    int displaySize = _display->getDisplaySize();
    bool revealPhaseActive = _lockedCount < displaySize;
    if (revealPhaseActive && (currentTime - _lastLockTime >= _lockDelay)) {
        _lastLockTime = currentTime;
        _lockedCount++;
        if (_lockedCount == displaySize) {
            _lockingCompleteTime = millis();
        }
    }

    // --- Drawing Logic (borrowed directly from MatrixAnimation) ---
    for (int i = 0; i < displaySize; ++i) {
        // REVEAL characters based on the counter, not the _isLocked vector.
        if (i < _lockedCount) {
            // This character is "revealed".
            if (i < _parsedText.length()) {
                _display->setChar(i, _parsedText[i], _dotStates[i]);
            } else {
                _display->setChar(i, ' ', false);
            }
        } else {
            // This character is still "spinning".
            _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
        }
    }
}