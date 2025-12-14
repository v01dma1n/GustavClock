# "Bare Soul of My Circuit" aka. Gustav VFD Clock

A 10-digit VFD (Vacuum Fluorescent Display) clock based on the ESP32. It features automatic NTP time synchronization, weather display from OpenWeatherMap, and a web-based configuration portal. This project is built using the `ESP32NTPClock` library framework.
<p align="center">
    <img src="photos/GustavClock.jpg" alt="Bubble LED Clock" width="500" style="margin: 0 auto; display: block;">
</p>

-----

## ✨ Features

  * **10-Digit VFD Display:** Driven by a MAX6921 chip over SPI.
  * **Automatic Time Sync:** Connects to your WiFi and synchronizes its time from an NTP server.
  * **Weather Display:** Fetches and displays the current temperature and humidity for a configured location from OpenWeatherMap.
  * **Animated Scenes:** Cycles through displaying the time, date, and weather using a playlist of customizable animations (Slot Machine, Scrolling, Matrix).
  * **Web Configuration:** On first boot or when WiFi fails, the clock enters an Access Point (AP) mode. You can connect to its WiFi network to configure all settings through a web page on your phone or computer.
  * **Note:** the clock does not have RTC (Real Time Clock) hardware 
-----

## ⚙️ Hardware Required

  * **ESP32 Module:** An ESP32-WROOM-DA was used in development.
  * **VFD Display:** A 12-digit Vacuum Fluorescent Display.
  * **VFD Driver:** A MAX6921 driver chip.
  * **Power Supply:** A stable 5V power supply capable of powering the ESP32 and the VFD.

-----

## 📚 Libraries & Dependencies

This project relies on the **`ESP32NTPClock`** framework and several other Arduino libraries.

  * **Core Libraries (created for this project):**

      * `ESP32NTPClock` (The core application engine)
      * `ESP32NTPClock_MAX6921` (The VFD display driver)

  * **Third-Party Libraries:**

      * [ESP Async WebServer](https://github.com/me-no-dev/ESPAsyncWebServer) (and its dependency, [AsyncTCP](https://github.com/me-no-dev/AsyncTCP))
      * [RTClib](https://github.com/adafruit/RTClib) by Adafruit
      * [SimpleFSM](https://www.google.com/search?q=https://github.com/tockn/SimpleFSM)

  * **Built-in ESP32 Libraries:**

      * `WiFi`, `SPI`, `Wire`, `Preferences`, `HTTPClient` (included with the ESP32 board package).

### Versions

Async TCP v.3.4.7
DNSServer v.3.3.0
ESP Async WebServer v.3.8.0
ESP32 Async UDP v.3.3.0
FS v.3.3.0
HTTPClient v.3.3.0
NetworkClientSecure v.3.3.0
Networking v.3.3.0
Preferences v.3.3.0
SPI v.3.3.0
WiFi v.3.3.0
Wire v.3.3.0
RTClib v.2.1.4
SimpleFSM v.1.3.1
ESP32NTPClock v.1.0.0
ESP32NTPClock Display Drivers v.1.0.0
Adafruit BusIO v.1.17.2

-----

## 🛠️ Configuration

1.  **First Boot:** On the very first boot, the clock will not have WiFi credentials. It will automatically start in Access Point (AP) mode. The display will show a message like "SETUP MODE...".
2.  **Connect to the AP:** Using your phone or computer, connect to the WiFi network named **`gustav-clock`**.
3.  **Captive Portal:** Once connected, a captive portal should automatically open. If it doesn't, navigate to **`192.168.4.1`** in your web browser.
4.  **Enter Settings:** Fill in your WiFi SSID and password, your [POSIX timezone string](https://www.google.com/search?q=http://www.timezoneconverter.com/cgi-bin/posix.tz.pl), and your OpenWeatherMap API key and location.
5.  **Save:** Click "Save and Restart." The clock will reboot and connect to your WiFi.

To re-enter configuration mode at any time, simply press the **reset button twice** in quick succession (about 1-2 seconds apart).

-----

## 🚀 Building the Project

1.  Install the [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/).
2.  Install the ESP32 board support package.
3.  Install all the libraries listed in the **Libraries & Dependencies** section above.
4.  Place all the `gustav_clock` project files into a sketch folder of the same name.
5.  Open `gustav_clock.ino`, select your ESP32 board and port, and click **Upload**.

-----

## 📜 License

This project is licensed under the MIT License - see the `LICENSE` file for details.
