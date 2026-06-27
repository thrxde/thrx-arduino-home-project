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

	// Pattern strings for OBIS code matching
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

	static const unsigned long PARSE_TIMEOUT_MS = 10000;
	static const int TELEGRAM_BUF_SIZE = 1024;
	static const int VALUE_BUF_SIZE = 17;  // max 16 chars + null
	static const int POWER_BUF_SIZE = 11;  // max 10 chars + null

	// Fixed-size buffers — NO heap allocation
	char var_bezug[VALUE_BUF_SIZE];
	char var_liefer[VALUE_BUF_SIZE];
	char var_momentan_L1[POWER_BUF_SIZE];
	char var_momentan_L2[POWER_BUF_SIZE];
	char var_momentan_L3[POWER_BUF_SIZE];
	char var_momentan_L1_3[POWER_BUF_SIZE];

	unsigned long waitTime;
	unsigned long lastupdate;
	int count;
	bool firstParseDiscarded;  // skip first parse result (always garbled from mid-stream start)

	HardwareSerial *serial;
	const char *mqttPrefix;

	void processLine(const char *line, int len,
		char *new_bezug, char *new_liefer,
		char *new_L1, char *new_L2, char *new_L3, char *new_L1_3,
		int &new_count);
	bool startsWith(const char *str, const char *prefix);
	int validatePowerValue(const char *value);

public:

	static PowerSerial swu, solar;
	const char *name;

	static void setup(unsigned long _waitTime);
	void begin(const char* _name, HardwareSerial& _serial, const char *_mqttPrefix, unsigned long _waitTime);
	void parseMe();
	void transmitDataToMqtt(MqttHandler &mqttHandler);
	int getCount();
};

#endif /* POWERSERIAL_H_ */
