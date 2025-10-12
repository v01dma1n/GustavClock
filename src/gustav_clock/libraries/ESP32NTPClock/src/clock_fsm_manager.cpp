#include "clock_fsm_manager.h"
#include "display_manager.h"
#include "sntp_client.h"
#include "anim_scrolling_text.h"
#include "anim_static_text.h"

#include <WiFi.h>

// This static instance pointer is the bridge between the C-style FSM callbacks
// and our C++ ClockFsmManager object.
static ClockFsmManager* _instance = nullptr;

// --- C-style Wrapper Functions ---
// These simple functions are needed for the SimpleFSM library.
// They call the actual methods on the active C++ object.
void sntp_sync_callback_bridge() {
    if (_instance) {
        _instance->on_enter_ntp_sync();
    }
}
void fsm_on_enter_startup_anim_wrapper() { if (_instance) _instance->on_enter_startup_anim(); }
void fsm_on_enter_wifi_connect_wrapper() { if (_instance) _instance->on_enter_wifi_connect(); }
void fsm_on_enter_ntp_sync_wrapper() { if (_instance) _instance->on_enter_ntp_sync(); }
void fsm_on_exit_ntp_sync_wrapper() { if (_instance) _instance->on_exit_ntp_sync(); }
void fsm_on_enter_ap_mode_wrapper() { if (_instance) _instance->on_enter_ap_mode(); }
void fsm_on_exit_ap_mode_wrapper() { if (_instance) _instance->on_exit_ap_mode(); }
void fsm_on_enter_running_normal_wrapper() { if (_instance) _instance->on_enter_running_normal(); }

bool fsm_guard_anim_done_wrapper() { return _instance ? _instance->guard_anim_done() : false; }
bool fsm_guard_wifi_connected_wrapper() { return _instance ? _instance->guard_wifi_connected() : false; }
bool fsm_guard_wifi_timed_out_wrapper() { return _instance ? _instance->guard_wifi_timed_out() : false; }
bool fsm_guard_ntp_success_wrapper() { return _instance ? _instance->guard_ntp_success() : false; }
bool fsm_guard_ntp_timeout_with_rtc_wrapper() { return _instance ? _instance->guard_ntp_timeout_with_rtc() : false; }
bool fsm_guard_ntp_timeout_no_rtc_wrapper() { return _instance ? _instance->guard_ntp_timeout_no_rtc() : false; }
bool fsm_guard_ap_timeout_with_rtc_wrapper() { return _instance ? _instance->guard_ap_timeout_with_rtc() : false; }


// --- Class Implementation ---

ClockFsmManager::ClockFsmManager(IBaseClock& clock) : _clock(clock) {
    _instance = this; // Set the static instance pointer
}

void ClockFsmManager::setup() {
    static State states[] = {
        State("STARTUP_ANIM", fsm_on_enter_startup_anim_wrapper),
        State("WIFI_CONNECT", fsm_on_enter_wifi_connect_wrapper),
        State("NTP_SYNC", fsm_on_enter_ntp_sync_wrapper, nullptr, fsm_on_exit_ntp_sync_wrapper),
        State("AP_MODE", fsm_on_enter_ap_mode_wrapper, nullptr, fsm_on_exit_ap_mode_wrapper),
        State("RUNNING_NORMAL", fsm_on_enter_running_normal_wrapper)
    };

    static TimedTransition transitions[] = {
        TimedTransition(&states[0], &states[1], 100, nullptr, "", fsm_guard_anim_done_wrapper),
        TimedTransition(&states[1], &states[2], 100, nullptr, "", fsm_guard_wifi_connected_wrapper),
        TimedTransition(&states[1], &states[3], 15000, nullptr, "", fsm_guard_wifi_timed_out_wrapper),
        TimedTransition(&states[2], &states[4], 100, nullptr, "", fsm_guard_ntp_success_wrapper),
        TimedTransition(&states[2], &states[4], 15000, nullptr, "", fsm_guard_ntp_timeout_with_rtc_wrapper),
        TimedTransition(&states[2], &states[3], 15000, nullptr, "", fsm_guard_ntp_timeout_no_rtc_wrapper),
        TimedTransition(&states[3], &states[4], 60000, nullptr, "", fsm_guard_ap_timeout_with_rtc_wrapper)
    };
    
    _fsm.add(transitions, sizeof(transitions) / sizeof(TimedTransition));
    _fsm.setInitialState(&states[0]);
}

void ClockFsmManager::update() {
    _fsm.run(50);
}

bool ClockFsmManager::isInState(const String& stateName) const {
    State* currentState = _fsm.getState();
    if (currentState != nullptr) {
        return currentState->getName() == stateName;
    }
    return false;
}

// --- FSM Callback Implementations (generic) ---

void ClockFsmManager::on_enter_startup_anim() {
    Serial.println("ClockFsmManager::on_enter_startup_anim()");
    // This can be overridden by the specific clock implementation for a custom message
    auto startupMsg = std::make_unique<ScrollingTextAnimation>("LOADING...");
    _clock.getClock().setAnimation(std::move(startupMsg));
}

void ClockFsmManager::on_enter_wifi_connect() {
    Serial.println("ClockFsmManager::on_enter_wifi_connect()");
    WiFi.setHostname(_clock.getAppName());
    WiFi.begin(_clock.getSsid(), _clock.getPassword());
    auto connectingMsg = std::make_unique<ScrollingTextAnimation>("CONNECTING TO WIFI...");
    _clock.getClock().setAnimation(std::move(connectingMsg));
}

void ClockFsmManager::on_enter_ntp_sync() {
    Serial.println("ClockFsmManager::on_enter_ntp_sync()");
    // Use the new interface method
    setupSntp(_clock.getTimezone());
    auto syncingMsg = std::make_unique<ScrollingTextAnimation>("SYNCING TIME...");
    _clock.getClock().setAnimation(std::move(syncingMsg));
}

void ClockFsmManager::on_exit_ntp_sync() {
    Serial.println("ClockFsmManager::on_exit_ntp_sync()");
    _clock.getClock().setAnimation(nullptr);
}

void ClockFsmManager::on_enter_ap_mode() {
    _clock.activateAccessPoint();    
}

void ClockFsmManager::on_exit_ap_mode() {
    WiFi.softAPdisconnect(true);
    _clock.getClock().setAnimation(nullptr);
}

void ClockFsmManager::on_enter_running_normal() {
    Serial.println("ClockFsmManager::on_enter_running_normal()");
    // This state is entered after a successful NTP sync (among other paths).
    // If timeAvail is true, it means we just got NTP time, so this is the
    // safe place to sync the RTC.
   if (timeAvail) {
        _clock.syncRtcFromNtp();
    }
}

// --- FSM Guard Implementations (now generic) ---

bool ClockFsmManager::guard_anim_done() { return !_clock.getClock().isAnimationRunning(); }
bool ClockFsmManager::guard_wifi_connected() { return WiFi.status() == WL_CONNECTED; }
bool ClockFsmManager::guard_wifi_timed_out() { return WiFi.status() != WL_CONNECTED; }
bool ClockFsmManager::guard_ntp_success() { return timeAvail; }
bool ClockFsmManager::guard_ntp_timeout_with_rtc() { return !timeAvail && _clock.isRtcActive(); }
bool ClockFsmManager::guard_ntp_timeout_no_rtc() { return !timeAvail && !_clock.isRtcActive(); }
bool ClockFsmManager::guard_ap_timeout_with_rtc() { return _clock.isRtcActive(); }
