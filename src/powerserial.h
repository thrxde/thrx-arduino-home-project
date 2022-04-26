#pragma once

#include "Arduino.h"
#include "HardwareSerial.h"
#include "mqtthandler.h"

class PowerSerial {

	const char *PATTERN_BEZUG_KEY  = "1-0:1.8.0*255"; //Bezugsregister kWh
	const char *PATTERN_LIEFER_KEY = "1-0:2.8.0*255"; //Lieferregister kWh

	const char *PATTERN_MOMENTAN_255_L1    = "1-0:21.7.255*255"; //Momentanleistung-L1 W
	const char *PATTERN_MOMENTAN_255_L2    = "1-0:41.7.255*255"; //Momentanleistung-L2 W
	const char *PATTERN_MOMENTAN_255_L3    = "1-0:61.7.255*255"; //Momentanleistung-L3 W
	const char *PATTERN_MOMENTAN_255_L1_3  = "1-0:1.7.255*255"; //Momentanleistung- L1 - L3 W

	const char *PATTERN_MOMENTAN_0_L1    = "1-0:21.7.0*255"; //Momentanleistung-L1 W
	const char *PATTERN_MOMENTAN_0_L2    = "1-0:41.7.0*255"; //Momentanleistung-L2 W
	const char *PATTERN_MOMENTAN_0_L3    = "1-0:61.7.0*255"; //Momentanleistung-L3 W
	const char *PATTERN_MOMENTAN_0_L1_3  = "1-0:1.7.0*255"; //Momentanleistung- L1 - L3 W


	const char *EXTERN_BEZUG_KEY   = "zaehler/strom/stand/bezug";
	const char *EXTERN_LIEFER_KEY  = "zaehler/strom/stand/lieferung";

	const char *EXTERN_MOMENTAN_L1     = "zaehler/strom/leistung/phase/1"; //Momentanleistung-L1 W
	const char *EXTERN_MOMENTAN_L2     = "zaehler/strom/leistung/phase/2"; //Momentanleistung-L2 W
	const char *EXTERN_MOMENTAN_L3     = "zaehler/strom/leistung/phase/3"; //Momentanleistung-L3 W
	const char *EXTERN_MOMENTAN_L1_3   = "zaehler/strom/leistung/phasen"; //Momentanleistung- L1 - L3 W

	void processLine(String line);

	String var_bezug;
	String var_liefer;
	String var_momentan_L1;
	String var_momentan_L2;
	String var_momentan_L3;
	String var_momentan_L1_3;

	unsigned long waitTime;
	unsigned long lastupdate;
	int count;

	HardwareSerial *serial;
    const char *mqttPrefix;

public:

	static PowerSerial swu, solar;
	const char *name;

	static void setup(unsigned long _waitTime);
	void begin(const char* _name, HardwareSerial& _serial, const char *_mqttPrefix, unsigned long _waitTime);
	void parseMe();
	void transmitDataToMqtt(MqttHandler mqttHandler);
	int getCount();
};

