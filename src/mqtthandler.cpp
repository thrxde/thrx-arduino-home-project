#include "mqtthandler.h"

char mqttClientName[]  = "arduino_1";
char topicConnect[]    = "arduino/1/status";
char topicLastWill[]   = "arduino/1/status";
char topicCommand[]    = "arduino/1/command";


void MqttHandler::begin() {
    PubSubClient mqttClient(mqttServer, 1883, MqttHandler::callback, ethClient);
	connectMqttServer();
}

void MqttHandler::connectMqttServer() {
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
//    	    Serial.print(MQTT_USER);
//    	    Serial.print(MQTT_PASS);
			Serial.println();
		}
	}
	mqttClient.loop();
}


void MqttHandler::publish(const char* topic, const char* payload){
	if (mqttClient.connected()) {
		Serial.print("publish -- topic: ");
		Serial.print(topic);
		Serial.print(" -- payload: ");
		Serial.print(payload);
		Serial.print(" -- lenghth: ");
		Serial.print( 5 + 2 + strlen(topic) + strlen(payload));
		Serial.print(" -- transmited: ");
		Serial.print(mqttClient.publish(topic,payload));
		Serial.println(" -- OK");
	} else {
		Serial.print("publish -- topic: ");
		Serial.print(topic);
		Serial.print(" -- payload: ");
		Serial.print(payload);
		Serial.println(" -- ERROR: MQTT not connected .... ");
	}	
}

void MqttHandler::callback(char* topic, byte* payload, unsigned int length) {
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


