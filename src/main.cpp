// Do not remove the include below
#include "main.h"

// Create an array of structures
Thing things[] = { 
	{"2822C99D400F5","temp","/sensor/temp/vorlauf"},
	{"2822C99D400F5","temp","/sensor/temp/ruecklauf"}
	};


char pName[] = "thrx home project - mqtt";
char pVersion[] = "Version 1.0.5";
byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                //Gateway / Router IP
byte mqttServer[] = { 192, 168, 1, 3 };             //Openhab / Mosquitto  IP
char mqttClientName[]  = "arduino_1";
char topicConnect[]    = "arduino/1/status";
char topicLastWill[]   = "arduino/1/status";
char topicCommand[]    = "arduino/1/command";
unsigned long waitTime = 5000; // max mqtt transmit rate 5sec


//int wasConnected;

EthernetClient ethClient;
MqttHandler mqttHandler{ethClient, mqttServer};


void setup() {

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println(pName);
	Serial.println("Version: " + String(pVersion));
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

  	mqttHandler.begin();

	PowerSerial::setup(waitTime);
	Temp::setup();
}

// The loop function is called in an endless loop
void loop() {
    //Serial.println("loop start");
    //	delay(5);

	mqttHandler.connectMqttServer();
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
	Temp::temperature.loop();
	Temp::temperature.transmitTempDataToMqtt(things,mqttHandler);	


}

