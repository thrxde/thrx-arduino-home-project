// Do not remove the include below
#include "irTest.h"
//#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
//#include <Dns.h>


byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                //Gateway ip
byte mqttServer[] = { 192,168,1,3 };                 //Openhab / Mosquitto  ip
char mqttUser[] = "openhabian";
char mqttPass[] = "mqtt4openhab";
char mqttClientName[]  = "Arduino Zaehlerschrank";
char topicConnect[]    = "arduino/1/status";
char topicLastWill[]   = "arduino/1/status";

//int wasConnected;

EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);


#define port 80

unsigned long lastupdate;

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	// Zähler
	int i = 0;
	// Hilfsvariablen für die Convertierung der Nachricht in ein String
	char message_buff[100];
	
	Serial.println("Message arrived: topic: " + String(topic));
	Serial.println("Length: " + String(length,DEC));
	
	// Kopieren der Nachricht und erstellen eines Bytes mit abschließender \0
	for(unsigned int i = 0; i < length; i++) {
		message_buff[i] = payload[i];
	}
	message_buff[i] = '\0';
	
	// Konvertierung der nachricht in ein String
	String msgString = String(message_buff);
	Serial.println("Payload: " + msgString);
}

void setup() {
	lastupdate = 0;

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println("home thrx project - mqtt");
	Serial.println("Version 1.0.0");
	delay(1000);

   	Ethernet.begin(mac, ip); //configure manually

	delay(1000);
	Serial.print("Local IP address: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}
	Serial.println();
	Serial.println(Ethernet.dnsServerIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println();

  	connectMqttServer();

	PowerSerial::setup();
}

void connectMqttServer() {
   // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this,
    // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
    // PubSubClient.h
 	// Aufbau der Verbindung mit MQTT falls diese nicht offen ist.
	if (!mqttClient.connected()) {
	    Serial.println("MQTT not connected");
		// connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
		if (mqttClient.connect(mqttClientName, mqttUser, mqttPass,topicLastWill,1,false,"offline")) {
			mqttClient.publish(topicConnect ,"online");
        	// Abonieren von Nachrichten mit dem angegebenen Topic
			mqttClient.subscribe("openHAB/broadcast");
    	    Serial.println("MQTT subscribed to openHAB/broadcast");
		}
	}
}

// The loop function is called in an endless loop
void loop() {
    //Serial.println("loop start");
	delay(1000);

	connectMqttServer();

	if (PowerSerial::solar.count < 0) {
		int waitTime = millis() - lastupdate;
		if (waitTime > 15000) {
			Serial.println("transmit Every 15 seconlmillisds");
			lastupdate = millis();
			if (mqttClient.connected()) {
    			Serial.println("Publish to MQTT");
				if (PowerSerial::solar.var_bezug.length() > 0) {
               		Serial.println(mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_BEZUG_KEY).c_str(),
				   		PowerSerial::solar.var_bezug.c_str()
					));
				}
				if (PowerSerial::solar.var_liefer.length() > 0) {
               		mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_LIEFER_KEY).c_str(),
				   		PowerSerial::solar.var_liefer.c_str()
					);
				}
				if (PowerSerial::solar.var_momentan_L1.length() > 0) {
               		mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_MOMENTAN_L1).c_str(),
				   		PowerSerial::solar.var_momentan_L1.c_str()
					);
				}
				if (PowerSerial::solar.var_momentan_L2.length() > 0) {
               		mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_MOMENTAN_L2).c_str(),
				   		PowerSerial::solar.var_momentan_L2.c_str()
					);
				}
				if (PowerSerial::solar.var_momentan_L3.length() > 0) {
               		mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_MOMENTAN_L3).c_str(),
				   		PowerSerial::solar.var_momentan_L3.c_str()
					);
				}
				if (PowerSerial::solar.var_momentan_L1_3.length() > 0) {
               		mqttClient.publish(
				   		((String)PowerSerial::solar.mqttPrefix + (String)PowerSerial::solar.EXTERN_MOMENTAN_L1_3).c_str(),
				   		PowerSerial::solar.var_momentan_L1_3.c_str()
					);
				}
			}	
			PowerSerial::solar.count = 0;
		} else {
			Serial.print(".");
		}
	} else {
		Serial.println("Powerserial has no result .... waiting: ");
	}

	if (PowerSerial::solar.count >= 0){
		PowerSerial::solar.parseMe();
	}
}

