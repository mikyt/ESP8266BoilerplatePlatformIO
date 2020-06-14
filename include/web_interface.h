#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <ArduinoLog.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <SerialAndBuffer.h>
#include <WiFiManager.h>

class WebInterface {
 public:
  // Creates a WebInterface listening on the given port, and able to print out
  // the logs from the specified buffer.
  WebInterface(int port, LogBuffer* log_buffer)
      : webserver_(port), log_buffer_(log_buffer) {
    webserver_.on("/", std::bind(&WebInterface::WebHandleRoot, this));
    webserver_.on("/serialOut",
                  std::bind(&WebInterface::WebHandleSerialOut, this));
    webserver_.on("/resetresetreset",
                  std::bind(&WebInterface::WebHandleReset, this));
    webserver_.begin();
  }

  void HandleClient() { webserver_.handleClient(); }

 private:
  void WebHandleRoot() {
    Log.verbose("[WebInterface] Serving the home page.\n");
    const char* homepage = R"(
        <html>
            <head><title>Web interface</title></head>
            <body>
                <h1>Web interface</h1>
                <a href="/serialOut">Latest serial output console</a>
            </body>
        </html>)";
    webserver_.send(HTTP_CODE_OK, "text/html", homepage);
  }

  void WebHandleReset() {
    WiFiManager wm;
    wm.resetSettings();
    ESP.restart();
  }

  void WebHandleSerialOut() {
    Log.verbose("[WebInterface] Serving the serial logs.\n");
    const int buffer_size = log_buffer_->size();
    char* serial_out;
    serial_out = new char[buffer_size + 1];

    for (int i = 0; i < buffer_size; ++i) {
      const char character = (*log_buffer_)[i];
      serial_out[i] = character;
    }
    serial_out[buffer_size] = '\0';
    webserver_.send(HTTP_CODE_OK, "text/plain", serial_out);
  }

  ESP8266WebServer webserver_;
  LogBuffer* log_buffer_;
};

#endif  // WEBINTERFACE_H