

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifndef STASSID
	#define	STASSID	"<<< YOUR NETWORK SSID >>>"
	#define	STAPSK	"<<< YOUR NETWORK PASSWORD >>>"
#endif
#define PODID		1
#define API_HOST	"<<< YOUR API HOSTNAME >>>"
#define API_PORT	"<<< YOUR API PORT >>>"
#define API_SLEEP	15000			// 3600000 cloud api default: 5 mins;
#define ZONE_ON		LOW
#define ZONE_OFF	HIGH

const bool	isDebug	= true;
const int	zone1	= D1;			// 		-- gpio: 5	--
const int	zone2	= D2;			// 		-- gpio: 4	--
const int	zone3	= D3;			// 		-- gpio: 0	-- boot failure if pulled LOW
const bool	z3		= true;
const int	zone4	= D4;			// 		-- gpio: 2 	-- pin is high on BOOT, boot failure if pulled LOW
const int	zone5	= false;		// D5	-- gpio: 14	-- "low-voltage" signal at BOOT
const int	zone6	= false;		// D6	-- gpio: 12	-- "low-voltage" signal at BOOT
const int	zone7	= false;		// D7	-- gpio: 13	-- pin is high at BOOT
const int	zone8	= false;		// D8	-- gpio: 15	-- boot failure if pulled HIGH
const bool	zoneOn	= ZONE_ON;
const bool	zoneOff	= ZONE_OFF;

const char *	ssid		= STASSID;
const char *	pwd			= STAPSK;
const int		podId		= PODID;
const String	host		= API_HOST;
const int		port		= API_PORT;
const int		sleep		= API_SLEEP;
const int		sleepOnTest	= 2500;
const int		sleepOnDoh	= 10000;

String		deviceMacId	= "";
WiFiClient	client;

ESP8266WebServer server(80);


void setup() {

	Serial.begin(115200);
	if (zone1) {
		digitalWrite(zone1, zoneOff);	// write HIGH before setting pinMode to avoid flapping the solenoid at boot
		pinMode(zone1, OUTPUT);
	}
	if (zone2) {
		digitalWrite(zone2, zoneOff);
		pinMode(zone2, OUTPUT);
	}
	if (z3) {							// boolean test doesn't work for zone3(a.k.a int "0", a.k.a. "false")
		digitalWrite(zone3, zoneOff);
		pinMode(zone3, OUTPUT);
	}
	if (zone4) {
		digitalWrite(zone4, zoneOff);
		pinMode(zone4, OUTPUT);
	}
	if (zone5) {
		digitalWrite(zone5, zoneOff);
		pinMode(zone5, OUTPUT);
	}
	if (zone6) {
		digitalWrite(zone6, zoneOff);
		pinMode(zone6, OUTPUT);
	}
	if (zone7) {
		digitalWrite(zone7, zoneOff);
		pinMode(zone7, OUTPUT);
	}
	if (zone8) {
		digitalWrite(zone8, zoneOff);
		pinMode(zone8, OUTPUT);
	}

	Serial.println();
	Serial.println("configuring wifi access...connected");

	WiFi.mode(WIFI_STA);									// Allow only WiFi connects
	deviceMacId	= WiFi.macAddress();
	WiFi.begin(ssid, pwd);
	while ( WiFi.status() != WL_CONNECTED ) {
		if (isDebug) Serial.print(".");
		delay(1000);
	}

	Serial.print( "\nIP address: " );
	Serial.print( WiFi.localIP() );
	Serial.printf( "\tdevice MAC: %s\n", WiFi.macAddress().c_str());
	Serial.println( );

	bootTest();

	server.on("/", handleZoneOn );
	// server.on("/stat", returnStatus);

	Serial.print("HTTP server starting..." );
	server.begin();
	Serial.print("done\n" );
}


