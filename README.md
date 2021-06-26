
# IotSprinklerRelayEsp8266-12e

**UPDATE:** Newly outfitted with a ham-handed web server implementation that checks for browser
requests within the control loop.  Not great/secure, especially when paired with a serverless cloud API,
it's a nice add for local testing, just lob a request at the specific device like so: 

`http://<<<device ip>>>?z=1&d=100000`

where _**z=zone number**_ and _**d=duration**_ (in milliseconds)

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
JSON relay instruction as payload.

For minimal build configuration, just copy `./versions/main.cpp.<<<config option>>>` to `./src/main.cpp`
and build using the Platformio profile of choice.

The api can run securely without the need to manage certificates behind a firewall or
with minimal configuration on a serverless instance in the cloud.

---

### OTA

The `main.cpp.combo.ota` variant supports pushing updates remotely using
`LittleFS.h` and can be readily configured to fit existing pipeline deployments, or with little/no
code changes this variant support manual pushes via default platformio profile config.

**NOTE:** The latter push is likely to fail with `[ERROR]: No Answer` message due 
to the loop cycle sleep which is included to cut down on API calls, particularly important for
serverless API configurations, where continuous API requests can result in much higher
server costs, unnecessarily.

If this is not a consideration for the implementation, it should be resolved by defining 
`API_SLEEP` period to below that of the default timeout(10 s) or removing the sleep at the 
loop's end, altogether.