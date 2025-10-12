#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "i_display_driver.h"
#include "i_animation.h"
#include <memory>

class DisplayManager {
public:
    DisplayManager(IDisplayDriver& display);
    void begin();
    void setAnimation(std::unique_ptr<IAnimation> animation);
    void update();
    bool isAnimationRunning() const;

private:
    IDisplayDriver& _display;
    std::unique_ptr<IAnimation> _currentAnimation;
};

#endif // DISPLAY_MANAGER_H