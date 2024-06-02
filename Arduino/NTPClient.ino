#include <time.h>
#include <WiFi.h>

/*
Example source: https://werner.rothschopf.net/microcontroller/202103_arduino_esp32_ntp_en.htm
NTP TZ DST - bare minimum
NetWork Time Protocol - Time Zone - Daylight Saving Time

Our target for this MINI sketch is:
- get the SNTP request running
- set the timezone
- (implicit) respect daylight saving time
- how to "read" time to be printed to Serial.Monitor

This example is a stripped down version of the NTP-TZ-DST (v2) from the ESP8266
and contains some #precompiler defines to make it working for
- ESP32 core 1.0.5, 1.0.6, 2.0.2

by noiasca
2021-03-28
*/

/* Configuration of NTP */
// choose the best fitting NTP server pool for your country
String NTPServer;

// choose your time zone from this list
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
String TimeZoneInformation;

// last day when the NTP time was aqquired
uint16_t lastDay = -1;

/* Globals */
time_t now;  // this are the seconds since Epoch (1970) - UTC
tm rtcTime;  // the structure tm holds time information in a more convenient way *

// Force updating the apsolute AccelStepper position
extern bool bForceUpdateTime;

void showTime() {
  time(&now);                   // read the current time
  localtime_r(&now, &rtcTime);  // update the structure tm with the current time
  Serial.print("year:");
  Serial.print(rtcTime.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(rtcTime.tm_mon + 1);  // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(rtcTime.tm_mday);  // day of month
  Serial.print("\thour:");
  Serial.print(rtcTime.tm_hour);  // hours since midnight 0-23
  Serial.print("\tmin:");
  Serial.print(rtcTime.tm_min);  // minutes after the hour 0-59
  Serial.print("\tsec:");
  Serial.print(rtcTime.tm_sec);  // seconds after the minute 0-61*
  Serial.print("\twday");
  Serial.print(rtcTime.tm_wday);  // days since Sunday 0-6
  if (rtcTime.tm_isdst == 1)      // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}

double getTime() {
  time(&now);                   // read the current time
  localtime_r(&now, &rtcTime);  // update the structure tm with the current time
  double hour = rtcTime.tm_hour % 12;
  double min = rtcTime.tm_min;

  return hour + ((min + 1) / 60);
}

void taskNTPClient(void* parameter) {

  resetRTC();
  for (;;) {
    getTime();

    static bool bFirstRun = true;

    if (rtcTime.tm_year + 1900 == 1970) {
      if (!bFirstRun) Serial.println("Failed to update the NPT Time - try again");
      updateNTPClient(5000);
      bFirstRun = false;
    }

    if (lastDay != rtcTime.tm_yday && rtcTime.tm_hour == 4) {
      lastDay = rtcTime.tm_yday;
      Serial.println("UpdateNTPLoop");
      updateNTPClient(5000);
    }

    // static unsigned long ulNTPShowTime = 60 * 1000;
    // if (millis() - ulNTPShowTime >= 60 * 1000) {
    //   showTime();
    //   ulNTPShowTime = millis();
    // }

    // static unsigned long ulNTPUpdateTime = 60 * 1000;
    // if (millis() - ulNTPUpdateTime >= 15 * 1000) {
    //   adjustSec(45);
    //   ulNTPUpdateTime = millis();
    // }

    vTaskDelay(calculateDelayInTicks(100));
  }
}

void updateNTPClient(uint16_t uiDelay) {
  resetRTC();
  if (WiFi.status() != WL_CONNECTED)
    WIFI_Connect();

  delay(uiDelay);

  configTime(0, 0, NTPServer.c_str());           // 0, 0 because we will use TZ in the next line
  setenv("TZ", TimeZoneInformation.c_str(), 1);  // Set environment variable with your time zone
  tzset();

  Serial.println("Try to update NTP Time");
  int iCountFail = 0;
  while (rtcTime.tm_year + 1900 == 1970 || iCountFail >= 60) {
    Serial.print(".");
    getTime();
    iCountFail++;
    delay(1000);
  }

  if (rtcTime.tm_year + 1900 > 1970) {
    Serial.println("Success");
    WIFI_Disconnect();
    bForceUpdateTime = true;
  } else {
    Serial.println("Failed");
    uiClockCall = ClockFunction::ClockSayNTPError;
  }
}

void resetRTC() {
  // Set the default time to January 1, 1970, 00:00:00
  timeval defaultTime;
  defaultTime.tv_sec = 0;   // Set the seconds to 0
  defaultTime.tv_usec = 0;  // Set the microseconds to 0

  // Set the system time
  if (settimeofday(&defaultTime, nullptr) == -1) {
    Serial.println("Failed to set system time.");
  } else {
    Serial.println("System time has been set to January 1, 1970, 00:00:00.");
  }
}

void adjustTime(const char* timeString) {
  // Get the current time
  struct timeval currentTime;
  gettimeofday(&currentTime, nullptr);


  // Parse the time string
  int day, hours, minutes;
  sscanf(timeString, "%d:%d:%d", &day, &hours, &minutes);

  Serial.println("Set time, hour: " + String(hours) + ", minute: " + String(minutes));

  // Add one day (24 hours) to the current time
  currentTime.tv_sec += day * 24 * 60 * 60;  // 24 hours * 60 minutes * 60 seconds

  // Convert timeval to time_t
  time_t currentTimeSeconds = currentTime.tv_sec;

  // Calculate the new time
  struct tm* timeInfo;
  timeInfo = localtime(&currentTimeSeconds);
  timeInfo->tm_hour = hours;
  timeInfo->tm_min = minutes;
  timeInfo->tm_sec = 0;

  // Update the time
  time_t newTime = mktime(timeInfo);
  struct timeval newTimeval;
  newTimeval.tv_sec = newTime;
  newTimeval.tv_usec = currentTime.tv_usec;

  if (settimeofday(&newTimeval, nullptr) == -1) {
    Serial.println("Failed to set system time.");
    uiClockCall = ClockFunction::ClockSayNTPError;
  } else {
    Serial.println("System time has been adjusted.");
  }

  showTime();
}

void adjustSec(int iSeconds) {
  // Get the current time
  struct timeval currentTime;
  gettimeofday(&currentTime, nullptr);

  // Add specified seconds to the current time
  currentTime.tv_sec += iSeconds;

  if (settimeofday(&currentTime, nullptr) == -1) {
    Serial.println("Failed to set system time.");
    uiClockCall = ClockFunction::ClockSayNTPError;
  }
}