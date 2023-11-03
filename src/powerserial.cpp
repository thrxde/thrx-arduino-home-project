#include "powerserial.h"

PowerSerial PowerSerial::swu;
PowerSerial PowerSerial::solar;

// swu/zaehler/strom/stand/bezug
// swu/zaehler/strom/stand/lieferung
// swu/zaehler/strom/leistung/phase/1
// swu/zaehler/strom/leistung/phase/2
// swu/zaehler/strom/leistung/phase/3
// swu/zaehler/strom/leistung/phasen

void PowerSerial::setup(unsigned long _waitTime) {
//	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
	Serial.println("PowerSerial::setup()");
	swu.begin("SWU", Serial2, "swu", _waitTime);
	solar.begin("Solar", Serial3, "solar", _waitTime);
}

void PowerSerial::begin(const char *_name, HardwareSerial &_serial,	const char *_mqttPrefix, unsigned long _waitTime) {
	name = _name;
	serial = &_serial;
	serial->begin(9600, SERIAL_7E1);
	count = 0;
	mqttPrefix=_mqttPrefix;
	waitTime=_waitTime;
	lastupdate = 0;
	Serial.println("PowerSerial::begin():");
}

void PowerSerial::parseMe() {
	if (count < 0){
    	Serial.println();
		Serial.print(name);
		Serial.print(":PowerSerial::parseMe():  Waiting ... count=");
   	    Serial.println(count);
		return;
	} else {
    	Serial.println();
		Serial.print(name); 		
		Serial.print(":PowerSerial::parseMe(): count=");
	    Serial.println(count);
	}

	var_bezug = "";
	var_liefer = "";
	var_momentan_L1 = "";
	var_momentan_L2 = "";
	var_momentan_L3 = "";
	var_momentan_L1_3 = "";

	String complete = "";
	int append = 0;
	int tryToRead = 1;
	while(tryToRead > 0){
		if (serial->available()){
			char c = serial->read();
			if ( c > 0) {
				if (append == 1) {
					//Serial.print(c); 
					complete.concat(c);
					if (c=='!') {	// ende telegramm
						append = 0;
						tryToRead = 0;
						Serial.println();
						Serial.print(name); 		
						Serial.println(":PowerSerial:: end telegram");
					}
				} else 	if (c=='/') { // start telegramm
					Serial.println();
					Serial.print(name); 		
					Serial.println(":PowerSerial:: start telegram");
					complete = "";
					append = 1;
				} else {
					Serial.print("u"); 
				}	

			} else {
				Serial.print("x"); 
				tryToRead++;
				if (tryToRead >= 500){
					Serial.print(name); 		
					Serial.print(":PowerSerial:: ERROR no data to read, retry count: ");
					Serial.println(tryToRead);
					return;
				}
			}
		} else {
			Serial.print("."); 
		}
	}
	Serial.println();
	Serial.print(name);
	Serial.print(":PowerSerial GO ...");
	Serial.println();
	Serial.println(complete);
	Serial.println();

	Serial.print(name); 		
	Serial.println(":PowerSerial:: processLine:");
	Serial.println();
	int lastNewLinePosition = 0;
	int newLinePosition = 0;
	do {
		lastNewLinePosition = newLinePosition;
		newLinePosition = complete.indexOf('\n',newLinePosition+1);
		if (newLinePosition != -1) {
			processLine(complete.substring(lastNewLinePosition+1, newLinePosition+1));
		} else { // here after the last comma is found
			processLine(complete.substring(lastNewLinePosition+1, complete.length()));
			newLinePosition = -1;
		}
	} while (newLinePosition >= 0);
 
}

