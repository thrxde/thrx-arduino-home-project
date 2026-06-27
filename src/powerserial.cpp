#include "powerserial.h"

// Debug mode flag — defined in main.cpp
extern volatile bool debugMode;

// MQTT topic structure (prefix is "swu" or "solar"):
// <prefix>/zaehler/strom/stand/bezug
// <prefix>/zaehler/strom/stand/lieferung
// <prefix>/zaehler/strom/leistung/phase/1
// <prefix>/zaehler/strom/leistung/phase/2
// <prefix>/zaehler/strom/leistung/phase/3
// <prefix>/zaehler/strom/leistung/phasen

PowerSerial PowerSerial::swu;
PowerSerial PowerSerial::solar;

// Static pattern string definitions (shared across instances)
const char *PowerSerial::PATTERN_BEZUG_KEY = "1-0:1.8.0*255";
const char *PowerSerial::PATTERN_LIEFER_KEY = "1-0:2.8.0*255";
const char *PowerSerial::PATTERN_MOMENTAN_255_L1 = "1-0:21.7.255*255";
const char *PowerSerial::PATTERN_MOMENTAN_255_L2 = "1-0:41.7.255*255";
const char *PowerSerial::PATTERN_MOMENTAN_255_L3 = "1-0:61.7.255*255";
const char *PowerSerial::PATTERN_MOMENTAN_255_L1_3 = "1-0:1.7.255*255";
const char *PowerSerial::PATTERN_MOMENTAN_0_L1 = "1-0:21.7.0*255";
const char *PowerSerial::PATTERN_MOMENTAN_0_L2 = "1-0:41.7.0*255";
const char *PowerSerial::PATTERN_MOMENTAN_0_L3 = "1-0:61.7.0*255";
const char *PowerSerial::PATTERN_MOMENTAN_0_L1_3 = "1-0:1.7.0*255";
const char *PowerSerial::EXTERN_BEZUG_KEY = "zaehler/strom/stand/bezug";
const char *PowerSerial::EXTERN_LIEFER_KEY = "zaehler/strom/stand/lieferung";
const char *PowerSerial::EXTERN_MOMENTAN_L1 = "zaehler/strom/leistung/phase/1";
const char *PowerSerial::EXTERN_MOMENTAN_L2 = "zaehler/strom/leistung/phase/2";
const char *PowerSerial::EXTERN_MOMENTAN_L3 = "zaehler/strom/leistung/phase/3";
const char *PowerSerial::EXTERN_MOMENTAN_L1_3 = "zaehler/strom/leistung/phasen";

void PowerSerial::setup(unsigned long _waitTime) {
	Serial.println(F("PowerSerial::setup()"));
	swu.begin("SWU", Serial2, "swu", _waitTime);
	solar.begin("Solar", Serial3, "solar", _waitTime);
}

void PowerSerial::begin(const char *_name, HardwareSerial &_serial, const char *_mqttPrefix, unsigned long _waitTime) {
	name = _name;
	serial = &_serial;
	serial->begin(9600, SERIAL_7E1);
	count = 0;
	mqttPrefix = _mqttPrefix;
	waitTime = _waitTime;
	lastupdate = 0;
	Serial.print(F("PowerSerial::begin(): "));
	Serial.println(name);
}

