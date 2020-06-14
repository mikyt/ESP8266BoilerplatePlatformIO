#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <LittleFS.h>

// The name and expected sizeof the config file on the internal file system.
const char kConfigFile[] = "/config.json";
const size_t kMaxConfigFileSizeBytes = 1024;
const size_t kMaxOTAPasswordChars = 64;

// Structure containing the configuration loaded from the internal flash.
struct Config {
  // The password to authorize Over-The-Air updates.
  String OTAPassword;

  String toString() const {
    String result = "";
    // Let's not show this config entry, given that it is a password.
    result += "OTAPassword: <hidden>";
    return result;
  }

  // Loads the config from the configuration file.
  // After its execution the `config` global variable will be properly
  // initialized, either with values from the file (if possible) or with default
  // values/
  void load() {
    File configFile = LittleFS.open(kConfigFile, "r");

    StaticJsonDocument<kMaxConfigFileSizeBytes> jsonConfig;
    DeserializationError error = deserializeJson(jsonConfig, configFile);
    if (error) {
      Log.error(F("Failed to deserialize configuration file.\n"));
    }

    // Initialize the Config object, with values from the json file or default
    // values.
    this->OTAPassword = (jsonConfig["OTAPassword"] | "");

    configFile.close();
    Log.notice(F("\n"
                 "--------------------------\n"
                 "| The live configuration |\n"
                 "--------------------------\n"
                 "%s\n"
                 "--------------------------\n"),
               this->toString().c_str());
  }

  // Sets the flag that will trigger a save of the configuration.
  // The configuration will actually be saved after handleSave() has been called
  // as well.
  void setSaveFlag() { should_save_ = true; }

  // Save the configuration if the appropriate flag has been set with
  // setSaveFlag().
  void handleSave() {
    if (should_save_) {
      saveInternal();
    }
  }

 private:
  // Saves the configuration on the flash memory.
  bool saveInternal() {
    Log.notice(F("Saving configuration to file...\n"));
    // This is reset immediately, so that in case of failure there isn't
    // an uncontrolled number of failed attempts.
    should_save_ = false;

    LittleFS.remove(kConfigFile);

    File file = LittleFS.open(kConfigFile, "w");
    if (!file) {
      Log.error(F("Failed to create the config file\n"));
      return false;
    }

    StaticJsonDocument<kMaxConfigFileSizeBytes> jsonConfig;

    jsonConfig["OTAPassword"] = this->OTAPassword;

    if (serializeJson(jsonConfig, file) == 0) {
      Log.error(F("Failed to write the config to file"));
      return false;
    }

    file.close();
    Log.notice(F("Configuration saved to file.\n"));
    return true;
  }

  bool should_save_ = false;
};

#endif  // CONFIG_H