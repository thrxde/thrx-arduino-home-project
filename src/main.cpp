// Do not remove the include below
#include "main.h"


char pName[] = "thrx home project - mqtt";
char pVersion[] = "Version 1.0.6";
byte mac[] = MAC_ADDRESS;     		//Ethernet shield mac address
const char *ip = IP_ADDRESS;       		//Ethernet shield ip address
const char *mqttServer = MQTT_SERVER_IP; //Openhab / Mosquitto  IP
char mqttClientName[]  = "arduino_1";
char topicConnect[]    = "arduino/1/status";
char topicLastWill[]   = "arduino/1/status";
char topicCommand[]    = "arduino/1/command";
unsigned long waitTime = 5000; // max mqtt transmit rate 5sec
unsigned long resetTime = 0;

void(* resetFunc) (void) = 0; // create a standard reset function

//int wasConnected;
EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);
MqttHandler mqttHandler{mqttClient};


//#define port 80
void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	// electricity meter
	int i = 0;
	// variables for converting the message to a string
	char message_buff[100];
	
	Serial.println("Message arrived: topic: " + String(topic));
	Serial.println("Length: " + String(length,DEC));
	
	// Copy the message and create a byte with a terminating 0
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
	Serial.println(pName);
	Serial.println("Version: " + String(pVersion));
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

}

void connectMqttServer() {
   // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this,
    // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
    // PubSubClient.h
 	// Establishing the connection to MQTT server if it is not open.
	if (!mqttClient.connected()) {
	    Serial.println("MQTT not connected");
		// connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
		if (mqttClient.connect(mqttClientName, MQTT_USER, MQTT_PASS,topicLastWill,1,false,"offline")) {
			mqttClient.publish(topicConnect ,"online");
        	// Subscribe to messages with the specified topic
			mqttClient.subscribe(topicCommand);
    	    Serial.print("MQTT subscribed to ");
    	    Serial.println(topicCommand);
		} else {
    	    Serial.print("Error connection to MQTT using:");
			Serial.println();
		}
	} else {
		mqttClient.loop();
	}
}

// The loop function is called in an endless loop
void loop() {

	if ( millis()  >= 86400000){ //call reset every 24 hours (1 Day).
		Serial.println("Resetting Arduino time is up");
		mqttClient.disconnect();
		resetFunc();
	} 

	connectMqttServer();
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

