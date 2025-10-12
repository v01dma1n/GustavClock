#ifndef I_BASE_CLOCK_H
#define I_BASE_CLOCK_H

#include "RTClib.h"

// Forward-declare all classes that are used as references or pointers
// in this interface file. This breaks circular header dependencies.
class DisplayManager;
class IDisplayDriver;
// -----------------------

// Forward-declare the weather interface
class IWeatherClock;

class IBaseClock {
public:
    virtual ~IBaseClock() {}

    // Method to safely check if this clock supports the weather interface.
    // Base implementation returns nullptr.
    virtual IWeatherClock* asWeatherClock() { return nullptr; }

    // --- UNIVERSAL METHODS ---
    virtual const char* getSsid() const = 0;
    virtual const char* getPassword() const = 0;
    virtual const char* getTimezone() const = 0;
    virtual bool isOkToRunScenes() const = 0;
    virtual void formatTime(char *txt, unsigned txt_size, const char *format, time_t now) = 0;
    virtual IDisplayDriver& getDisplay() = 0;
    virtual DisplayManager& getClock() = 0;
    virtual RTC_DS1307& getRtc() = 0;
    virtual bool isRtcActive() const = 0;
    virtual void syncRtcFromNtp() = 0;
    virtual const char* getAppName() const = 0;
    virtual void activateAccessPoint() = 0;
};

#endif // I_BASE_CLOCK_H

