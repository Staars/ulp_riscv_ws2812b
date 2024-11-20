| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

# Failed attempt to run WS2812b LED's with the ULP

It seems, the ULP of the ESP32-S3 is a tiny bit to slow.
A short high pulse would be needed to transfer a zero-bit but this does not work. Maybe it is about 100 nanoseconds too long.
As a result the ULP code can only turn on every pixel full white, when used in Tasmota using Berrys ULP module. The C-part in this repository does nothing useful.
