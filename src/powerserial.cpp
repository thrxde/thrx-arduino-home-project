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

bool PowerSerial::startsWith(const char *str, const char *prefix) {
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

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
	var_bezug[0] = '\0';
	var_liefer[0] = '\0';
	var_momentan_L1[0] = '\0';
	var_momentan_L2[0] = '\0';
	var_momentan_L3[0] = '\0';
	var_momentan_L1_3[0] = '\0';
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

	// Fixed-size telegram buffer on STACK — zero heap allocation
	char telegram[TELEGRAM_BUF_SIZE];
	int telegramIdx = 0;
	int append = 0;
	int tryToRead = 1;
	unsigned long parseStart = millis();
	bool parseSuccess = false;

	while (tryToRead > 0) {
		wdt_reset();

		if ((millis() - parseStart) > PARSE_TIMEOUT_MS) {
			Serial.print(name);
			Serial.println(F(": TIMEOUT"));
			return;  // member vars unchanged
		}

		if (serial->available()) {
			char c = serial->read();
			if (c > 0) {
				if (append == 1) {
					if (telegramIdx < TELEGRAM_BUF_SIZE - 1) {
						telegram[telegramIdx++] = c;
					}
					if (c == '!') {  // end of telegram
						telegram[telegramIdx] = '\0';
						append = 0;
						tryToRead = 0;
						parseSuccess = true;
					}
				} else if (c == '/') {  // start of telegram
					telegramIdx = 0;
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
		Serial.println(F(": parse incomplete"));
		return;  // member vars unchanged
	}

	Serial.print(name);
	Serial.print(F(": OK "));
	Serial.print(telegramIdx);
	Serial.println(F("B"));

	if (debugMode) {
		Serial.println(telegram);
	}

	// Parse into LOCAL fixed-size buffers — no heap allocation
	char new_bezug[VALUE_BUF_SIZE] = "";
	char new_liefer[VALUE_BUF_SIZE] = "";
	char new_L1[POWER_BUF_SIZE] = "";
	char new_L2[POWER_BUF_SIZE] = "";
	char new_L3[POWER_BUF_SIZE] = "";
	char new_L1_3[POWER_BUF_SIZE] = "";
	int new_count = 0;

	// Process telegram line by line
	char *lineStart = telegram;
	for (int i = 0; i <= telegramIdx; i++) {
		if (telegram[i] == '\n' || telegram[i] == '\0' || i == telegramIdx) {
			int lineLen = &telegram[i] - lineStart;
			// Trim \r\n from end
			while (lineLen > 0 && (lineStart[lineLen - 1] == '\r' || lineStart[lineLen - 1] == '\n')) {
				lineLen--;
			}
			if (lineLen > 0) {
				processLine(lineStart, lineLen, new_bezug, new_liefer,
					new_L1, new_L2, new_L3, new_L1_3, new_count);
			}
			lineStart = &telegram[i + 1];
		}
	}

	// ATOMIC promotion — only after full successful parse
	strncpy(var_bezug, new_bezug, VALUE_BUF_SIZE - 1);
	var_bezug[VALUE_BUF_SIZE - 1] = '\0';
	strncpy(var_liefer, new_liefer, VALUE_BUF_SIZE - 1);
	var_liefer[VALUE_BUF_SIZE - 1] = '\0';
	strncpy(var_momentan_L1, new_L1, POWER_BUF_SIZE - 1);
	var_momentan_L1[POWER_BUF_SIZE - 1] = '\0';
	strncpy(var_momentan_L2, new_L2, POWER_BUF_SIZE - 1);
	var_momentan_L2[POWER_BUF_SIZE - 1] = '\0';
	strncpy(var_momentan_L3, new_L3, POWER_BUF_SIZE - 1);
	var_momentan_L3[POWER_BUF_SIZE - 1] = '\0';
	strncpy(var_momentan_L1_3, new_L1_3, POWER_BUF_SIZE - 1);
	var_momentan_L1_3[POWER_BUF_SIZE - 1] = '\0';
	count = -1;
}

void PowerSerial::processLine(const char *line, int len,
	char *new_bezug, char *new_liefer,
	char *new_L1, char *new_L2, char *new_L3, char *new_L1_3,
	int &new_count) {

	// Find '(' to split key and value
	int parenPos = -1;
	for (int i = 0; i < len; i++) {
		if (line[i] == '(') { parenPos = i; break; }
	}
	if (parenPos <= 0) return;  // no key(value) structure

	// Extract key (everything before '(')
	char key[32];
	int keyLen = (parenPos < 31) ? parenPos : 31;
	strncpy(key, line, keyLen);
	key[keyLen] = '\0';

	// Extract value (between '(' and '*' or ')')
	int valueStart = parenPos + 1;
	int valueEnd = -1;

	// Find '*' after '(' (unit separator, e.g., "00040461.79*kWh")
	for (int i = valueStart; i < len; i++) {
		if (line[i] == '*') { valueEnd = i; break; }
	}
	// If no '*', find last ')'
	if (valueEnd < 0) {
		for (int i = len - 1; i >= valueStart; i--) {
			if (line[i] == ')') { valueEnd = i; break; }
		}
	}
	if (valueEnd < 0) return;  // malformed line

	int valueLen = valueEnd - valueStart;
	char value[VALUE_BUF_SIZE];
	if (valueLen >= VALUE_BUF_SIZE) valueLen = VALUE_BUF_SIZE - 1;
	strncpy(value, line + valueStart, valueLen);
	value[valueLen] = '\0';

	if (debugMode) {
		Serial.print(key);
		Serial.print(F(" "));
		Serial.println(value);
	}

	// Match OBIS key to field — using strncmp via startsWith()
	if (startsWith(key, PATTERN_BEZUG_KEY)) {
		strncpy(new_bezug, value, VALUE_BUF_SIZE - 1);
		new_bezug[VALUE_BUF_SIZE - 1] = '\0';
	} else if (startsWith(key, PATTERN_LIEFER_KEY)) {
		strncpy(new_liefer, value, VALUE_BUF_SIZE - 1);
		new_liefer[VALUE_BUF_SIZE - 1] = '\0';
	} else if (startsWith(key, PATTERN_MOMENTAN_255_L1) || startsWith(key, PATTERN_MOMENTAN_0_L1)) {
		strncpy(new_L1, value, POWER_BUF_SIZE - 1);
		new_L1[POWER_BUF_SIZE - 1] = '\0';
	} else if (startsWith(key, PATTERN_MOMENTAN_255_L2) || startsWith(key, PATTERN_MOMENTAN_0_L2)) {
		strncpy(new_L2, value, POWER_BUF_SIZE - 1);
		new_L2[POWER_BUF_SIZE - 1] = '\0';
	} else if (startsWith(key, PATTERN_MOMENTAN_255_L3) || startsWith(key, PATTERN_MOMENTAN_0_L3)) {
		strncpy(new_L3, value, POWER_BUF_SIZE - 1);
		new_L3[POWER_BUF_SIZE - 1] = '\0';
	} else if (startsWith(key, PATTERN_MOMENTAN_255_L1_3) || startsWith(key, PATTERN_MOMENTAN_0_L1_3)) {
		strncpy(new_L1_3, value, POWER_BUF_SIZE - 1);
		new_L1_3[POWER_BUF_SIZE - 1] = '\0';
	}
	new_count++;
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

	if (strlen(var_bezug) == 16) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_BEZUG_KEY);
		mqttHandler.publish(topicBuf, var_bezug);
	} else if (debugMode) {
		Serial.print(F("var_bezug invalid len="));
		Serial.println(strlen(var_bezug));
	}

	if (strlen(var_liefer) == 16) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_LIEFER_KEY);
		mqttHandler.publish(topicBuf, var_liefer);
	} else if (debugMode) {
		Serial.print(F("var_liefer invalid len="));
		Serial.println(strlen(var_liefer));
	}

	if (validatePowerValue(var_momentan_L1)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L1);
		mqttHandler.publish(topicBuf, var_momentan_L1);
	} else if (debugMode) {
		Serial.print(F("var_L1 invalid len="));
		Serial.println(strlen(var_momentan_L1));
	}

	if (validatePowerValue(var_momentan_L2)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L2);
		mqttHandler.publish(topicBuf, var_momentan_L2);
	} else if (debugMode) {
		Serial.print(F("var_L2 invalid len="));
		Serial.println(strlen(var_momentan_L2));
	}

	if (validatePowerValue(var_momentan_L3)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L3);
		mqttHandler.publish(topicBuf, var_momentan_L3);
	} else if (debugMode) {
		Serial.print(F("var_L3 invalid len="));
		Serial.println(strlen(var_momentan_L3));
	}

	if (validatePowerValue(var_momentan_L1_3)) {
		snprintf(topicBuf, sizeof(topicBuf), "%s/%s", mqttPrefix, EXTERN_MOMENTAN_L1_3);
		mqttHandler.publish(topicBuf, var_momentan_L1_3);
	} else if (debugMode) {
		Serial.print(F("var_L1_3 invalid len="));
		Serial.println(strlen(var_momentan_L1_3));
	}

	if (debugMode) {
		Serial.print(name);
		Serial.println(F(": transmit done"));
	}
	count = 0;
}

int PowerSerial::validatePowerValue(const char *value) {
	int len = strlen(value);
	if (len == 9) {
		return true;
	} else if (len == 10 && value[0] == '-') {
		return true;
	}
	return false;
}

int PowerSerial::getCount() {
	return count;
}
