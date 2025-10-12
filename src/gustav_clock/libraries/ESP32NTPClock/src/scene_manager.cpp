#include "scene_manager.h"
#include "enc_debug.h"
#include "i_weather_clock.h"

#include "anim_slot_machine.h"
#include "anim_matrix.h"
#include "anim_scrolling_text.h"

SceneManager::SceneManager(IBaseClock& clock) : _app(clock) {}

void SceneManager::setup(const DisplayScene* playlist, int numScenes) {
    _scenePlaylist = playlist;
    _numScenes = numScenes;
    _currentSceneIndex = -1;
    _sceneStartTime = 0;    
}

void SceneManager::update() {

    // This logic only runs if the app says it's okay (e.g., FSM is in the right state)
    if (_app.isOkToRunScenes() == false) {
        return;
    }
 
    if (_app.getClock().isAnimationRunning()) {
        return;
    }

    // --- Dynamic buffer allocation based on actual display size ---
    const int displaySize = _app.getDisplay().getDisplaySize();
    // Use a vector for safe memory management. Add +1 for the null terminator.
    std::vector<char> buffer(displaySize + 1);

    unsigned long currentTime = millis();

    // Check if it's time to switch to a new scene
    bool isSceneSwitchTime = false;
    if (_currentSceneIndex < 0) {
        isSceneSwitchTime = true;
    } else {
    const DisplayScene& currentScene = _scenePlaylist[_currentSceneIndex];
        if ((currentTime - _sceneStartTime) >= currentScene.duration_ms) {
            isSceneSwitchTime = true;
        }
    }

    if (isSceneSwitchTime) {
        // --- Logic to select the next scene, skipping weather if needed ---
        int nextIndex = _currentSceneIndex;
        
        const char* units = "imperial"; // Default
        IWeatherClock* weather_app = _app.asWeatherClock(); // Safely check for weather support
        if (weather_app) {
            units = weather_app->getTempUnit();
        }
        
        while (true) {
            nextIndex = (nextIndex + 1) % _numScenes;
            const DisplayScene& nextScene = _scenePlaylist[nextIndex];
            
            if (strcmp(nextScene.scene_name, "Temperature") == 0) {
                bool isImperialScene = (strstr(nextScene.format_string, "F") != nullptr);
                if ((strcmp(units, OWM_UNIT_IMPERIAL) == 0 && isImperialScene) || 
                    (strcmp(units, OWM_UNIT_METRIC) == 0 && !isImperialScene)) {
                    break;
                }
            } else {
                break;
            }
        }
        _currentSceneIndex = nextIndex;
        _sceneStartTime = millis();

        const DisplayScene& newScene = _scenePlaylist[_currentSceneIndex];

        // --- Prepare the text buffer for the NEW scene ---
        std::string sceneText;
        time_t now = _app.isRtcActive() ? _app.getRtc().now().unixtime() : time(0);
        if (strcmp(newScene.scene_name, "Time") == 0 || strcmp(newScene.scene_name, "Date") == 0) {
            _app.formatTime(buffer.data(), buffer.size(), newScene.format_string, now);
            sceneText = buffer.data();
        } else {
            float value = newScene.getDataValue();
            if (value == UNSET_VALUE) {
                sceneText = "NO DATA";
            } else {
                snprintf(buffer.data(), buffer.size(), newScene.format_string, value);
                sceneText = buffer.data();
            }
        }

        // // --- TEMPORARY DEBUGGING ---
        // // This will show the text statically for 2 seconds.
        // ENC_LOG("Forcing StaticTextAnimation with text: '%s'", sceneText.c_str());
        // auto anim = std::make_unique<StaticTextAnimation>(sceneText);
        // _app.getClock().setAnimation(std::move(anim));
        // // --- END TEMPORARY DEBUGGING ---

        
        // --- Create and set the animation with the prepared text ---
        if (newScene.animation_type == SLOT_MACHINE) {
            auto anim = std::make_unique<SlotMachineAnimation>(sceneText, newScene.anim_param_1, 2000, newScene.anim_param_2, newScene.dots_with_previous);
            _app.getClock().setAnimation(std::move(anim));
        } else if (newScene.animation_type == MATRIX) {
            auto anim = std::make_unique<MatrixAnimation>(sceneText, newScene.anim_param_1, 2000, newScene.anim_param_2, newScene.dots_with_previous);
            _app.getClock().setAnimation(std::move(anim));
        } else if (newScene.animation_type == SCROLLING) {
            auto anim = std::make_unique<ScrollingTextAnimation>(sceneText, newScene.anim_param_1, newScene.dots_with_previous);
            _app.getClock().setAnimation(std::move(anim));
        }

        ENC_LOG("Scene: %s - %s", newScene.scene_name, sceneText.c_str());

    } else {

        // // --- Logic to update the display for the CURRENT scene (if it's static) ---
        // const DisplayScene& currentScene = _scenePlaylist[_currentSceneIndex];

        // // If the scene has an animation, its final frame should persist. Don't overwrite it.
        // if (currentScene.animation_type == SCROLLING ||
        //     currentScene.animation_type == MATRIX ||
        //     currentScene.animation_type == SLOT_MACHINE) {
        //     return;
        // }

        // // Only update static scenes (of which there are none currently, but this is good practice)
        // time_t now = _app.isRtcActive() ? _app.getRtc().now().unixtime() : time(0);
        // if (strcmp(currentScene.scene_name, "Time") == 0 || strcmp(currentScene.scene_name, "Date") == 0) {
        //     _app.formatTime(buffer.data(), buffer.size(), currentScene.format_string, now);
        // } else {
        //     float value = currentScene.getDataValue();
        //     if (value == UNSET_VALUE) {
        //         // Use snprintf for safety; it guarantees null termination.
        //         snprintf(buffer.data(), buffer.size(), "NO DATA");
        //     } else {
        //         snprintf(buffer.data(), buffer.size(), currentScene.format_string, value);
        //     }
        // }
        // _app.getDisplay().print(buffer.data(), currentScene.dots_with_previous);
    }
}
