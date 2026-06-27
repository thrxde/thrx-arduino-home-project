// Do not remove the include below
#include "main.h"


char pName[] = "thrx home project - mqtt";
char pVersion[] = "1.1.4";
byte mac[] = MAC_ADDRESS;     		//Ethernet shield mac address
const char *ip = IP_ADDRESS;       		//Ethernet shield ip address
const char *mqttServer = MQTT_SERVER_IP; //Openhab / Mosquitto  IP
char mqttClientName[]  = "arduino_1";
char topicStatus[]     = "arduino/1/status";
char topicLastWill[]   = "arduino/1/will";
char topicReset[]      = "arduino/1/reset";
char topicUptime[]     = "arduino/1/uptime";
char topicCommand[]    = "arduino/1/command";
char topicVersion[]    = "arduino/1/version";
char topicFreeram[]    = "arduino/1/freeram";
char topicDebug[]      = "arduino/1/debug";
unsigned long waitTime = 5000; // max mqtt transmit rate 5sec
unsigned long lastReconnectAttempt = 0;
unsigned long lastStatusPublish = 0;

// Debug mode: toggle via MQTT command "debug on" / "debug off"
// Default OFF — prevents Serial0 flooding that crashes ATmega16U2 USB bridge
volatile bool debugMode = false;

EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);
MqttHandler mqttHandler(mqttClient);


int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

/**
 * Hardware reset using AVR Watchdog Timer.
 * This properly resets ALL peripherals (Ethernet W5100, timers, etc.)
 * unlike the old software reset (jump to 0) which left hardware in
 * inconsistent state.
 */
void hardwareReset() {
	Serial.println(F(">>> Hardware reset via WDT"));
	Serial.flush();
	wdt_enable(WDTO_15MS);
	while (1) {} // wait for watchdog to fire
}

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	char message_buff[100];

	if (debugMode) {
		Serial.print(F("Message arrived: topic: "));
		Serial.println(topic);
		Serial.print(F("Length: "));
		Serial.println(length);
	}

	// Safely copy payload to buffer (max 99 chars)
	unsigned int copyLen = (length < 99) ? length : 99;
	for (unsigned int i = 0; i < copyLen; i++) {
		message_buff[i] = payload[i];
	}
	message_buff[copyLen] = '\0';

	if (debugMode) {
		Serial.print(F("Payload: "));
		Serial.println(message_buff);
	}

	// Handle commands
	if (strcmp(topic, topicCommand) == 0) {
		if (strcmp(message_buff, "reboot") == 0) {
			Serial.println(F("Reboot command received via MQTT"));
			mqttClient.publish(topicStatus, "rebooting");
			mqttClient.disconnect();
			delay(100);
			hardwareReset();
		} else if (strcmp(message_buff, "debug on") == 0) {
			debugMode = true;
			mqttClient.publish(topicDebug, "on");
			Serial.println(F("Debug mode ON"));
		} else if (strcmp(message_buff, "debug off") == 0) {
			debugMode = false;
			mqttClient.publish(topicDebug, "off");
			Serial.println(F("Debug mode OFF"));
		}
	}
}

void setup() {
	// Clear reset flags and disable watchdog early in setup
	// MCUSR must be cleared BEFORE wdt_disable() per AVR app note AVR132,
	// otherwise WDT reset flag persists and bootloader may hang
	MCUSR = 0;
	wdt_disable();

	// LED heartbeat — confirms ATmega2560 is alive even if USB bridge is wedged
	pinMode(13, OUTPUT);

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println(pName);
	Serial.print(F("Version: "));
	Serial.println(pVersion);
	delay(1000);

	IPAddress apip;
	if (apip.fromString(ip)) {
		Serial.print(F("Local IP address: "));
		Serial.println(apip);
	} else {
		Serial.println(F("UnParsable IP"));
	}
	Ethernet.begin(mac, apip);

	delay(1000);
	Serial.println();
	Serial.print(F("Local IP address: "));
	Serial.println(Ethernet.localIP());
	Serial.print(F("DNS IP address: "));
	Serial.println(Ethernet.dnsServerIP());
	Serial.print(F("Gateway IP address: "));
	Serial.println(Ethernet.gatewayIP());
	Serial.println();

	connectMqttServer();

	PowerSerial::setup(waitTime);

	// Enable watchdog with 8 second timeout
	// If loop() hangs for >8s, WDT triggers a full hardware reset
	wdt_enable(WDTO_8S);
	Serial.println(F("Watchdog enabled (8s timeout)"));
}

