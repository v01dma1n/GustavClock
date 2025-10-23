#include "gustav_app.h"
#include "display_manager.h"
#include "debug.h"
#include "version.h"             
#include "default_7seg_font.h" 
#include "vfd_hardware_map.h"

#include <Wire.h> 
#include <WiFi.h> 

// --- Data getters for the scene playlist ---u
float GustavClockApp_getTimeData() { return 0; }
float GustavClockApp_getTempData() { return GustavClockApp::getInstance().getTempData(); }
float GustavClockApp_getHumidityData() { return GustavClockApp::getInstance().getHumidityData(); }

// --- The application's specific scene playlist for the 12-digit VFD ---
static const DisplayScene scenePlaylist[] = {
    { "Date",        "%b.%d %Y",     MATRIX,        true,  false, 10000, 350, 50, GustavClockApp_getTimeData },
    { "Time",        " %H-%M-%S",    SLOT_MACHINE,  false, true,  10000, 200, 50, GustavClockApp_getTimeData },
    { "Date",        "%b.%d %Y",     MATRIX,        true,  false, 10000, 350, 50, GustavClockApp_getTimeData },
    { "Time",        " %H-%M-%S",    SLOT_MACHINE,  false, true,  10000, 200, 50, GustavClockApp_getTimeData },
    { "Temperature", "T %3.0f F",     MATRIX,        false, false,  7000, 250, 40, GustavClockApp_getTempData },
    { "Time",        " %H-%M-%S",    SLOT_MACHINE,  false, true,  10000, 200, 50, GustavClockApp_getTimeData },
    { "Humidity",    "H %2.0f PCT",   MATRIX,        false, false,  7000, 250, 40, GustavClockApp_getHumidityData }
};
static const int numScenes = sizeof(scenePlaylist) / sizeof(DisplayScene);

static Default7SegFont s_vfd_font;

// --- Constructor ---
GustavClockApp::GustavClockApp() :
    // Initialize the VFD driver with display size and SPI pins
    _display(DISP_LEN, s_vfd_font, s_vfd_segment_map),
    _appPrefs(),
    _apManager(_appPrefs)
{
    _displayManager = std::make_unique<DisplayManager>(_display);

    _prefs = &_appPrefs;
    BaseNtpClockApp::_apManager = &_apManager;
    _rtcActive = false;
}
 
// --- Destructor ---
GustavClockApp::~GustavClockApp() = default;

void GustavClockApp::setupHardware() {
    i2c_bus_clear();
    Wire.begin();
    _displayManager->begin();
    // This clock uses an RTC, so we initialize it.
    _rtcActive = _rtc.begin();
    if (_rtcActive && (!_rtc.isrunning() || _rtc.now() < DateTime(F(__DATE__), F(__TIME__)))) {
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

// --- Main Setup and Loop ---
void GustavClockApp::setup() {

    // Call the generic setup engine from the library
    BaseNtpClockApp::setup();
    
    // Set the global log level from the loaded preferences
    g_appLogLevel = _appPrefs.config.logLevel;

    // Initialize application-specific managers
    _weatherManager = std::make_unique<GustavWeatherDataManager>(*this);
    if (_sceneManager) {
        _sceneManager->setup(scenePlaylist, numScenes);
    }
    LOGINF("--- GUSTAV VFD APP SETUP COMPLETE ---");
}

void GustavClockApp::loop() {
    BaseNtpClockApp::loop();
    if (_weatherManager) _weatherManager->update();
}

DisplayManager& GustavClockApp::getClock() {
    return *_displayManager;
}

// --- IBaseClock & IWeatherClock Interface Implementations ---

void GustavClockApp::activateAccessPoint() {
    _apManager.setup(getAppName());
    String waitingMsgStr = "SETUP MODE -- WIFI ";
    waitingMsgStr += getAppName();
    
    String connectedMsgStr = "CONNECT AT ";
    connectedMsgStr += WiFi.softAPIP().toString();
    
    _apManager.runBlockingLoop(*_displayManager, waitingMsgStr.c_str(), connectedMsgStr.c_str());
}

float GustavClockApp::getTempData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.temperatureF : UNSET_VALUE;
}

float GustavClockApp::getHumidityData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.humidity : UNSET_VALUE;
}

bool GustavClockApp::isOkToRunScenes() const {
    return _fsmManager && _fsmManager->isInState("RUNNING_NORMAL");
}

void GustavClockApp::formatTime(char *txt, unsigned txt_size, const char *format, time_t now) {
    struct tm timeinfo = *localtime(&now);
    strftime(txt, txt_size, format, &timeinfo);
}

void GustavClockApp::syncRtcFromNtp() {
    if (!_rtcActive) return;
    time_t now_utc = time(nullptr);
    _rtc.adjust(DateTime(now_utc));
    LOGINF("RTC synchronized with NTP time.");
}

const char* GustavClockApp::getAppName() const {
    return AP_HOST_NAME;
}
