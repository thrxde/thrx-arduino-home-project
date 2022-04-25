#pragma once

#include "HardwareSerial.h"
#include <Ethernet.h>
#include <PubSubClient.h>

class MqttHandler {

EthernetClient ethClient;
PubSubClient mqttClient;
IPAddress mqttServer;

public:

	MqttHandler(EthernetClient _ethClient, IPAddress _mqttServer) {
		ethClient = _ethClient;
		mqttServer = _mqttServer;
	}

    void begin();
	void connectMqttServer();
	void publish(const char* topic, const char* payload);
	static void callback(char* topic, byte* payload, unsigned int length);


};