bool connectMqttServer() {
	if (!mqttClient.connected()) {
		if (debugMode) Serial.println(F("MQTT not connected"));
		unsigned long now = millis();
		if (now - lastReconnectAttempt > 5000) {
			if (debugMode) Serial.println(F("MQTT try reconnect"));
			lastReconnectAttempt = now;
			if (reconnect()) {
				Serial.println(F("MQTT reconnect successful"));
				lastReconnectAttempt = 0;
			}
		}
	} else {
		mqttClient.loop();
	}
	return mqttClient.connected();
}

boolean reconnect() {
	// connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
	if (mqttClient.connect(mqttClientName, MQTT_USER, MQTT_PASS, topicLastWill, 1, false, "disconnected")) {
		mqttClient.publish(topicLastWill, "connected");
		mqttClient.publish(topicVersion, pVersion);
		mqttClient.publish(topicDebug, debugMode ? "on" : "off");
		char uptimeBuf[12];
		ltoa(millis(), uptimeBuf, 10);
		mqttClient.publish(topicUptime, uptimeBuf);
		mqttClient.subscribe(topicCommand);
		Serial.print(F("MQTT subscribed to "));
		Serial.println(topicCommand);
	} else {
		Serial.println(F("MQTT connection failed"));
	}
	return mqttClient.connected();
}

// The loop function is called in an endless loop
void loop() {
	// Feed the watchdog - must be called within 8 seconds
	wdt_reset();

	// LED heartbeat — blinks every 1s to show ATmega2560 is alive
	static unsigned long lastBlink = 0;
	if (millis() - lastBlink > 1000) {
		lastBlink = millis();
		digitalWrite(13, !digitalRead(13));
	}

	// Maintain Ethernet stack — cleans up stale W5100 sockets
	Ethernet.maintain();

	if (!connectMqttServer()) {
		if (debugMode) Serial.println(F("MQTT not connected to server"));
		return;
	}

	// Preventive reset disabled in v1.1.4 — the 6h WDT reset was crashing
	// the ATmega16U2 USB bridge on restart. With serial flooding fixed (v1.1.3),
	// long uptimes should be stable. Monitor freeram for degradation instead.
	// if (millis() - bootTime >= 21600000UL) { ... }

	// Publish status at most every 5 seconds (not every loop iteration)
	if (millis() - lastStatusPublish >= 5000) {
		lastStatusPublish = millis();
		char uptimeBuf[12];
		ltoa(millis(), uptimeBuf, 10);
		mqttClient.publish(topicUptime, uptimeBuf);
		char ramBuf[8];
		itoa(freeRam(), ramBuf, 10);
		mqttClient.publish(topicFreeram, ramBuf);
		mqttClient.publish(topicReset, "false");
		mqttClient.publish(topicStatus, "online");

		// Debug: print waiting status only every 5s, not every loop
		if (debugMode) {
			if (PowerSerial::swu.getCount() >= 0)
				Serial.println(F("SWU: waiting for telegram"));
			if (PowerSerial::solar.getCount() >= 0)
				Serial.println(F("Solar: waiting for telegram"));
		}
	}

	if (PowerSerial::swu.getCount() >= 0){
		PowerSerial::swu.parseMe();
	}
	if (PowerSerial::solar.getCount() >= 0){
		PowerSerial::solar.parseMe();
	}

	if (PowerSerial::swu.getCount() < 0) {
		PowerSerial::swu.transmitDataToMqtt(mqttHandler);
	}
	if (PowerSerial::solar.getCount() < 0) {
		PowerSerial::solar.transmitDataToMqtt(mqttHandler);
	}
}
