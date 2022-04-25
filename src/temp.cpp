#include "config.h"
#include "temp.h"

OneWire ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds); /* Dallas Temperature Library fuer Nutzung der oneWire Library vorbereiten */

Temp Temp::temperature;
int loopCnt=0;

// The setup function is called once at startup of the sketch
void Temp::setup(){
	delay(5000);
	temperature.begin();
}

void Temp::begin() {
	delay(5000);
    loopCnt=0;
	sensors.begin(); /* Inizialisieren der Dallas Temperature library */
	delay(1000);
	Serial.print("DeviceCount: ");
	delay(1000);
	Serial.println("-----" + sensors.getDeviceCount());
	delay(1000);
}

// The loop function is called in an endless loop
void Temp::loop() {
	if (++loopCnt >= 30) {
		sensors.begin();
	}
	// reconfigure every 30th loop
	Serial.print(loopCnt);
	Serial.println(" -- getTemperatureSensors -----------------------------");
	delay(1000);
	sensors.requestTemperatures();
}

void transmitTempDataToMqtt(Thing *things, MqttHandler mqttHandler) {
	

}

void Temp::printAllData() {
	int count = sensors.getDeviceCount();
	Serial.print("-- getDeviceCount: ");
	Serial.println(count);
	for (int i = 0; i < count; i++) {
		byte addr[8];
		sensors.getAddress(addr, i);
		serialPrintHexAndTemp(addr);
		sensors.resetAlarmSearch();
	}
}

void serialPrintHexAndTemp(byte addr[8]){
	for (int i = 0; i < 8; i++)	{
		Serial.print(addr[i], HEX);
	}
	Serial.print(" -- ");
	Serial.println(sensors.getTempC(addr));
}