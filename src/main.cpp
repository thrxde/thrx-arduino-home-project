// Do not remove the include below
#include "main.h"


char pName[] = "thrx home project - mqtt";
char pVersion[] = "1.1.1";
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
unsigned long waitTime = 5000; // max mqtt transmit rate 5sec
unsigned long resetTime = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long lastStatusPublish = 0;

EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);
MqttHandler mqttHandler(mqttClient);


/**
 * Hardware reset using AVR Watchdog Timer.
 * This properly resets ALL peripherals (Ethernet W5100, timers, etc.)
 * unlike the old software reset (jump to 0) which left hardware in
 * inconsistent state.
 */
void hardwareReset() {
	Serial.println(">>> Hardware reset via WDT");
	Serial.flush();
	wdt_enable(WDTO_15MS);
	while (1) {} // wait for watchdog to fire
}

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	char message_buff[100];
	
	Serial.print("Message arrived: topic: ");
	Serial.println(topic);
	Serial.print("Length: ");
	Serial.println(length);
	
	// Safely copy payload to buffer (max 99 chars)
	unsigned int copyLen = (length < 99) ? length : 99;
	for (unsigned int i = 0; i < copyLen; i++) {
		message_buff[i] = payload[i];
	}
	message_buff[copyLen] = '\0';
	
	Serial.print("Payload: ");
	Serial.println(message_buff);

	// Handle reboot command
	if (strcmp(topic, topicCommand) == 0 && strcmp(message_buff, "reboot") == 0) {
		Serial.println("Reboot command received via MQTT");
		mqttClient.publish(topicStatus, "rebooting");
		mqttClient.disconnect();
		delay(100);
		hardwareReset();
	}
}

void setup() {
	// Clear reset flags and disable watchdog early in setup
	// MCUSR must be cleared BEFORE wdt_disable() per AVR app note AVR132,
	// otherwise WDT reset flag persists and bootloader may hang
	MCUSR = 0;
	wdt_disable();

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println(pName);
	Serial.print("Version: ");
	Serial.println(pVersion);
	delay(1000);

    IPAddress apip;
	if (apip.fromString(ip)) { // try to parse into the IPAddress
    	Serial.print("Local IP address: ");
		Serial.println(apip); // print the parsed IPAddress 
	} else {
		Serial.println("UnParsable IP");
	}
   	Ethernet.begin(mac, apip); //configure manually

	delay(1000);
	Serial.println();
   	Serial.print("Local IP address: ");
	Serial.println(Ethernet.localIP());
   	Serial.print("DNS IP address: ");
	Serial.println(Ethernet.dnsServerIP());
   	Serial.print("Gateway IP address: ");
	Serial.println(Ethernet.gatewayIP());
 	Serial.println();

  	connectMqttServer();

	PowerSerial::setup(waitTime);

	// Enable watchdog with 8 second timeout
	// If loop() hangs for >8s, WDT triggers a full hardware reset
	wdt_enable(WDTO_8S);
	Serial.println("Watchdog enabled (8s timeout)");
}

bool connectMqttServer() {
   // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this,
    // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
    // PubSubClient.h
 	// Establishing the connection to MQTT server if it is not open.
	if (!mqttClient.connected()) {
	    Serial.println("MQTT not connected");
		unsigned long now = millis();
		Serial.print("MQTT not connected now: ");
		Serial.println(now);
    	if (now - lastReconnectAttempt > 5000) {
     	    Serial.println("MQTT try reconnect");
			lastReconnectAttempt = now;
			// Attempt to reconnect
			if (reconnect()) {
         	    Serial.println("MQTT reconnect successful");
				lastReconnectAttempt = 0;
			}
		}
		
	} else {
   	    Serial.println("MQTT loop");
		mqttClient.loop();
	}
	return mqttClient.connected();
}

boolean reconnect() {
 	// connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
	if (mqttClient.connect(mqttClientName, MQTT_USER, MQTT_PASS, topicLastWill, 1, false, "disconnected")) {
		mqttClient.publish(topicLastWill, "connected");
		mqttClient.publish(topicVersion, pVersion);
		char uptimeBuf[12];
		ltoa(millis(), uptimeBuf, 10);
		mqttClient.publish(topicUptime, uptimeBuf);
		// Subscribe to messages with the specified topic
		mqttClient.subscribe(topicCommand);
		Serial.print("MQTT subscribed to ");
		Serial.println(topicCommand);
	} else {
		Serial.print("Error connection to MQTT using:");
		Serial.println();
	}
	return mqttClient.connected();
}

// The loop function is called in an endless loop
void loop() {
	// Feed the watchdog - must be called within 8 seconds
	wdt_reset();

	if (!connectMqttServer()) {
   	    Serial.println("MQTT not connected to server");
		return;
	}
	if (millis() >= 86400000) { // reset every 24 hours (1 Day)
		Serial.println("Resetting Arduino - 24h uptime reached");
		mqttClient.publish(topicReset, "true");
		mqttClient.publish(topicStatus, "offline");
		mqttClient.disconnect();
		delay(100);
		hardwareReset(); // proper hardware reset via WDT
	}

	// Publish status at most every 5 seconds (not every loop iteration)
	if (millis() - lastStatusPublish >= 5000) {
		lastStatusPublish = millis();
		char uptimeBuf[12];
		ltoa(millis(), uptimeBuf, 10);
		mqttClient.publish(topicUptime, uptimeBuf);
		mqttClient.publish(topicReset, "false");
		mqttClient.publish(topicStatus, "online");
	}

	if (PowerSerial::swu.getCount() >= 0){
    	PowerSerial::swu.parseMe();
	}
    if (PowerSerial::solar.getCount() >= 0){
    	PowerSerial::solar.parseMe();
	}

	if (PowerSerial::swu.getCount() < 0) {
		PowerSerial::swu.transmitDataToMqtt(mqttHandler);
	} else {
		Serial.println("SWU: Powerserial has no result .... waiting: ");
	}
	if (PowerSerial::solar.getCount() < 0) {
		PowerSerial::solar.transmitDataToMqtt(mqttHandler);
	} else {
		Serial.println("Solar: Powerserial has no result .... waiting: ");
	}

}
