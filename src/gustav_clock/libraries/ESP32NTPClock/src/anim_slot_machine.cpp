#include "anim_slot_machine.h"
#include <Arduino.h>
#include <algorithm> // Required for std::find

SlotMachineAnimation::SlotMachineAnimation(std::string targetText,
                                           unsigned long lockDelay,
                                           unsigned long spinDelay,
                                           bool dotsWithPreviousChar)
    : _targetText(targetText),
      _lockDelay(lockDelay),
      _spinDelay(spinDelay),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _lastLockTime(0),
      _lastSpinTime(0),
      _finalFrameDrawn(false),
      _rng(millis())
{}

SlotMachineAnimation::~SlotMachineAnimation() {}

void SlotMachineAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    
    _parsedText = _targetText;
    _dotStates.assign(_targetText.length(), 0); 
    
    // Clear the list of locked indices.
    _lockedIndices.clear();
    
    _lastLockTime = millis();
    _lastSpinTime = millis();
    _finalFrameDrawn = false;
    
    int displaySize = _display->getDisplaySize();
    for (int i = 0; i < displaySize; ++i) {
        _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
    }
}

bool SlotMachineAnimation::isDone() {
    return _lockedIndices.size() >= _display->getDisplaySize();
}

void SlotMachineAnimation::update() {
    if (isDone()) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastSpinTime < _spinDelay) {
        return;
    }
    _lastSpinTime = currentTime;

    // --- State Update Logic ---
    int displaySize = _display->getDisplaySize();
    if (_lockedIndices.size() < displaySize && (currentTime - _lastLockTime >= _lockDelay)) {
        _lastLockTime = currentTime;
        
        while (_lockedIndices.size() < displaySize) {
            int potentialIndex = _rng.nextRange(0, displaySize - 1);
            bool alreadyLocked = (std::find(_lockedIndices.begin(), _lockedIndices.end(), potentialIndex) != _lockedIndices.end());
            if (!alreadyLocked) {
                _lockedIndices.push_back(potentialIndex);
                break;
            }
        }
    }

    // --- Drawing Logic ---
    for (int i = 0; i < displaySize; ++i) {
        bool isLocked = (std::find(_lockedIndices.begin(), _lockedIndices.end(), i) != _lockedIndices.end());
        
        if (isLocked) {
            if (i < _parsedText.length()) {
                _display->setChar(i, _parsedText[i], _dotStates[i]);
            } else {
                _display->setChar(i, ' ', false);
            }
        } else {
            _display->setChar(i, _rng.nextRange(0, 9) + '0', false);    
        }
    }
}