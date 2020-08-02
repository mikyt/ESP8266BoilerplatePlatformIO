#include <ArduinoLog.h>       // https://github.com/thijse/Arduino-Log
#include <ArduinoOTA.h>       // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>      // https://github.com/esp8266/Arduino
#include <LittleFS.h>         // https://github.com/esp8266/Arduino
#include <SerialAndBuffer.h>  // ./lib/SerialAndBuffer
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager

#include "config.h"
#include "web_interface.h"

// The hostname of this device.
// It will be used:
// * to set the wifi hostname (that some routers then expose over DNS, e.g.
// hostname.fritz.box)
// * to set the mDNS hostname (with .local suffix. e.g., hostname.local)
// * as the access point name to configure the WiFi credentials (e.g.
// hostnameAP)
const char kHostName[] = "ESP8266BoilerplatePlatformIO";

// The current configuration.
// The values of this variable will be stored and available across reboots.
Config config;

// The destination for all logs.
std::unique_ptr<SerialAndBuffer> serial_and_buffer;

// The web server used for communicating with the user.
std::unique_ptr<WebInterface> web_interface;

// Prepares the timestamp prefix for the logging library.
void printTimestamp(Print* _logOutput) {
  char c[14];
  sprintf(c, "[%10lu] ", millis());
  _logOutput->print(c);
}

void setup() {
  // Set up serial port and wait until connected
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {
  }
  serial_and_buffer.reset(new SerialAndBuffer(&Serial));
  Log.begin(LOG_LEVEL_NOTICE, serial_and_buffer.get());
  Log.setPrefix(printTimestamp);

  // Go on a new line (in case there is some initial garbage on the serial
  // console).
  serial_and_buffer->println();
  Log.notice(F("Starting up.\n\n"));

  // Start filesystem support.
  Log.notice(F("Starting FS...\n"));
  LittleFS.begin();
  Log.notice(F("FS started.\n"));

  Log.notice(F("Loading config...\n"));
  config.load();
  Log.notice(F("Config loaded.\n"));

  {
    Log.notice(F("Connecting to the WiFi...\n"));

    // Explicitly set the WiFI mode: ESP defaults to STA+AP
    WiFi.mode(WIFI_STA);

    // Set the WiFi hostname.
    WiFi.hostname(kHostName);

    // WiFiManager, Local intialization. Once its business is done, there is no
    // need to keep it around
    WiFiManager wm;
    wm.setSaveConfigCallback([]() { config.setSaveFlag(); });

    // Set up custom parameters.
    WiFiManagerParameter customOTAPassword(
        "OTAPassword", "Password protecting future Over-The-Air updates.",
        config.OTAPassword.c_str(), kMaxOTAPasswordChars);
    wm.addParameter(&customOTAPassword);

    // Automatically connects to the WiFi using the saved credentials.
    // If the connection fails, it starts an access point with the specified
    // name, then goes into a blocking loop awaiting configuration and will
    // return a success result
    bool success;
    success = wm.autoConnect((String(kHostName) + "AP").c_str());

    if (!success) {
      Log.fatal(F("Failed to connect. Restarting the ESP\n"));
      ESP.restart();
    }
    Log.notice(F("Successfully connected to the WiFI\n"));

    // Extracting custom paramers values.
    config.OTAPassword = customOTAPassword.getValue();
  }

  // Set up an mDNS name for this device.
  // Only required if we are searching for other devices over mDNS.
  // Still useful in any case to make this device more easily detectable.
  Log.notice(F("Setting up mDNS...\n"));
  if ((!MDNS.begin(kHostName))) {
    Log.fatal(F("Error setting up mDNS responder. Restarting the ESP\n"));
    ESP.restart();
  }
  Log.notice(F("mDNS started\n"));

  // Support for Over-The-Air updates.
  Log.notice(F("Setting up OTA support...\n"));
  ArduinoOTA.setPassword(config.OTAPassword.c_str());
  ArduinoOTA.begin();
  Log.notice(F("OTA support set up completed.\n"));

  Log.notice(F("Setting up the web server...\n"));
  web_interface.reset(new WebInterface(80, serial_and_buffer->GetBuffer()));
  Log.notice(F("Web server setup completed...\n"));

  Log.notice(F("Setup completed.\n"));
}

void loop() {
  // Make sure we still have a WiFi connection
  if (!(WiFi.status() == WL_CONNECTED)) {
    Log.notice("No WiFi connection. Attempting to reconnect to the WiFi...");
    delay(1000);
    return;
  }

  // Check if there is any Over-The-Air update.
  ArduinoOTA.handle();

  // Save the configuration (if needed).
  config.handleSave();

  // Serve any pending HTTP traffic.
  web_interface->HandleClient();

  // TODO: put your main code here, to run repeatedly:
}
