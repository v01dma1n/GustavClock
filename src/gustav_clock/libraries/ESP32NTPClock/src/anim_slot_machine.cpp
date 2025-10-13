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
    
    // --- FINAL FIX 1: Bypass the faulty parser ---
    // This was the source of the "stuck" scene issue during testing.
    _parsedText = _targetText;
    _dotStates.assign(_targetText.length(), 0); 

    _isLocked.assign(_display->getDisplaySize(), 0);
    
    // Reset all state variables
    _lastLockTime = millis();
    _lastSpinTime = millis();
    _lockedCount = 0;
    _lockingCompleteTime = 0;
    _finalFrameDrawn = false;
    randomSeed(analogRead(0));

    // Restore the initial "spinning" frame drawing.
    int displaySize = _display->getDisplaySize();
    for (int i = 0; i < displaySize; ++i) {
        // _display->setChar(i, random(0, 10) + '0', false);
        _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
    }
}

bool SlotMachineAnimation::isDone() {
    bool lockingComplete = _lockedCount >= _display->getDisplaySize();
    if (!lockingComplete) {
        return false;
    }
    return (millis() - _lockingCompleteTime >= _holdTime);
}

// --- FINAL FIX 2: Restore the original, fully functional update() function ---
void SlotMachineAnimation::update() {
    if (_finalFrameDrawn) {
        return;
    }

    int displaySize = _display->getDisplaySize();

    if (isDone()) {
        for (int i = 0; i < displaySize; ++i) {
            if (i < _parsedText.length()) {
                _display->setChar(i, _parsedText[i], _dotStates[i]);
            } else {
                _display->setChar(i, ' ', false);
            }
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
    bool isLockingPhase = _lockedCount < displaySize;
    if (isLockingPhase && (currentTime - _lastLockTime >= _lockDelay)) {
        _lastLockTime = currentTime;
        if (_lockedCount < _isLocked.size()) {
            _isLocked[_lockedCount] = 1;
            _lockedCount++;
        }

        if (_lockedCount == displaySize) {
            _lockingCompleteTime = millis();
        }
    }

    // --- Drawing Logic ---
    for (int i = 0; i < displaySize; ++i) {
        if (i < _isLocked.size() && _isLocked[i]) {
            if (i < _parsedText.length()) {
                _display->setChar(i, _parsedText[i], _dotStates[i]);
            } else {
                _display->setChar(i, ' ', false);
            }
        } else {
            // _display->setChar(i, random(0, 10) + '0', false);
            _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
        }
    }
}