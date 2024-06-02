// Struct.h
#ifndef STRUCT_H
#define STRUCT_H

const int MAX_NETWORKS = 10; // Adjust as needed
const int SSID_BUFFER_SIZE = 32; // Adjust as needed
const int PASSWORD_BUFFER_SIZE = 64; // Adjust as needed

// Define the structure to hold SSID and password
struct Network {
  char ssid[SSID_BUFFER_SIZE];
  char password[PASSWORD_BUFFER_SIZE];
};

enum ClockFunction {
  ClockNone,
  ClockSayTime,
  ClockSayVolume,
  ClockSayMinuteStepping,
  ClockSayNTPError,
  ClockSayResetSettings,
  ClockSayWIFILoginFailed,
  ClockSayWIFINotAvailable,
  ClockSayAPDS9960Error,
  ClockSayHomingError
};

uint8_t uiClockCall = ClockFunction::ClockNone;

#endif  // STRUCT_H