void PowerSerial::parseMe() {
	if (count < 0) {
		if (debugMode) {
			Serial.print(name);
			Serial.println(F(": parseMe skipped (count<0)"));
		}
		return;
	}

	if (debugMode) {
		Serial.print(name);
		Serial.print(F(": parseMe count="));
		Serial.println(count);
	}

	// Read telegram into local buffer — member vars UNTOUCHED until success
	String complete = "";
	int append = 0;
	int tryToRead = 1;
	unsigned long parseStart = millis();
	bool parseSuccess = false;

	while (tryToRead > 0) {
		// Feed watchdog during potentially long serial reads
		wdt_reset();

		// Timeout protection: abort if reading takes too long
		if ((millis() - parseStart) > PARSE_TIMEOUT_MS) {
			Serial.print(name);
			Serial.println(F(": TIMEOUT - aborting telegram read"));
			return;  // member vars unchanged — last good values preserved
		}

		if (serial->available()) {
			char c = serial->read();
			if (c > 0) {
				if (append == 1) {
					complete.concat(c);
					if (c == '!') {  // end of telegram
						append = 0;
						tryToRead = 0;
						parseSuccess = true;
					}
				} else if (c == '/') {  // start of telegram
					complete = "";
					append = 1;
					if (debugMode) {
						Serial.print(name);
						Serial.println(F(": start telegram"));
					}
				}
			} else {
				tryToRead++;
				if (tryToRead >= 500) {
					Serial.print(name);
					Serial.println(F(": ERROR no data, retry limit"));
					return;  // member vars unchanged
				}
			}
		} else {
			delay(1);
		}
	}

	if (!parseSuccess) {
		Serial.print(name);
		Serial.println(F(": parse incomplete (no end marker)"));
		return;  // member vars unchanged
	}

	// Summary (always printed — minimal output)
	Serial.print(name);
	Serial.print(F(": OK "));
	Serial.print(complete.length());
	Serial.println(F("B"));

	if (debugMode) {
		Serial.println(complete);
	}

	// Parse into LOCAL temporaries — not into member vars
	String new_bezug = "";
	String new_liefer = "";
	String new_L1 = "";
	String new_L2 = "";
	String new_L3 = "";
	String new_L1_3 = "";
	int new_count = 0;

	int lastNewLinePosition = 0;
	int newLinePosition = 0;
	do {
		lastNewLinePosition = newLinePosition;
		newLinePosition = complete.indexOf('\n', newLinePosition + 1);
		String segment;
		if (newLinePosition != -1) {
			segment = complete.substring(lastNewLinePosition + 1, newLinePosition + 1);
		} else {
			segment = complete.substring(lastNewLinePosition + 1, complete.length());
			newLinePosition = -1;
		}

		// Process line into local vars
		int end = segment.length();
		if (end > 0 && segment.charAt(end - 1) == '\n') end--;
		if (end > 0 && segment.charAt(end - 1) == '\r') end--;
		String trimmed = segment.substring(0, end);

		if (debugMode) Serial.println(trimmed);

		if (trimmed.indexOf('(') > 0) {
			String key = trimmed.substring(0, trimmed.indexOf('('));
			String value = "";
			if (trimmed.indexOf('*', trimmed.indexOf('(')) > 0) {
				value = trimmed.substring(trimmed.indexOf('(') + 1, trimmed.lastIndexOf('*'));
			} else {
				value = trimmed.substring(trimmed.indexOf('(') + 1, trimmed.lastIndexOf(')'));
			}

			if (debugMode) {
				Serial.print(key);
				Serial.print(F(" "));
				Serial.println(value);
			}

			if (key.startsWith(PATTERN_BEZUG_KEY)) {
				new_bezug = value;
			} else if (key.startsWith(PATTERN_LIEFER_KEY)) {
				new_liefer = value;
			} else if (key.startsWith(PATTERN_MOMENTAN_255_L1) || key.startsWith(PATTERN_MOMENTAN_0_L1)) {
				new_L1 = value;
			} else if (key.startsWith(PATTERN_MOMENTAN_255_L2) || key.startsWith(PATTERN_MOMENTAN_0_L2)) {
				new_L2 = value;
			} else if (key.startsWith(PATTERN_MOMENTAN_255_L3) || key.startsWith(PATTERN_MOMENTAN_0_L3)) {
				new_L3 = value;
			} else if (key.startsWith(PATTERN_MOMENTAN_255_L1_3) || key.startsWith(PATTERN_MOMENTAN_0_L1_3)) {
				new_L1_3 = value;
			}
			new_count++;
		}
	} while (newLinePosition >= 0);

	// ATOMIC promotion — only update member vars after full successful parse
	var_bezug = new_bezug;
	var_liefer = new_liefer;
	var_momentan_L1 = new_L1;
	var_momentan_L2 = new_L2;
	var_momentan_L3 = new_L3;
	var_momentan_L1_3 = new_L1_3;
	count = -1;  // mark as ready for transmit
}

void PowerSerial::transmitDataToMqtt(MqttHandler &mqttHandler) {
	unsigned long currentWaitTime = millis() - lastupdate;
	if (currentWaitTime < 5000) {
		return;
	}

	lastupdate = millis();

	if (debugMode) {
		Serial.print(name);
		Serial.println(F(": transmit to MQTT"));
	}

	char topicBuf[48];
	if (var_bezug.length() > 0 && var_bezug.length() == 16) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_BEZUG_KEY);
		mqttHandler.publish(topicBuf, var_bezug.c_str());
	} else if (debugMode) {
		Serial.print(F("var_bezug invalid: "));
		Serial.println(var_bezug.length());
	}
	if (var_liefer.length() > 0 && var_liefer.length() == 16) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_LIEFER_KEY);
		mqttHandler.publish(topicBuf, var_liefer.c_str());
	} else if (debugMode) {
		Serial.print(F("var_liefer invalid: "));
		Serial.println(var_liefer.length());
	}
	if (validateValue(var_momentan_L1)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L1);
		mqttHandler.publish(topicBuf, var_momentan_L1.c_str());
	} else if (debugMode) {
		Serial.print(F("var_L1 invalid: "));
		Serial.println(var_momentan_L1.length());
	}
	if (validateValue(var_momentan_L2)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L2);
		mqttHandler.publish(topicBuf, var_momentan_L2.c_str());
	} else if (debugMode) {
		Serial.print(F("var_L2 invalid: "));
		Serial.println(var_momentan_L2.length());
	}
	if (validateValue(var_momentan_L3)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L3);
		mqttHandler.publish(topicBuf, var_momentan_L3.c_str());
	} else if (debugMode) {
		Serial.print(F("var_L3 invalid: "));
		Serial.println(var_momentan_L3.length());
	}
	if (validateValue(var_momentan_L1_3)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L1_3);
		mqttHandler.publish(topicBuf, var_momentan_L1_3.c_str());
	} else if (debugMode) {
		Serial.print(F("var_L1_3 invalid: "));
		Serial.println(var_momentan_L1_3.length());
	}

	if (debugMode) {
		Serial.print(name);
		Serial.println(F(": transmit done"));
	}
	count = 0;
}

int PowerSerial::validateValue(const String& value) {
	if (value.length() == 9) {
		return true;
	} else if (value.length() == 10 && value.startsWith("-")) {
		return true;
	}
	return false;
}

int PowerSerial::getCount() {
	return count;
}
