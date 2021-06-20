
# IotSprinklerRelayEsp8266-12e

**UPDATE:** Newly outfitted with a ham-handed web server implementation that checks for browser
requests within the control loop.  Not great(or secure) when paired with a serverless cloud API,
it's a nice add for local testing.  Just lob a request like so to enable specific relays: 
`http://<<<device ip>>>?z=1&d=100000` , where _**z=zone number**_ and _**d=duration**_ (in milliseconds)

---

Wi-Fi controller flash configuration that uses either of the following
platformio profiles:

---

    platform    = espressif8266
    board       = nodemcuv2
    framework   = arduino

---

    platform    = espressif8266
    board       = esp12e
    framework   = arduino

---

Configured with [micro-sprinkler-api](https://github.com/wejafoo/iot-sprinkler-relay-esp8266-12e)
the solenoid relay can be triggered by a subscription proxy to a GCP pubsub topic which carries
JSON relay instruction as payload.  Conversely, the relays can be fired with a direct http request 
with a path similar to `/pod/1/zone/1/action/1`, where "pod" refers to the board, "zone" refers to the 
specific relay, and "action" which turns on or off the given relay.

For minimal build configuration, just copy main.<config option>.cpp to main.cpp and build using the 
preferred Platformio build profile.

The api can run securely without the need to manage certificates behind a firewall or with minimal configuration on a serverless
instance in the cloud.

