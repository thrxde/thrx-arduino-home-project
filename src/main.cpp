// Do not remove the include below
#include "main.h"


byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                //Gateway / Router IP
byte mqttServer[] = { 192,168,1,3 };                 //Openhab / Mosquitto  IP
char mqttUser[] = "username";                       // MQTT Username
char mqttPass[] = "password";                       // MQTT Password
char mqttClientName[]  = "Arduino Zaehlerschrank";
char topicConnect[]    = "arduino/1/status";
char topicLastWill[]   = "arduino/1/status";
unsigned long waitTime = 5000; // max mqtt transmit rate 5sec

//int wasConnected;

EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);


//#define port 80

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	// Zähler
	int i = 0;
	// Hilfsvariablen für die Konvertierung der Nachricht in ein String
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

	PowerSerial::setup(waitTime);
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
    //	delay(5);

	connectMqttServer();
	if (PowerSerial::swu.getCount() >= 0){
    	PowerSerial::swu.parseMe();
	}
    if (PowerSerial::solar.getCount() >= 0){
    	PowerSerial::solar.parseMe();
	}

	if (PowerSerial::swu.getCount() < 0) {
		PowerSerial::swu.transmitDataToMqtt(mqttClient);
	} else {
		Serial.println("SWU: Powerserial has no result .... waiting: ");
	}
	if (PowerSerial::solar.getCount() < 0) {
		PowerSerial::solar.transmitDataToMqtt(mqttClient);
	} else {
		Serial.println("Solar: Powerserial has no result .... waiting: ");
	}


}

