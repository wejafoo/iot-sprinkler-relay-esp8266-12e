

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifndef STASSID
	#define	STASSID	"<<< YOUR NETWORK SSID >>>"
	#define	STAPSK	"<<< YOUR NETWORK PASSWORD >>>"
#endif
#define PODID		1
#define API_HOST	"<<< YOUR API HOSTNAME >>>"
#define API_PORT	"<<< YOUR API PORT >>>"

// Gotta be a better way to silence spiff deprecation warnings?!
// #ifdef USE_LittleFS
// 	#define	SPIFFS LittleFS
// #endif

const bool isDebug	= true;
const bool z3		= true;

const int zone1 = D1;		// gpio: 5	--
const int zone2 = D2;		// gpio: 4	--
const int zone3 = D3;		// gpio: 0	-- boot failure if pulled LOW
const int zone4 = D4;		// gpio: 2 	-- pin is high on BOOT, boot failure if pulled LOW
const int zone5 = D5;		// gpio: 14	-- "low-voltage" signal at BOOT
const int zone6 = D6;		// gpio: 12	-- "low-voltage" signal at BOOT
const int zone7 = false;	// gpio: 13	-- pin is high at BOOT
const int zone8 = false;	// gpio: 15	-- boot failure if pulled HIGH

const char *	ssid		= STASSID;
const char *	pwd			= STAPSK;
const int		podId		= PODID;
const String	host		= API_HOST;
const int		port		= API_PORT;
const int		sleep		= 3600000;
const int		sleepOnTest	= 2500;
const int		sleepOnDoh	= 10000;

String		deviceMacId	= "";
WiFiClient	client;

// Todo: Add builtin button reference using ' ' on running unit to claw back TX serial log for troubleshooting quad relays.

void bootTest() {
	if (zone1) {
		delay(sleepOnTest);
		Serial.println("zone1> ON(low)");
		digitalWrite(zone1, LOW);
		delay(sleepOnTest);
		Serial.println("zone1> OFF(high)");
		digitalWrite(zone1, HIGH);
	}
	if (zone2) {
		delay(sleepOnTest);
		Serial.println("zone2> ON(low)");
		digitalWrite(zone2, LOW);
		delay(sleepOnTest);
		Serial.println("zone2> OFF(high)");
		digitalWrite(zone2, HIGH);
	}
	if (z3) {									// boolean test doesn't work for zone3(a.k.a int "0", a.k.a. "false")
		delay(sleepOnTest);
		Serial.println("zone3> ON(low)");
		digitalWrite(zone3, LOW);
		delay(sleepOnTest);
		Serial.println("zone3> OFF(high)");
		digitalWrite(zone3, HIGH);
	}
	if (zone4) {
		delay(sleepOnTest);
		Serial.println("zone4> ON(low)");
		digitalWrite(zone4, LOW);
		delay(sleepOnTest);
		Serial.println("zone4> OFF(high)");
		digitalWrite(zone4, HIGH);
	}
	if (zone5) {
		delay(sleepOnTest);
		Serial.println("zone5> ON(low)");
		digitalWrite(zone5, LOW);
		delay(sleepOnTest);
		Serial.println("zone5> OFF(high)");
		digitalWrite(zone5, HIGH);
	}
	if (zone6) {
		delay(sleepOnTest);
		Serial.println("zone6> ON(low)");
		digitalWrite(zone6, LOW);
		delay(sleepOnTest);
		Serial.println("zone6> OFF(high)");
		digitalWrite(zone6, HIGH);
	}
	if (zone7) {
		delay(sleepOnTest);
		Serial.println("zone7> ON(low)");
		digitalWrite(zone7, LOW);
		delay(sleepOnTest);
		Serial.println("zone7> OFF(high)");
		digitalWrite(zone7, HIGH);
	}
	if (zone8) {
		delay(sleepOnTest);
		Serial.println("zone8> ON(low)");
		digitalWrite(zone8, LOW);
		delay(sleepOnTest);
		Serial.println("zone8> OFF(high)");
		digitalWrite(zone8, HIGH);
	}
}


void setup() {
	Serial.begin(115200);
	if (zone1) {
		digitalWrite(zone1, HIGH);			// write HIGH before setting pinMode to avoid flapping the solenoid at boot
		pinMode(zone1, OUTPUT);
	}
	if (zone2) {
		digitalWrite(zone2, HIGH);
		pinMode(zone2, OUTPUT);
	}
	if (z3) {								// boolean test doesn't work for zone3(a.k.a int "0", a.k.a. "false")
		digitalWrite(zone3, HIGH);
		pinMode(zone3, OUTPUT);
	}
	if (zone4) {
		digitalWrite(zone4, HIGH);
		pinMode(zone4, OUTPUT);
	}
	if (zone5) {
		digitalWrite(zone5, HIGH);
		pinMode(zone5, OUTPUT);
	}
	if (zone6) {
		digitalWrite(zone6, HIGH);
		pinMode(zone6, OUTPUT);
	}
	if (zone7) {
		digitalWrite(zone7, HIGH);
		pinMode(zone7, OUTPUT);
	}
	if (zone8) {
		digitalWrite(zone8, HIGH);
		pinMode(zone8, OUTPUT);
	}
	WiFi.mode(WIFI_STA);					// allows only WiFi connects
	deviceMacId	= WiFi.macAddress();
	WiFi.begin(ssid, pwd);
	while ( WiFi.status() != WL_CONNECTED ) {
		if (isDebug) Serial.print(".");
		delay(1000);
	}

	Serial.println( "WiFi connected!");
	Serial.print("IP address: \n");
	Serial.println(WiFi.localIP());
	Serial.print("Hardware MAC address: \n");
	Serial.println(WiFi.macAddress().c_str());

	bootTest();
}


