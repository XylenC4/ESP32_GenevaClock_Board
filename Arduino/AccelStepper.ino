// Motor pin definitions:
#define ACCELSTEPPER_PIN1 35  // IN1 on the ULN2003 driver
#define ACCELSTEPPER_PIN2 36  // IN2 on the ULN2003 driver
#define ACCELSTEPPER_PIN3 37  // IN3 on the ULN2003 driver
#define ACCELSTEPPER_PIN4 38  // IN4 on the ULN2003 driver

// Define Inputs
#define ACCELSTEPPER_HOME_SENSOR_PIN 14

#define ACCELSTEPPER_DEFAULT_SPEED 500


// Define the Ratios
const float fStepsPerDeg = 5.625F * (1.0F / 32.0F);
const float fStepsPerRot = 360.0F / fStepsPerDeg * (32.0F / 8.0F);
const float fStepsPer12H = fStepsPerRot * (12.0F / 3.0F);
const float fStepsPer1H = fStepsPerRot * (1.0F / 3.0F);
const float fStepsPer1M = fStepsPer1H / 60.0F;

float fStepperPos = 0;

float HomeOffset;

bool bForceUpdateTime = true;

extern tm rtcTime;


// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper stepper(AccelStepper::FULL4WIRE, ACCELSTEPPER_PIN4, ACCELSTEPPER_PIN2, ACCELSTEPPER_PIN3, ACCELSTEPPER_PIN1);

enum AccelStepperStates {
  None,
  MoveToHour,
  Homing
};

int iHomingState = 0;

AccelStepperStates AccelStepperState = None;

bool SetStateHoming() {
  AccelStepperState = Homing;

  if (iHomingState == 5) {
    return true;
    iHomingState = 0;
  } else {
    return false;
  }
}

float fHour = 0;

void taskAccelStepper(void* parameter) {
  // Homing sensor pulldown
  pinMode(ACCELSTEPPER_HOME_SENSOR_PIN, INPUT_PULLDOWN);  // enable the internal pull-up resistor

  stepper.setMaxSpeed(ACCELSTEPPER_DEFAULT_SPEED);  // set the maximum speed
  stepper.setAcceleration(100);                     // set acceleration
  stepper.setSpeed(ACCELSTEPPER_DEFAULT_SPEED);     // set initial speed
  stepper.setCurrentPosition(0);                    // set position

  SetStateHoming();
  for (;;) {
    switch (AccelStepperState) {
      case None:
        break;
      case MoveToHour:
        MoveToTime();
        break;
      case Homing:
        Home();
        break;
      default:
        // statements
        break;
    }
    vTaskDelay(1);
  }
}

void Home() {
  switch (iHomingState) {
    case 0:
      Serial.println("Init Homing");
      stepper.setSpeed(ACCELSTEPPER_DEFAULT_SPEED);  // set initial speed
      stepper.setCurrentPosition(0);                 // set position
      stepper.moveTo(fStepsPer12H + fStepsPerRot);
      iHomingState = 1;
    case 1:
      stepper.run();
      if (digitalRead(ACCELSTEPPER_HOME_SENSOR_PIN)) {
        Serial.println("Homing sensor active");
        stepper.setCurrentPosition(0);  // set position
        stepper.moveTo(-1000);
        iHomingState = 2;
      } else if (stepper.distanceToGo() == 0) {
        uiClockCall = ClockFunction::ClockSayHomingError;
        iHomingState = 99;
        ESP32Restart();
      }
      break;
    case 2:
      stepper.run();
      if (stepper.distanceToGo() == 0) {
        Serial.println("Back position reached, go to sensor slowly");
        stepper.setSpeed(ACCELSTEPPER_DEFAULT_SPEED / 3);  // set initial speed
        stepper.moveTo(+500);
        iHomingState = 3;
      }
      break;
    case 3:
      stepper.run();
      if (digitalRead(ACCELSTEPPER_HOME_SENSOR_PIN)) {
        Serial.println("Homing sensor active");
        stepper.setSpeed(ACCELSTEPPER_DEFAULT_SPEED);  // set initial speed
        float Offset = HomeOffset * fStepsPer1H;
        stepper.setCurrentPosition(Offset);  // set position
        stepper.moveTo(0);
        iHomingState = 4;
      }
      break;
    case 4:
      Serial.println("Set zero position, END");
      iHomingState = 5;
    case 5:
      AccelStepperState = MoveToHour;
      bForceUpdateTime = true;
      iHomingState = 0;
    default:
      // statements
      break;
  }
}

void MoveToTime() {
  if (bForceUpdateTime) {
    showTime();

    fStepperPos = getTime() * fStepsPer1H;
    Serial.println("Force update Stepper position absolute: " + String(getTime()));
    bForceUpdateTime = false;
  }

  static unsigned long ulRefresPos = 0;
  if (millis() - ulRefresPos >= 5 * 1000) {
    static int iAccelStepperLastMin = rtcTime.tm_min;
    if (iAccelStepperLastMin != rtcTime.tm_min && rtcTime.tm_year + 1900 > 1970) {
      fStepperPos += fStepsPer1M;
      iAccelStepperLastMin = rtcTime.tm_min;
    }
    ulRefresPos = millis();
  }

  static int iLastDayHomeSync = rtcTime.tm_yday;
  if (digitalRead(ACCELSTEPPER_HOME_SENSOR_PIN) && iLastDayHomeSync != rtcTime.tm_yday) {
    AccelStepperState = Homing;
    bForceUpdateTime = true;
    iLastDayHomeSync = rtcTime.tm_yday;
  }

  stepper.moveTo(fStepperPos);

  static bool bDisableStatus = false;
  if (stepper.distanceToGo() == 0 && !bDisableStatus) {
    stepper.disableOutputs();
    bDisableStatus = true;
    //Serial.println("DISABLE Stepper");
  } else if (stepper.distanceToGo() != 0 && bDisableStatus) {
    stepper.enableOutputs();
    bDisableStatus = false;
    //Serial.println("ENABLE Stepper");
  }

  stepper.run();
}
