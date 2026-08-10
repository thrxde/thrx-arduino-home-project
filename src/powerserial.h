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
	static const uint8_t REJECT_RECOVERY_THRESHOLD = 10;  // consecutive rejections before self-heal

	// Fixed-size buffers — NO heap allocation
	char var_bezug[VALUE_BUF_SIZE];
	char var_liefer[VALUE_BUF_SIZE];
	char var_momentan_L1[POWER_BUF_SIZE];
	char var_momentan_L2[POWER_BUF_SIZE];
	char var_momentan_L3[POWER_BUF_SIZE];
	char var_momentan_L1_3[POWER_BUF_SIZE];

	// Last known good values for validation (monotonicity + delta check).
	// After REJECT_RECOVERY_THRESHOLD consecutive rejections, lastGood resets
	// to 0 (self-heal: accept next value as new baseline). This prevents
	// permanent lockout from serial bit-flip contamination.
	float lastGoodBezug;
	float lastGoodLiefer;

	// Consecutive rejection counters — drive self-healing and health reporting
	uint8_t rejectCountBezug;
	uint8_t rejectCountLiefer;

	unsigned long waitTime;
	unsigned long lastupdate;
	int count;
	uint8_t discardCount;  // discard first N telegrams after boot (serial line needs stabilization)

	HardwareSerial *serial;
	const char *mqttPrefix;

	void processLine(const char *line, int len,
		char *new_bezug, char *new_liefer,
		char *new_L1, char *new_L2, char *new_L3, char *new_L1_3,
		int &new_count);
	bool startsWith(const char *str, const char *prefix);
	int validatePowerValue(const char *value);
	bool validateEnergyValue(const char *value, float &lastGood, uint8_t &rejectCount);

public:

	static PowerSerial swu, solar;
	const char *name;

	// Health accessors — used by main.cpp for MQTT health topic
	uint8_t getRejectCountBezug() { return rejectCountBezug; }
	uint8_t getRejectCountLiefer() { return rejectCountLiefer; }

	static void setup(unsigned long _waitTime);
	void begin(const char* _name, HardwareSerial& _serial, const char *_mqttPrefix, unsigned long _waitTime);
	void parseMe();
	void transmitDataToMqtt(MqttHandler &mqttHandler);
	int getCount();
};

#endif /* POWERSERIAL_H_ */
