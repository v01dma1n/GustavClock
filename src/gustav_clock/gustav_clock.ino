#include "gustav_app.h"
#include <ESP32NTPClock.h>

SemaphoreHandle_t serialMutex = NULL;
AppLogLevel g_appLogLevel = APP_LOG_INFO;
GustavClockApp& app = GustavClockApp::getInstance();

// The displayTriggerQueue is now REMOVED.

/**
 * @brief High-priority task to handle display multiplexing.
 *
 * This task is now a free-running, non-blocking multiplexer.
 * It continuously reads the driver's internal buffer and sends
 * one digit at a time to the display.
 */
void displayTask(void *pvParameters) {
  GustavClockApp& app_ref = GustavClockApp::getInstance();

  for (;;) {
    // 1. Call the non-blocking function to display the next digit.
    //    This function reads from the driver's internal buffer.
    app_ref.getDisplay().writeNextDigit();

    // 2. This is the non-blocking delay.
    //    It yields the CPU, allowing other tasks to run.
    //    A 2ms delay for 10 digits gives a ~20ms total refresh,
    //    or a 50Hz refresh rate.
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void setup() {
  delay(300);
  Serial.begin(115200);
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 2000)) {
    ; // Wait for serial port to connect, with a 2-second timeout
  }
  Serial.println("\n>>> Starting Gustav VFD Clock...");
  
  serialMutex = xSemaphoreCreateMutex();

  // Run the application setup
  app.setup();

  // Create the high-priority display task on Core 1
  xTaskCreatePinnedToCore(
      displayTask,      // Task function
      "DisplayTask",    // Name for debugging
      4096,             // Stack size
      NULL,             // Task input parameter
      10,               // Priority (high)
      NULL,             // Task handle
      1);               // Core ID

  Serial.println("\n>>> ... setup complete");
}

void loop() {
  app.loop();
}