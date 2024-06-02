// Author: Leonardo La Rocca
// email: info@melopero.com
//
// In this example it is shown how to configure the device to detect
// gesture related interrupts.
//
// First make sure that your connections are setup correctly:
// I2C pinout:
// APDS9960 <------> Arduino MKR
//     VIN <------> VCC
//     SCL <------> SCL (12)
//     SDA <------> SDA (11)
//     GND <------> GND
//     INT <------> 1
//
// In this example we are using an MKR board. Each arduino board (type)
// has dfferent pins that can listen for interrupts. If you want to learn
// more about which pins can be used for interrupt detection on your arduino
// board we recommend you to consult this site: https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
//
// Note: Do not connect the device to the 5V pin!

#include "Melopero_APDS9960.h"
#include "Struct.h"

Melopero_APDS9960 device;

bool interruptOccurred = false;
//This is the pin that will listen for the hardware interrupt.
const byte interruptPin = 11;


int Brightness = 100;

// Define the structure to hold SSID and password
enum APDS9960Geasture {
  APDSNone = 0,
  APDSLeft = 1,
  APDSRight = 2
};

void interruptHandler() {
  interruptOccurred = true;
}

void setup_APDS9960() {
  int8_t status = NO_ERROR;

  Wire.begin(10, 9);

  status = device.initI2C(0x39, Wire);  // Initialize the comunication library
  if (status != NO_ERROR) {
    Serial.println("APDS9960: Error during initialization");
    uiClockCall = ClockFunction::ClockSayAPDS9960Error;
    ESP32Restart();
  }
  status = device.reset();  // Reset all interrupt settings and power off the device
  if (status != NO_ERROR) {
    Serial.println("APDS9960: Error during reset.");
    uiClockCall = ClockFunction::ClockSayAPDS9960Error;
    ESP32Restart();
  }

  Serial.println("APDS9960: Device initialized correctly!");

  device.enableAlsEngine();           // enable the color/ALS engine
  device.setAlsIntegrationTime(450);  // set the color engine integration time
  device.updateSaturation();          // updates the saturation value, stored in device.alsSaturation

  // Gesture engine settings
  device.enableGesturesEngine(true);        // enable the gesture engine
  device.setGestureProxEnterThreshold(25);  // Enter the gesture engine only when the proximity value
  // is greater than this value proximity value ranges between 0 and 255 where 0 is far away and 255 is very near.
  device.setGestureExitThreshold(20);  // Exit the gesture engine only when the proximity value is less
  // than this value.
  device.setGestureExitPersistence(EXIT_AFTER_7_GESTURE_END);  // Exit the gesture engine only when 4
  // consecutive gesture end signals are fired (distance is greater than the threshold)

  // Gesture engine interrupt settings
  device.enableGestureInterrupts();
  device.setGestureFifoThreshold(FIFO_INT_AFTER_16_DATASETS);  // trigger an interrupt as soon as there are 16 datasets in the fifo
  // To clear the interrupt pin we have to read all datasets that are available in the fifo.
  // Since it takes a little bit of time to read alla these datasets the device may collect
  // new ones in the meantime and prevent us from clearing the interrupt ( since the fifo
  // would not be empty ). To prevent this behaviour we tell the device to enter the sleep
  // state after an interrupt occurred. The device will exit the sleep state when the interrupt
  // is cleared.
  device.setSleepAfterInterrupt(true);

  //Next we want to setup our interruptPin to detect the interrupt and to call our
  //interruptHandler function each time an interrupt is triggered.
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interruptHandler, FALLING);


  device.setLedDrive(LED_DRIVE_100_mA);
  device.setLedBoost(LED_BOOST_200);
  device.wakeUp();  // wake up the device
}

void taskAPDS9960(void* parameter) {
  setup_APDS9960();

  static unsigned long ulADPS9960InterruptTime = 0;
  static uint8_t uiADPS9960Geasture = APDS9960Geasture::APDSNone;
  for (;;) {
    if (interruptOccurred) {
      ulADPS9960InterruptTime = millis();
      interruptOccurred = false;
      Serial.println("Interrupt occurred!");

      // Try to detect a gesture from the datasets present in the fifo
      device.parseGestureInFifo(12, 6, 6);

      // // Check for horizontal axis gesture
      // switch (device.parsedLeftRightGesture) {
      //   case NO_GESTURE:
      //     Serial.println("No horizontal axis gesture detected");
      //     break;
      //   case LEFT_GESTURE:
      //     if (uiADPS9960Geasture == APDS9960Geasture::APDSNone) {
      //       Serial.println("Detected Gesture: LEFT");
      //       uiADPS9960Geasture = APDS9960Geasture::APDSLeft;
      //     }
      //     break;
      //   case RIGHT_GESTURE:
      //     if (uiADPS9960Geasture == APDS9960Geasture::APDSNone) {
      //       Serial.println("Detected Gesture: RIGHT");
      //       uiADPS9960Geasture = APDS9960Geasture::APDSRight;
      //     }
      //     break;
      //   default:
      //     Serial.println("Unknown Gesture");
      //     break;
      // }
    }

    // Geasture interaction
    if (ulADPS9960InterruptTime > 0 && millis() - ulADPS9960InterruptTime >= 1000) {
      if (uiADPS9960Geasture == APDS9960Geasture::APDSNone) {
        uiClockCall = ClockFunction::ClockSayTime;
      }
      ulADPS9960InterruptTime = 0;
    }

    static unsigned long ulADPS9960Color = millis();
    if (millis() - ulADPS9960Color >= 10000) {
      device.updateColorData();  // update the values stored in device.red/green/blue/clear

      Brightness = (int)device.clear;
      //Serial.println("C:" + String(Brightness));
      ulADPS9960Color = millis();
    }

    vTaskDelay(calculateDelayInTicks(100));
  }
}
