#include "mqtthandler.h"

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
