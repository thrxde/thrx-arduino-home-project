#ifndef POWERSERIAL_H_
#define POWERSERIAL_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "HardwareSerial.h"
#include "mqtthandler.h"
#include <avr/wdt.h>


class PowerSerial {

	// Pattern strings — static (shared across swu/solar instances)
	static const char *PATTERN_BEZUG_KEY;
	static const char *PATTERN_LIEFER_KEY;
	static const char *PATTERN_MOMENTAN_255_L1;
	static const char *PATTERN_MOMENTAN_255_L2;
	static const char *PATTERN_MOMENTAN_255_L3;
	static const char *PATTERN_MOMENTAN_255_L1_3;
	static const char *PATTERN_MOMENTAN_0_L1;
	static const char *PATTERN_MOMENTAN_0_L2;
	static const char *PATTERN_MOMENTAN_0_L3;
	static const char *PATTERN_MOMENTAN_0_L1_3;
	static const char *EXTERN_BEZUG_KEY;
	static const char *EXTERN_LIEFER_KEY;
	static const char *EXTERN_MOMENTAN_L1;
	static const char *EXTERN_MOMENTAN_L2;
	static const char *EXTERN_MOMENTAN_L3;
	static const char *EXTERN_MOMENTAN_L1_3;

	static const unsigned long PARSE_TIMEOUT_MS = 10000; // 10s max for telegram read

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
	void transmitDataToMqtt(MqttHandler &mqttHandler);
	int validateValue(const String& value);
	int getCount();
};

#endif /* POWERSERIAL_H_ */
