#include <SD.h>
#include <credentials.h>
#include "Struct.h"

#define SD_CS 15
#define SPI_MOSI 7
#define SPI_MISO 5
#define SPI_SCK 6

File settingsFile;
extern float HomeOffset;
extern int AudioVolume;
extern int AudioHoursOn;
extern int AudioHoursOff;
extern int MinuteStepping;
extern String NTPServer;
extern String TimeZoneInformation;

const int MAX_LOG_SIZE = 10 * 1024 * 1024;  // Maximum log file size (10 MB)

extern Network networks[];

File logFile;                 // Array of file objects for the log files
int currentLogFileIndex = 0;  // Index of the current log file
int currentLogFileSize = 0;   // File size of the current log file
#define LOG_FILE_MAX_SIZE 10 * 1024 * 1024

void setup_SDCard() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("Failed to initialize SD card");
    ESP32Restart();
  }

  //deleteFile("/settings.ini");

  if (!SD.exists("/settings.ini")) {
    if (!generateSettings()) {
      Serial.println("Failed to generate settings.ini file");
      ESP32Restart();
    }
  }

  if (!readSettings()) {
    Serial.println("Failed to generate settings.ini file");
    ESP32Restart();
  }
}

bool readSettings() {
  settingsFile = SD.open("/settings.ini");
  if (!settingsFile) {
    Serial.println("Failed to open settings.ini file");
    ESP32Restart();
  }

  // Read parameters from the section
  String sHomeOffset = getValueFromSection("Clock", "HomeOffset");
  HomeOffset = sHomeOffset.toFloat();
  // Read parameters from the section
  String sAudioVolume = getValueFromSection("Clock", "AudioVolume");
  AudioVolume = sAudioVolume.toInt();
  // Read parameters from the section
  String sAudioHoursOn = getValueFromSection("Clock", "AudioHoursOn");
  AudioHoursOn = sAudioHoursOn.toInt();
  // Read parameters from the section
  String sAudioHoursOff = getValueFromSection("Clock", "AudioHoursOff");
  AudioHoursOff = sAudioHoursOff.toInt();
  // Read parameters from the section
  String sMinuteStepping = getValueFromSection("Clock", "MinuteStepping");
  MinuteStepping = sMinuteStepping.toInt();

  Serial.println("MinuteStepping" + MinuteStepping);


  parseNetworkConfig();

  NTPServer = getValueFromSection("NTPClient", "NTPServer");
  TimeZoneInformation = getValueFromSection("NTPClient", "TimeZoneInformation");

  settingsFile.close();
  return true;
}

bool generateSettings() {
  settingsFile = SD.open("/settings.ini", FILE_WRITE);
  if (!settingsFile) {
    Serial.println("Failed to create settings.ini file");
    return false;
  }

  String data = "[WIFI]\n";
  // Construct the data string using mySSIDs and myPasswords arrays
  for (int i = 0; i < sizeof(mySSIDs) / sizeof(mySSIDs[0]); i++) {
    data += "WIFI_" + String(i + 1) + "=" + String(mySSIDs[i]) + "\n";
    data += "PASSWORD_" + String(i + 1) + "=" + String(myPasswords[i]) + "\n";
  }

  data += "\n";
  data += "[Clock]\n";
  data += "Homing offset to match the clock time to the real time.\n";
  data += "HomeOffset=2\n";
  data += "Audio Volume can be 0..22\n";
  data += "See: https://github.com/schreibfaul1/ESP32-audioI2S/wiki\n";
  data += "AudioVolume=12\n";
  data += "Hours to enable the talking clock 0 to 24\n";
  data += "AudioHoursOn=8\n";
  data += "Hours to disable the talking clock 0 to 24\n";
  data += "AudioHoursOff=20\n";
  data += "Minute stepping: 0 = disabled, 1 = every minute, 2 = every five minutes, 3 = every 15 minutes, 4 = every 30 minutes, 5 = every 60 minutes\n";
  data += "MinuteStepping=4\n";

  data += "\n";
  data += "[NTPClient]\n";
  data += "Example: https://werner.rothschopf.net/microcontroller/202103_arduino_esp32_ntp_en.htm\n";
  data += "NTPServer=time.google.com\n";
  data += "Time Zone with DST (Daylight saving time) UTC -1 'CET-1CEST,M3.5.0/02,M10.5.0/03'\n";
  data += "Time Zone without DST (Daylight saving time) UTC -1 'CET-1'\n";
  data += "TimeZoneInformation=CET-1CEST,M3.5.0/02,M10.5.0/03\n";

  data += "\n";
  settingsFile.print(data);
  settingsFile.close();

  Serial.println("Generated settings.ini file");
  return true;
}

bool deleteFile(String sFileName) {
  if (SD.exists(sFileName.c_str())) {
    if (SD.remove(sFileName.c_str())) {
      Serial.println("Deleted " + sFileName + " file");
      return true;
    } else {
      Serial.println("File " + sFileName + " was NOT deleted");
      return false;
    }
  } else {
    Serial.println("File " + sFileName + " does not exist");
    return true;
  }
}

String getValueFromSection(String section, String parameter) {
  settingsFile.seek(0);  // Reset file pointer to the beginning of the file
  bool sectionFound = false;
  while (settingsFile.available()) {
    String line = settingsFile.readStringUntil('\n');

    line.trim();

    if (line == ("[" + section + "]")) {
      sectionFound = true;
      continue;
    }

    if (sectionFound) {
      int separatorIndex = line.indexOf('=');
      if (separatorIndex != -1) {
        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);
        //Serial.println("KEY: " + key + " VALUE: " + value);
        if (key == parameter) {
          return value;
        }
      }
    }
  }
  return "";  // Return empty string if parameter not found
}

void parseNetworkConfig() {
  int index = 0;
  int maxIterations = 10;  // Maximum iterations allowed
  int numNetworks = 0;
  while (index < maxIterations) {
    String ssid_key = "WIFI_" + String(index + 1);
    String password_key = "PASSWORD_" + String(index + 1);

    String ssid = getValueFromSection("WIFI", ssid_key);
    String password = getValueFromSection("WIFI", password_key);

    if (ssid == "" || password == "") {
      // Reached the end of configuration or no parameter found
      if (index == 0) {
        // No parameters found, restart ESP32
        ESP32Restart();
      }
      break;
    }

    // Copy SSID and password into network structure
    ssid.toCharArray(networks[numNetworks].ssid, SSID_BUFFER_SIZE);
    password.toCharArray(networks[numNetworks].password, PASSWORD_BUFFER_SIZE);

    numNetworks++;
    index++;
  }
}