void loop() {
	
	client.setTimeout(20000);
	if ( client.connect(host, port) ) {
		if (isDebug) Serial.printf(
			"connected %s as %s ... continuing",
			WiFi.macAddress().c_str(),
			WiFi.localIP().toString().c_str()
		);
	} else {
		if (isDebug) Serial.println("not connected... retrying");
		delay(sleepOnDoh);
		return;
	}

	client.printf("GET /api/pod/%d?device=%s HTTP/1.1", podId, deviceMacId.c_str());
	client.printf("\r\nHost: %s\r\nReferer: %s\r\nAccept: application/json\r\nConnection: close\r\n\r\n", host.c_str(), deviceMacId.c_str());

	while ( client.connected() || client.available() ) {
		if ( client.available() ) {
			String line = client.readStringUntil('\n');
			if (line == "\r") break;
		}
	}

	DynamicJsonDocument doc(2560);				// FYI, arduinojson.org/v6/assistant way underestimated with 256
	DeserializationError error = deserializeJson(doc, client);
	if ( error ) {
		if (isDebug) Serial.printf("????????????????????? %s", error.c_str());
		client.stop();
		delay(sleepOnDoh);		// Override the full loop cycle and try to connect again sooner... not too soon though
		return;
	}

	const JsonObject	podObj	= doc.as<JsonObject>();
	const int			pod		= podObj["pod"];
	if ( pod == podId ) {
		JsonObject zoneObj	= podObj["payload"].as<JsonObject>();
		if ( zoneObj.containsKey("1")||zoneObj.containsKey("2")||zoneObj.containsKey("3")||zoneObj.containsKey("4")) {
			if (zoneObj.containsKey("1")) {
				int duration = zoneObj["1"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone1> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone1, duration/60000);
					digitalWrite(zone1, LOW);
					delay(duration);
					Serial.printf("\n\tZone1> OFF(active high(~(+4.7v))");
					digitalWrite(zone1, HIGH);
				}
			}
			if (zoneObj.containsKey("2")) {
				int duration = zoneObj["2"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone2> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone2, duration/60000);
					digitalWrite(zone2, LOW);
					delay(duration);
					Serial.printf("\n\tZone2> OFF(active high(~(+4.7v))");
					digitalWrite(zone2, HIGH);
				}
			}
			if (zoneObj.containsKey("3")) {
				int duration = zoneObj["3"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone3> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone3, duration/60000);
					digitalWrite(zone3, LOW);
					delay(duration);
					Serial.printf("\tZone3> OFF(active high(~(+4.7v))");
					digitalWrite(zone3, HIGH);
				}
			}
			if (zoneObj.containsKey("4")) {
				int duration = zoneObj["4"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone4> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone4, duration/60000);
					digitalWrite(zone4, LOW);
					delay(duration);
					Serial.printf("\n\tZone4> OFF(active high(~(+4.7v))");
					digitalWrite(zone4, HIGH);
				}
			}
			if (zoneObj.containsKey("5")) {
				int duration = zoneObj["5"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone5> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone5, duration/60000);
					digitalWrite(zone5, LOW);
					delay(duration);
					Serial.printf("\n\tZone5> OFF(active high(~(+4.7v))");
					digitalWrite(zone5, HIGH);
				}
			}
			if (zoneObj.containsKey("6")) {
				int duration = zoneObj["6"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone6> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone6, duration/60000);
					digitalWrite(zone6, LOW);
					delay(duration);
					Serial.printf("\n\tZone6> OFF(active high(~(+4.7v))");
					digitalWrite(zone6, HIGH);
				}
			}
			if (zoneObj.containsKey("7")) {
				int duration = zoneObj["7"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone7> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone7, duration/60000);
					digitalWrite(zone7, LOW);
					delay(duration);
					Serial.printf("\n\tZone7> OFF(active high(~(+4.7v))");
					digitalWrite(zone7, HIGH);
				}
			}
			if (zoneObj.containsKey("8")) {
				int duration = zoneObj["8"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone8> gpio: %i\tON(active low(~(-2.2v)): %.2d minutes", zone8, duration/60000);
					digitalWrite(zone8, LOW);
					delay(duration);
					Serial.printf("\n\tZone8> OFF(active high(~(+4.7v))");
					digitalWrite(zone8, HIGH);
				}
			}
		} else {
			if (isDebug) Serial.println("no instructions in pod1 object\n...  ZzZzZzZzZzZzZzZzZzZz ...");
		}
	} else {
		if (isDebug) Serial.println("nothing doing\n...  ZzZzZzZzZzZzZzZzZzZz  ...");
	}
	client.stop();
	if (isDebug) Serial.println("...  ZzZzZzZzZzZzZzZzZzZz  ...");
	delay(sleep);
}



// Secure Web Client
/*
	if (apiIsTest) {
		host = HOST_TEST;
		port = PORT_TEST;
		WiFiClientSecure client2;		// Todo: Make WiFiClient conditionally inherit WifiClientSecure for non-local
		client2.setInsecure();			// Skip surprisingly tedious verification for now |:^o Todo: consider SSL fingerprint impl
	}
*/

