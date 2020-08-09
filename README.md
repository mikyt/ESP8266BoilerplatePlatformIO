Boilerplate code for an ESP8266 PlatformIO project that already contains:

* Wifi management through a captive portal at the first use (via the WifiManager library)
* Ready for Over-The-Air (OTA) updates
* Sets the hostname, and the mDNS name
* Configuration file saved on the internal flash (including the WiFi config and the OTA password)
* Logging on the serial port and over HTTP (via a circular buffer, to allow remote debugging)

To use OTA updates in PlatformIO:
1. Enable them in `platformio.ini`
2. Open a PlatformIO terminal
3. Set your password: `$Env:PLATFORMIO_UPLOAD_FLAGS="--auth=YourPasswordHere"`
4. Upload the code (re-use as many times as needed): `platformio run --target upload`