void loop() {

	client.setTimeout(20000);	// arbitrary duration, but semi-important for handling prolonged service outages

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

	DynamicJsonDocument doc(2560);					// FYI, arduinojson.org/v6/assistant way underestimated with 256
	DeserializationError error = deserializeJson(doc, client);
	if ( error ) {
		if (isDebug) Serial.printf("????????????????????? %s", error.c_str());
		client.stop();
		delay(sleepOnDoh);		// Override the full loop cycle and try to connect early and often to reestablish conn
		return;
	}

	const JsonObject	podObj	= doc.as<JsonObject>();
	const int			pod		= podObj["pod"];

	if ( pod == podId ) {

		JsonObject zoneObj	= podObj["payload"].as<JsonObject>();

		if (
				zoneObj.containsKey("1")	||
				zoneObj.containsKey("2")	||
				zoneObj.containsKey("3")	||
				zoneObj.containsKey("4")	||
				zoneObj.containsKey("5")	||
				zoneObj.containsKey("6")	||
				zoneObj.containsKey("7")	||
				zoneObj.containsKey("8")
				) {

			if (zoneObj.containsKey("1")) {
				int duration = zoneObj["1"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone1>\tON: %.2d minutes\t(gpio: %i -- active low( ~(-2.2v))", zone1, duration/60000);
					digitalWrite(zone1, LOW);
					delay(duration);
					Serial.printf("\n\tZone1>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone1);
					digitalWrite(zone1, HIGH);
				}
			}
			if (zoneObj.containsKey("2")) {
				int duration = zoneObj["2"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone2>\tON: %.2d minutes\t\t\t(gpio: %i -- active low(~(-2.2v))", zone2, duration/60000);
					digitalWrite(zone2, LOW);
					delay(duration);
					Serial.printf("\n\tZone2>\tOFF\t\t\t\t\t(gpio: %i -- active high(~(+4.7v))", zone1);
					digitalWrite(zone2, HIGH);
				}
			}
			if (zoneObj.containsKey("3")) {
				int duration = zoneObj["3"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone3>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone3, duration/60000);
					digitalWrite(zone3, LOW);
					delay(duration);
					Serial.printf("\n\tZone3>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone3);
					digitalWrite(zone3, HIGH);
				}
			}
			if (zoneObj.containsKey("4")) {
				int duration = zoneObj["4"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone4>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone4, duration/60000);
					digitalWrite(zone4, LOW);
					delay(duration);
					Serial.printf("\n\tZone4>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone4);
					digitalWrite(zone4, HIGH);
				}
			}
			if (zoneObj.containsKey("5")) {
				int duration = zoneObj["5"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone5>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone5, duration/60000);
					digitalWrite(zone5, LOW);
					delay(duration);
					Serial.printf("\n\tZone5>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone5);
					digitalWrite(zone5, HIGH);
				}
			}
			if (zoneObj.containsKey("6")) {
				int duration = zoneObj["6"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone6>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone6, duration/60000);
					digitalWrite(zone6, LOW);
					delay(duration);
					Serial.printf("\n\tZone6>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone6);
					digitalWrite(zone6, HIGH);
				}
			}
			if (zoneObj.containsKey("7")) {
				int duration = zoneObj["7"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone7>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone7, duration/60000);
					digitalWrite(zone7, LOW);
					delay(duration);
					Serial.printf("\n\tZone7>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone7);
					digitalWrite(zone7, HIGH);
				}
			}
			if (zoneObj.containsKey("8")) {
				int duration = zoneObj["8"].as<int>();
				if (duration > 0) {
					Serial.printf("\n\tZone8>\tON: %.2d minutes\t\t(gpio: %i -- active low(~(-2.2v))", zone8, duration/60000);
					digitalWrite(zone8, LOW);
					delay(duration);
					Serial.printf("\n\tZone8>\tOFF\t\t\t(gpio: %i -- active high(~(+4.7v))", zone8);
					digitalWrite(zone8, HIGH);
				}
			}
			if (isDebug) Serial.println("\n...  ZzZzZzZzZzZzZzZzZzZz  ...");
		} else {
			if (isDebug) Serial.println("no instructions in pod1 object\n...  ZzZzZzZzZzZzZzZzZzZz ...");
		}
	} else {
		if (isDebug) Serial.println("\nnothing doing\n...  ZzZzZzZzZzZzZzZzZzZz  ...");
	}
	client.stop();

	server.handleClient();				// Check for local web client requests before napping

	delay(sleep);
}


void returnStatus() {
	bool zone1IsOn  = digitalRead(zone1) == zoneOn;
	Serial.printf( " -- Zone #1 is %s", zone1IsOn ?"TRUE": "FALSE" );
	bool zone2IsOn  = digitalRead(zone2) == zoneOn;
	Serial.printf( " -- Zone #2 is %s", zone2IsOn ?"TRUE": "FALSE" );
	bool zone3IsOn  = digitalRead(zone3) == zoneOn;
	Serial.printf( " -- Zone #3 is %s", zone3IsOn ?"TRUE": "FALSE" );
	bool zone4IsOn  = digitalRead(zone4) == zoneOn;
	Serial.printf( " -- Zone #4 is %s", zone4IsOn ?"TRUE": "FALSE" );
	bool zone5IsOn  = digitalRead(zone5) == zoneOn;
	Serial.printf( " -- Zone #5 is %s", zone5IsOn ?"TRUE": "FALSE" );
	bool zone6IsOn  = digitalRead(zone6) == zoneOn;
	Serial.printf( " -- Zone #6 is %s", zone6IsOn ?"TRUE": "FALSE" );
	bool zone7IsOn  = digitalRead(zone7) == zoneOn;
	Serial.printf( " -- Zone #7 is %s", zone7IsOn ?"TRUE": "FALSE" );
	bool zone8IsOn  = digitalRead(zone8) == zoneOn;
	Serial.printf( " -- Zone #8 is %s", zone8IsOn ?"TRUE": "FALSE" );
	char statusReturn[800];
	int truncCount = snprintf(
		statusReturn, 150,	"{\n\tpod1: [\n\t\tzone1: %s\n\t\tzone2: %s,\n\t\tzone3: %s,\n\t\tzone4: %s,\n\t\tzone5: %s,\n\t\tzone6: %s,\n\t\tzone7: \"-1\",\n\t\tzone8: \"-1\"\n\t]\n}",
		zone1IsOn?"1": "0", zone2IsOn?"1": "0", zone3IsOn?"1": "0", zone4IsOn?"1": "0", zone5IsOn?"1": "0", zone6IsOn?"1": "0"
	);
	Serial.println("Truncation count: " + String(truncCount));
	server.send(200,"text/plain", statusReturn);
}


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


void SetPin(byte pinNumber, byte newPinState) {
    digitalWrite(pinNumber, newPinState);
    Serial.print("\nmanually setting pin#");
    Serial.print(pinNumber);
    Serial.print(" to: ");
    Serial.println(newPinState);

    returnStatus();
}


void handleZoneOn() {
	int zone;
	String z = server.arg("z");
	if 		(z == "1"){ zone = zone1; }
	else if (z == "2"){ zone = zone2; }
	else if (z == "3"){ zone = zone3; }
	else if (z == "4"){ zone = zone4; }
	else if (z == "5"){ zone = zone5; }
	else if (z == "6"){ zone = zone6; }
	else if (z == "7"){ zone = zone7; }
	else if (z == "8"){ zone = zone8; }
	else 			  { return;		  }
	SetPin(zone, zoneOn);
	delay(server.arg("d").toInt());
	SetPin(zone, zoneOff);
}