void PowerSerial::processLine(String line) {
	Serial.print(line);
	if (line.endsWith("\n")){
		line = line.substring(0,line.indexOf('\n'));
	}
	if (line.endsWith("\r")){
		line = line.substring(0,line.indexOf('\r'));
	}
	if (line.indexOf('/') >= 0){
		Serial.println("/ found -> start it");
		count = 0;
	} else if (line.indexOf('!') >= 0){
		Serial.println("! found -> set count to -1");
		count = -1;
	} else if (line.indexOf('(') > 0){
		String key = line.substring(0, line.indexOf('('));
 	    Serial.print(key);
    	Serial.print(" ");
		String value = "";
		if (line.indexOf('*',line.indexOf('(')) > 0){
			value = line.substring(line.indexOf('(')+1, line.lastIndexOf('*'));
		} else {
			value = line.substring(line.indexOf('(')+1, line.lastIndexOf(')'));
		}    
	    Serial.println(value);
		if (key.startsWith(PATTERN_BEZUG_KEY)){
			var_bezug = value; 
		} else if (key.startsWith(PATTERN_LIEFER_KEY)){
			var_liefer = value; 
		} else if (key.startsWith(PATTERN_MOMENTAN_255_L1) || key.startsWith(PATTERN_MOMENTAN_0_L1)){
			var_momentan_L1 = value;
		} else if (key.startsWith(PATTERN_MOMENTAN_255_L2) || key.startsWith(PATTERN_MOMENTAN_0_L2)){
			var_momentan_L2=value;
		} else if (key.startsWith(PATTERN_MOMENTAN_255_L3) || key.startsWith(PATTERN_MOMENTAN_0_L3)){
			var_momentan_L3=value;
		} else if (key.startsWith(PATTERN_MOMENTAN_255_L1_3) || key.startsWith(PATTERN_MOMENTAN_0_L1_3)){
			var_momentan_L1_3=value;
		} else if (key.startsWith("1-0:0.0.0*255")){
//			Serial.println("3 NOT MAPPED:  "+line);
		} else {
//			Serial.println("2 NOT MAPPED:  "+line);
		}
		count++;
	} else {
		Serial.println("1 UNKNOWN Line:  "+line);
	}

}

void PowerSerial::transmitDataToMqtt(MqttHandler mqttHandler) {
	int currentWaitTime = millis() - lastupdate;
	if (currentWaitTime < 5000) {
		//Serial.print(".");
	} else {
    	Serial.println();
    	Serial.print(name);
		Serial.println(":PowerSerial::transmit Every 5 seconds (start)");
		lastupdate = millis();

		Serial.print(name);
		Serial.println(":PowerSerial::Publish to MQTT");
		if (var_bezug.length() > 0  && var_bezug.length() == 16)  { 
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_BEZUG_KEY).c_str(),
				var_bezug.c_str()
			);
		} else {
			Serial.print("var_bezug -- ");
			Serial.print(var_bezug);
			Serial.print(" -- ");
			Serial.println(var_bezug.length());
		}
		if (var_liefer.length() > 0 && var_liefer.length() == 16) {
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_LIEFER_KEY).c_str(),
				var_liefer.c_str()
			);
		} else {
			Serial.print("var_liefer -- ");
			Serial.print(var_liefer);
			Serial.print(" -- ");
			Serial.println(var_liefer.length());
		}
		if (validateValue(var_momentan_L1))  {
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_MOMENTAN_L1).c_str(),
				var_momentan_L1.c_str()
			);
		} else {
			Serial.print("var_momentan_L1 -- ");
			Serial.print(var_momentan_L1);
			Serial.print(" -- ");
			Serial.println(var_momentan_L1.length());
		}
		if (validateValue(var_momentan_L2)) {
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_MOMENTAN_L2).c_str(),
				var_momentan_L2.c_str()
			);
		} else {
			Serial.print("var_momentan_L2 -- ");
			Serial.print(var_momentan_L2);
			Serial.print(" -- ");
			Serial.println(var_momentan_L2.length());
		}
		if (validateValue(var_momentan_L3)) {
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_MOMENTAN_L3).c_str(),
				var_momentan_L3.c_str()
			);
		} else {
			Serial.print("var_momentan_L3 -- ");
			Serial.print(var_momentan_L3);
			Serial.print(" -- ");
			Serial.println(var_momentan_L3.length());
		}
		if (validateValue(var_momentan_L1_3)) {
			mqttHandler.publish(
				((String)mqttPrefix + "/" + (String)EXTERN_MOMENTAN_L1_3).c_str(),
				var_momentan_L1_3.c_str()
			);
		} else {
			Serial.print("var_momentan_L1_3 -- ");
			Serial.print(var_momentan_L1_3);
			Serial.print(" -- ");
			Serial.println(var_momentan_L1_3.length());
		}
    	Serial.print(name);
		Serial.println(":PowerSerial::transmit Every 5 seconds (end)");
		count = 0;
	}

}

int PowerSerial::validateValue(String value) {
	if (value.length() > 0) {
		if (value.length() == 9) {
			return true;
		} else if (value.length() == 10 && value.startsWith("-")) {
			return true;
		} else {
			return false;
		}
		return true;
	} else {
		return false;
	}
}

int PowerSerial::getCount(){
	return count;
}


