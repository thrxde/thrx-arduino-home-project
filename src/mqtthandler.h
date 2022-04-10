#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "HardwareSerial.h"
#include <PubSubClient.h>


class MqttHandler {


PubSubClient mqttClient;


public:

	MqttHandler(PubSubClient _mqttClient) {
		mqttClient = _mqttClient;
	}
	void publish(const char* topic, const char* payload);

};

