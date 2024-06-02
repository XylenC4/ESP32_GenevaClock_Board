#include <WiFi.h>
#include <WiFiClient.h>
#include "Struct.h"

extern String NTPServer;
String hostname = "GenevaTalkingClock";

// Array of networks
Network networks[MAX_NETWORKS];  // Assuming a maximum of 10 networks

void WIFI_Connect() {
  Serial.println("Scanning for networks...");
  int numSsid = WiFi.scanNetworks();

  if (numSsid == 0) {
    Serial.println("No networks found.");
    ESP32Restart();
    return;
  }

  Serial.print(numSsid);
  Serial.println(" networks found.");

  int bestNetworkIndex = -1;
  int bestRSSI = -100;  // Initialize to a very low value

  for (int i = 0; i < numSsid; i++) {
    String ssid = WiFi.SSID(i);
    Serial.print("Network ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(ssid);

    for (int j = 0; j < MAX_NETWORKS; j++) {
      if (ssid.equals(networks[j].ssid)) {
        int rssi = WiFi.RSSI(i);
        Serial.print("RSSI: ");
        Serial.println(rssi);
        if (rssi > bestRSSI) {
          bestRSSI = rssi;
          bestNetworkIndex = j;
        }
      }
    }
  }

  if (bestNetworkIndex == -1) {
    Serial.println("No matching networks found in the list.");
    uiClockCall = ClockFunction::ClockSayWIFINotAvailable;
    ESP32Restart();
    return;
  }

  Serial.print("Connecting to the best network: ");
  Serial.println(networks[bestNetworkIndex].ssid);
  WiFi.setHostname(hostname.c_str());  //define hostname
  WiFi.begin(networks[bestNetworkIndex].ssid, networks[bestNetworkIndex].password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi, restart.");
    uiClockCall = ClockFunction::ClockSayWIFILoginFailed;
    ESP32Restart();
  }

  if (!pingServer()) {
    Serial.println("\nFailed to connect to the NTP Server, restart.");
    uiClockCall = ClockFunction::ClockSayNTPError;
    ESP32Restart();
  }
}

void WIFI_Disconnect() {
  WiFi.disconnect(true);  // Disconnect WiFi, including any persistent connection
  Serial.println("WiFi disconnected");
}

bool pingServer() {
  WiFiClient client;
  Serial.print("Try to connect to server: ");
  Serial.println(NTPServer);

  bool bConnected = false;
  for (int i = 0; i < 5; i++) {
    Serial.print(".");
    if (client.connect(NTPServer.c_str(), 80)) {
      Serial.println("Connected!");
      client.stop();
      return true;
      break;
    } else {
      client.stop();
      delay(500);
    }
  }

  Serial.println("Failed to connect to server");
  return false;
}