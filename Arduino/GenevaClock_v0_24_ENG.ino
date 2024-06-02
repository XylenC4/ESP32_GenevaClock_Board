#include <AccelStepper.h>
#include "esp_system.h"

extern AccelStepper stepper;
extern const float fStepsPerDeg;
extern const float fStepsPer12H;
extern const float fStepsPer1H;
extern tm rtcTime;
extern bool bForceUpdateTime;
extern float fStepperPos;


// Create task
TaskHandle_t taskHandleNTPClient;
TaskHandle_t taskHandleAccelStepper;
TaskHandle_t taskHandleTalkingClock;
TaskHandle_t taskHandleAPDS9960;

bool bRestartAfter60s = false;

void setup() {
  Serial.begin(115200);

  setup_SDCard();

  xTaskCreatePinnedToCore(
    taskNTPClient,         // Function name of the task
    "taskNTPClient",       // Name of the task (e.g. for debugging)
    4096,                  // Stack size (bytes)
    NULL,                  // Parameter to pass
    1,                     // Task priority
    &taskHandleNTPClient,  // Task handle
    1                      // Core
  );

  while (rtcTime.tm_year + 1900 == 1970) {
    delay(1000);
  }
  showTime();

  xTaskCreatePinnedToCore(
    taskAccelStepper,         // Function name of the task
    "taskAccelStepper",       // Name of the task (e.g. for debugging)
    4096,                     // Stack size (bytes)
    NULL,                     // Parameter to pass
    1,                        // Task priority
    &taskHandleAccelStepper,  // Task handle
    0                         // Core
  );

  xTaskCreatePinnedToCore(
    taskTalkingClock,         // Function name of the task
    "taskTalkingClock",       // Name of the task (e.g. for debugging)
    4096,                     // Stack size (bytes)
    NULL,                     // Parameter to pass
    1,                        // Task priority
    &taskHandleTalkingClock,  // Task handle
    1                         // Core
  );

  xTaskCreatePinnedToCore(
    taskAPDS9960,         // Function name of the task
    "taskAPDS9960",       // Name of the task (e.g. for debugging)
    4096,                 // Stack size (bytes)
    NULL,                 // Parameter to pass
    1,                    // Task priority
    &taskHandleAPDS9960,  // Task handle
    1                     // Core
  );
}

void loop() {

  // static unsigned long ulAccelStepperSerialPrint = 0;
  // if (millis() - ulAccelStepperSerialPrint >= 10000) {
  //   Serial.print(F("Current Position: "));
  //   Serial.print(stepper.currentPosition());
  //   Serial.print(F(";Distance GoTo: "));
  //   Serial.print(stepper.distanceToGo());
  //   Serial.print(F(";Position in time: "));
  //   Serial.print(String((float)stepper.currentPosition() / fStepsPer1H, 2));
  //   Serial.print(F(";fStepperPos in time: "));
  //   Serial.println(String(fStepperPos, 3));
  //   ulAccelStepperSerialPrint = millis();
  // }

  // send data only when you receive data:
  if (Serial.available() > 0) {
    String timeString = Serial.readStringUntil('\n');
    adjustTime(timeString.c_str());
    bForceUpdateTime = true;
  }

  if (bRestartAfter60s) {
    Serial.println("Restart in 60s...");
    vTaskDelete(taskHandleNTPClient);
    vTaskDelete(taskHandleAccelStepper);
    vTaskDelete(taskHandleAPDS9960);
    delay(60000);
    vTaskDelete(taskHandleTalkingClock);
    esp_restart();
  }

  delay(1000);
}

void ESP32Restart() {
  bRestartAfter60s = true;
}

// Function to calculate delay in ticks based on milliseconds
TickType_t calculateDelayInTicks(uint32_t delay_ms) {
  return (delay_ms * configTICK_RATE_HZ) / 1000;
}

void delay_us(uint32_t microseconds) {
  // Calculate the number of iterations needed for a 10 microsecond delay
  uint32_t iterations = microseconds * (APB_CLK_FREQ / 1000000) / 10;
  for (uint32_t i = 0; i < iterations; i++) {
    asm volatile("nop");  // Assembly NOP instruction for very short delay
  }
}
