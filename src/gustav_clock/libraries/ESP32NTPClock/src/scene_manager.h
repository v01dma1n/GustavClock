#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "enc_types.h"
#include "i_base_clock.h"
#include "openweather_client.h"

#include <ESP32NTPClock.h>

#define MAX_SCENE_TEXT_LEN 64

class SceneManager {
public:
    SceneManager(IBaseClock& clock);
    void setup(const DisplayScene* playlist, int numScenes);
    void update();

private:
     IBaseClock& _app;

    // State for the scene playlist
    const DisplayScene* _scenePlaylist;
    int _numScenes;
    int _currentSceneIndex;
    unsigned long _sceneStartTime;
    unsigned long _lastLiveUpdateTime;
    bool _wasAnimationRunning;
};

#endif // SCENE_MANAGER_H
