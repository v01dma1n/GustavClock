#include "display_manager.h"
#include "enc_debug.h"

DisplayManager::DisplayManager(IDisplayDriver& display) : _display(display) {}

void DisplayManager::begin() {
    _display.begin();
    _display.setBrightness(8);
}

void DisplayManager::setAnimation(std::unique_ptr<IAnimation> animation) {
    _currentAnimation = std::move(animation);
    if (_currentAnimation) {
        _currentAnimation->setup(&_display);
    }
}

void DisplayManager::update() {
    if (_currentAnimation) {
        _currentAnimation->update();
        if (_currentAnimation->isDone()) {
            _currentAnimation.reset();
            _display.clear();
        }
    }

    bool needsUpdate = _display.needsContinuousUpdate();
    if (needsUpdate) {
        _display.writeDisplay();
    }
}

bool DisplayManager::isAnimationRunning() const {
    // The animation is running if the unique_ptr is not null
    return _currentAnimation != nullptr;
}

