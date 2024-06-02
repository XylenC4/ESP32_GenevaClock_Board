#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "SPI.h"
#include "FS.h"
#include "Ticker.h"

// Digital I/O used
#define SD_CS 15
#define SPI_MOSI 7
#define SPI_MISO 5
#define SPI_SCK 6
#define I2S_DOUT 47
#define I2S_BCLK 48
#define I2S_LRC 45


#define BRIGHTNESS_AUDIO_LIMIT 25

Audio audio;
extern tm rtcTime;
extern int Brightness;

int AudioVolume = 10;
int AudioHoursOn = 8;
int AudioHoursOff = 19;
int MinuteStepping = 1;

bool f_time = false;
int8_t timefile = -1;
char chbuf[100][100];
char chTempBuf[100];
int chbufIndex = 0;
int chbufNextIndex = 0;


void taskTalkingClock(void* parameter) {
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(AudioVolume);  // 0...21

  for (;;) {
    audio.loop();

    static int iTalkingClockLastMin = rtcTime.tm_min;
    if (iTalkingClockLastMin != rtcTime.tm_min && Brightness >= BRIGHTNESS_AUDIO_LIMIT && rtcTime.tm_hour >= AudioHoursOn && rtcTime.tm_hour < AudioHoursOff) {
      TalkingClockStepping();
    }
    iTalkingClockLastMin = rtcTime.tm_min;

    ClockCall();

    if (!chbuf[chbufIndex][0] == '\0') {
      if (SD.exists(chbuf[chbufIndex])) {
        //Serial.println(String("Start: ") + String(chbufIndex) + String("; Path: ") + String(chbuf[chbufIndex]));
        audio.connecttoFS(SD, chbuf[chbufIndex]);
        // Reset chbuf to NULL
        chbuf[chbufIndex][0] = '\0';
      } else {
        //Serial.println(String("File does not exist: ") + String(chbufIndex) + String("; Path: ") + String(chbuf[chbufIndex]));
        // Reset chbuf to NULL
        chbuf[chbufIndex][0] = '\0';
        incrementIndex(&chbufIndex);
      }
    }
    vTaskDelay(calculateDelayInTicks(1));
  }
}

void PlayHour(int iHour) {
  sprintf(chTempBuf, "/voice/Hour_%02d.mp3", iHour);
  AddToBuf(chTempBuf);
}

void PlayMinute(int iMinute) {
  sprintf(chTempBuf, "/voice/Minute_%02d.mp3", iMinute);
  AddToBuf(chTempBuf);
}

void audio_eof_mp3(const char* info) {  //end of file
  //Serial.println("MP3: End: " + String(chbufIndex));
  incrementIndex(&chbufIndex);
  //Serial.println("MP3: ++: " + String(chbufIndex));
}

void AddToBuf(char chPath[100]) {
  strcpy(chbuf[chbufNextIndex], chPath);
  Serial.println(String("MP3: AddBuf: ") + String(chbufNextIndex) + String("; Path: ") + String(chbuf[chbufNextIndex]));
  incrementIndex(&chbufNextIndex);
}

void incrementIndex(int* chbufIndex) {
  (*chbufIndex)++;
  if (*chbufIndex >= 100)
    *chbufIndex = 0;
}

void TalkingClockStepping() {
  switch (MinuteStepping) {
    case 1:
      PlayHour(rtcTime.tm_hour);
      if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
      Serial.println("Play 1 minute");
      break;
    case 2:
      if (rtcTime.tm_min % 5 == 0) {
        PlayHour(rtcTime.tm_hour);
        if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
        Serial.println("Play 5 minutes");
      }
      break;
    case 3:
      if (rtcTime.tm_min % 15 == 0) {
        PlayHour(rtcTime.tm_hour);
        if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
        Serial.println("Play 15 minutes");
      }
      break;
    case 4:
      if (rtcTime.tm_min % 30 == 0) {
        PlayHour(rtcTime.tm_hour);
        if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
        Serial.println("Play 30 minutes");
      }
      break;
    case 5:
      if (rtcTime.tm_min == 0) {
        PlayHour(rtcTime.tm_hour);
        Serial.println("Play 60 minutes");
      }
      break;
    default:
      // statements
      break;
  }
}

void ClockCall() {
  static int iOldCall = 0;
  if (iOldCall != uiClockCall) {
    iOldCall = uiClockCall;
    return;
  }

  switch (uiClockCall) {
    case ClockFunction::ClockSayTime:
      static long oldMillis = 0;
      if (millis() - oldMillis > 30 * 1000 || oldMillis == 0) {
        PlayHour(rtcTime.tm_hour);
        if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
      } else {
        sprintf(chTempBuf, "/voice/Outtakes_%02d.mp3", random(1, 5));
        AddToBuf(chTempBuf);
      }
      oldMillis = millis();
      break;
    case ClockFunction::ClockSayVolume:
      AudioVolume = (AudioVolume + 3) % 21;
      audio.setVolume(AudioVolume);  // 0...21
      PlayHour(rtcTime.tm_hour);
      if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
      Serial.println("Inc audio volume: " + String(AudioVolume));
      break;
    case ClockFunction::ClockSayMinuteStepping:
      PlayHour(rtcTime.tm_hour);
      if (rtcTime.tm_min != 0) PlayMinute(rtcTime.tm_min);
      MinuteStepping = (MinuteStepping + 1) % 5;
      Serial.println("SayMinuteStepping");
      break;
    case ClockFunction::ClockSayNTPError:
      strcpy(chTempBuf, "/voice/NTPError.mp3");
      AddToBuf(chTempBuf);
      break;
    case ClockFunction::ClockSayResetSettings:
      strcpy(chTempBuf, "/voice/ResetSettings.mp3");
      AddToBuf(chTempBuf);
      break;
    case ClockFunction::ClockSayWIFILoginFailed:
      strcpy(chTempBuf, "/voice/WIFILoginFailed.mp3");
      AddToBuf(chTempBuf);
      break;
    case ClockFunction::ClockSayWIFINotAvailable:
      strcpy(chTempBuf, "/voice/WIFINotAvailable.mp3");
      AddToBuf(chTempBuf);
      break;
    case ClockFunction::ClockSayAPDS9960Error:
      strcpy(chTempBuf, "/voice/APDS9960Error.mp3");
      AddToBuf(chTempBuf);
      break;
    case ClockFunction::ClockSayHomingError:
      strcpy(chTempBuf, "/voice/HomingError.mp3");
      AddToBuf(chTempBuf);
      break;
    default:
      // statements
      break;
  }
  uiClockCall = ClockFunction::ClockNone;
}
