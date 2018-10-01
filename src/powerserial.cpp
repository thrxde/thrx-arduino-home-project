#include "powerserial.h"


//PowerSerial PowerSerial::power;
PowerSerial PowerSerial::solar;

// swu/zaehler/strom/stand/bezug
// swu/zaehler/strom/stand/lieferung
// swu/zaehler/strom/leistung/phase/1
// swu/zaehler/strom/leistung/phase/2
// swu/zaehler/strom/leistung/phase/3
// swu/zaehler/strom/leistung/phasen

void PowerSerial::setup() {
//	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
	Serial.println("PowerSerial::setup()");
	solar.begin("Solar", Serial2, 4500,"swu/");
}

void PowerSerial::begin(const char *_name, HardwareSerial &_serial,	unsigned long _maxage, const char *_mqttPrefix) {
	name = _name;
	serial = &_serial;
	maxage = _maxage;
	serial->begin(9600, SERIAL_7E1);
	count = 0;
	mqttPrefix=_mqttPrefix;
	Serial.print("PowerSerial::begin()");
}

// /ESY5Q3DA1024 V3.03
//
// 1-0:0.0.0*255(112940679)
// 1-0:1.8.0*255(00001013.0368091*kWh)
// 1-0:2.8.0*255(00000376.0128508*kWh)
// 1-0:21.7.255*255(000046.04*W)
// 1-0:41.7.255*255(000122.30*W)
// 1-0:61.7.255*255(000079.03*W)
// 1-0:1.7.255*255(000247.37*W)
// 1-0:96.5.5*255(82)
// 0-0:96.1.255*255(1ESY1233002534)
// !
//

void PowerSerial::parseMe() {
	if (count < 0){
		Serial.print("PowerSerial::parseMe():  Waiting ... count=");
   	    Serial.println(count);
		return;
	} else {
	   Serial.print("PowerSerial::parseMe(): count=");
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
	while(tryToRead == 1){
		int c = serial->read();
		char c2 = c;
		if ( c > 0) {
			if (c=='/') { // start telegramm
				append = 1;
			}

			if (append == 1) {
				complete.concat(c2);
				if (c=='!') {	// ende telegramm
					append = 0;
					tryToRead = 0;
				}
			}
		}
	}
	Serial.println();
	Serial.println("and GO ...");
	Serial.println(complete);
	int lastCommaPosition = 0;
	int commaPosition = 0;
	do {
		lastCommaPosition = commaPosition;
		commaPosition = complete.indexOf('\n',commaPosition+1);
		if (commaPosition != -1) {
			processLine(complete.substring(lastCommaPosition+1, commaPosition+1));
		} else { // here after the last comma is found
			processLine(complete.substring(lastCommaPosition+1, complete.length()));
			commaPosition = -1;
		}
	} while (commaPosition >= 0);
 
}


void PowerSerial::processLine(String line) {
//	Serial.print("processLine: ");
//	Serial.print(line);
	if (line.endsWith("\n")){
		line = line.substring(0,line.indexOf('\n'));
	}
	if (line.endsWith("\r")){
		line = line.substring(0,line.indexOf('\r'));
	}
	if (line.indexOf('/') >= 0){
//		Serial.println("/ found -> start it");
		count = 0;
	} else if (line.indexOf('!') >= 0){
//		Serial.println("! found -> set count to -1");
		count = -1;
	} else if (line.indexOf('(') > 0){
		String key = line.substring(0, line.indexOf('('));
//    	Serial.print("key: ");
//	    Serial.println(key);
		String value = "";
			if (line.indexOf('*',line.indexOf('(')) > 0){
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf('*'));
			} else {
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf(')'));
			}    
		if (key.startsWith(PATTERN_BEZUG_KEY)){
			var_bezug = value;
		} else if (key.startsWith(PATTERN_LIEFER_KEY)){
			var_liefer = value;
		} else if (key.startsWith(PATTERN_MOMENTAN_L1)){
			var_momentan_L1 = value;
		} else if (key.startsWith(PATTERN_MOMENTAN_L2)){
			var_momentan_L2=value;
		} else if (key.startsWith(PATTERN_MOMENTAN_L3)){
			var_momentan_L3=value;
		} else if (key.startsWith(PATTERN_MOMENTAN_L1_3)){
			var_momentan_L1_3=value;
		} else if (key.startsWith("1-0:0.0.0*255")){
//			Serial.println("3 NOT MAPPED:  "+line);
		} else {
//			Serial.println("2 NOT MAPPED:  "+line);
		}
		count++;
	} else {
		//Serial.println("1 NOT MAPPED:  "+line);
	}

}


