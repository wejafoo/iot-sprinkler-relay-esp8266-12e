
# IotSprinklerRelayEsp8266-12e

Wi-Fi controller flash configuration that uses the following
platformio profile:

    platform    = espressif8266
    board       = nodemcuv2
    framework   = arduino

Configured for use with [micro-sprinkler-api](https://github.com/wejafoo/iot-sprinkler-relay-esp8266-12e)
as a subscription proxy to a GCP pubsub topic which carries JSON relay instruction as payload.

