#include "powerserial.h"

PowerSerial PowerSerial::power;
PowerSerial PowerSerial::solar;

const String PowerSerial::PATTERN_BEZUG_KEY  = "1-0:1.8.0*255"; //Bezugsregister kWh
const String PowerSerial::PATTERN_LIEFER_KEY = "1-0:2.8.0*255"; //Lieferregister kWh
const String PowerSerial::EXTERN_BEZUG_KEY   = "SWU-Zaehlerstand-Bezug";
const String PowerSerial::EXTERN_LIEFER_KEY  = "SWU-Zaehlerstand-Liefer";

const String PowerSerial::PATTERN_MOMENTAN_L1    = "1-0:21.7.255*255"; //Momentanleistung-L1 W
const String PowerSerial::PATTERN_MOMENTAN_L2    = "1-0:41.7.255*255"; //Momentanleistung-L2 W
const String PowerSerial::PATTERN_MOMENTAN_L3    = "1-0:61.7.255*255"; //Momentanleistung-L3 W
const String PowerSerial::PATTERN_MOMENTAN_L1_3  = "1-0:1.7.255*255"; //Momentanleistung- L1 - L3 W

const String PowerSerial::EXTERN_MOMENTAN_L1     = "SWU-Momentan-L1"; //Momentanleistung-L1 W
const String PowerSerial::EXTERN_MOMENTAN_L2     = "SWU-Momentan-L2"; //Momentanleistung-L2 W
const String PowerSerial::EXTERN_MOMENTAN_L3     = "SWU-Momentan-L3"; //Momentanleistung-L3 W
const String PowerSerial::EXTERN_MOMENTAN_L1_3   = "SWU-Momentan-L1-L2-L3"; //Momentanleistung- L1 - L3 W


void PowerSerial::begin(const char *_name, HardwareSerial &_serial,	unsigned long _maxage) {
	name = _name;
	serial = &_serial;
	maxage = _maxage;
	serial->begin(9600, SERIAL_7E1);
	count = 0;
	Serial.println("PowerSerial::begin(): ");
}

/// ESY5Q3DA1024 V3.03
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
		Serial.println("PowerSerial::parseMe():  Waiting ... ");
		return;
	} else {
	   Serial.println("PowerSerial::parseMe():"+count);
	}

	jsonResult = "";
	String complete = "";
	int append = 0;
	int tryToRead = 1;
	while(tryToRead == 1){
		int c = serial->read();
		char c2 = c;
		if ( c > 0) {
//			Serial.print(c2);

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
//	String complete = serial->readStringUntil('/');
//	Serial.println("and GO ...");
//	Serial.println(complete);
	int lastCommaPosition = 0;
	int commaPosition = 0;
	do {
		lastCommaPosition = commaPosition;
		commaPosition = complete.indexOf('\n',commaPosition+1);
		if (commaPosition != -1) {
			processLine(complete.substring(lastCommaPosition+1, commaPosition+1));
//			complete = complete.substring(commaPosition+1, complete.length());
		} else { // here after the last comma is found
			processLine(complete.substring(lastCommaPosition+1, complete.length()));
			commaPosition = -1;
		}
	} while (commaPosition >= 0);

}


void PowerSerial::processLine(String line) {
//	Serial.println(line);
	if (line.endsWith("\n")){
		line = line.substring(0,line.indexOf('\n'));
	}
	if (line.endsWith("\r")){
		line = line.substring(0,line.indexOf('\r'));
	}
	if (line.indexOf('/') >= 0){
		// hier l�uft er nicht rein ...
		jsonResult = "";
		count = 0;
	} else if (line.indexOf('!') >= 0){
		jsonResult +="}";
//		Serial.println("jsonResult: "+jsonResult);
		count = -1;
	} else if (line.indexOf('(') > 0){
		String key = line.substring(0, line.indexOf('('));
		String value = "";
			if (line.indexOf('*',line.indexOf('(')) > 0){
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf('*'));
			} else {
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf(')'));
			}
		if (key.startsWith(PATTERN_BEZUG_KEY)){
			concatJson(EXTERN_BEZUG_KEY,value);
		} else if (key.startsWith(PATTERN_LIEFER_KEY)){
			concatJson(EXTERN_LIEFER_KEY,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1)){
			concatJson(EXTERN_MOMENTAN_L1,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L2)){
			concatJson(EXTERN_MOMENTAN_L2,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L3)){
			concatJson(EXTERN_MOMENTAN_L3,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1_3)){
			concatJson(EXTERN_MOMENTAN_L1_3,value);
		} else if (key.startsWith("1-0:0.0.0*255")){
//			Serial.println("3 NOT MAPPED:  "+line);
		} else {
//			Serial.println("2 NOT MAPPED:  "+line);
		}
	} else {
//		Serial.println("1 NOT MAPPED:  "+line);
	}

}


void PowerSerial::concatJson(String jsonKey, String jsonValue){
//	Serial.print("##   MATCH:    "+jsonKey );
//	Serial.println(":"+jsonValue );
	if (count++ > 0){
		jsonResult.concat(",");
	} else {
		jsonResult.concat("{");
	}
	jsonResult.concat(jsonKey);
	jsonResult.concat(":");
	jsonResult.concat(jsonValue);

}

void PowerSerial::setup() {
//	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
	Serial.println("PowerSerial::setup()");
	solar.begin("Solar", Serial2, 4500);
}

void PowerSerial::parse() {
	solar.parseMe();
}
