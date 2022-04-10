#include "mqtthandler.h"

void MqttHandler::publish(const char* topic, const char* payload){
	Serial.print("publish -- topic: ");
	Serial.print(strlen(topic));
	Serial.print(" -- payload: ");
	Serial.print(  strlen(payload));
	Serial.print(" -- lenghth: ");
	Serial.print( 5 + 2 + strlen(topic) + strlen(payload));
	Serial.print(" -- ");
	Serial.print(topic);
	Serial.print(" -- transmited: ");
	Serial.print(mqttClient.publish(topic,payload));
	delay(10);
	Serial.println(" -- OK");
}
