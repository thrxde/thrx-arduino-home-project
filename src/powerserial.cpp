#include "powerserial.h"


//PowerSerial PowerSerial::power;
PowerSerial PowerSerial::solar;

const String PowerSerial::PATTERN_BEZUG_KEY  = "1-0:1.8.0*255"; //Bezugsregister kWh
const String PowerSerial::PATTERN_LIEFER_KEY = "1-0:2.8.0*255"; //Lieferregister kWh

const String PowerSerial::PATTERN_MOMENTAN_L1    = "1-0:21.7.255*255"; //Momentanleistung-L1 W
const String PowerSerial::PATTERN_MOMENTAN_L2    = "1-0:41.7.255*255"; //Momentanleistung-L2 W
const String PowerSerial::PATTERN_MOMENTAN_L3    = "1-0:61.7.255*255"; //Momentanleistung-L3 W
const String PowerSerial::PATTERN_MOMENTAN_L1_3  = "1-0:1.7.255*255"; //Momentanleistung- L1 - L3 W

const String PowerSerial::EXTERN_BEZUG_KEY   = "zaehlerstand/bezug";
const String PowerSerial::EXTERN_LIEFER_KEY  = "zaehlerstand/lieferung";

const String PowerSerial::EXTERN_MOMENTAN_L1     = "momentanleistung/phase/1"; //Momentanleistung-L1 W
const String PowerSerial::EXTERN_MOMENTAN_L2     = "momentanleistung/phase/2"; //Momentanleistung-L2 W
const String PowerSerial::EXTERN_MOMENTAN_L3     = "momentanleistung/phase/3"; //Momentanleistung-L3 W
const String PowerSerial::EXTERN_MOMENTAN_L1_3   = "momentanleistung/phasen"; //Momentanleistung- L1 - L3 W

// swu/stromzaehler/zaehlerstand/bezug
// swu/stromzaehler/zaehlerstand/lieferung
// swu/stromzaehler/momentanleistung/phase/1
// swu/stromzaehler/momentanleistung/phase/2
// swu/stromzaehler/momentanleistung/phase/3
// swu/stromzaehler/momentanleistung/phasen

void PowerSerial::setup() {
//	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
	Serial.println("PowerSerial::setup()");
	solar.begin("Solar", Serial2, 4500,"swu/stromzaehler/");
}

void PowerSerial::begin(const char *_name, HardwareSerial &_serial,	unsigned long _maxage, String _mqttPrefix) {
	name = _name;
	serial = &_serial;
	maxage = _maxage;
	serial->begin(9600, SERIAL_7E1);
	count = 0;
	mqttPrefix=_mqttPrefix;
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
		Serial.println("PowerSerial::parseMe():  Waiting ... count="+count);
		return;
	} else {
	   Serial.println("PowerSerial::parseMe(): count="+count);
	}

	jsonResult = "";
	String complete = "";
	int append = 0;
	int tryToRead = 1;
	while(tryToRead == 1){
		int c = serial->read();
		char c2 = c;
		if ( c > 0) {
			Serial.print(c2);
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
	//String complete = serial->readStringUntil('/');
	Serial.println("and GO ...");
	//Serial.println(complete);
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
//	Serial.println(line);
	if (line.endsWith("\n")){
		line = line.substring(0,line.indexOf('\n'));
	}
	if (line.endsWith("\r")){
		line = line.substring(0,line.indexOf('\r'));
	}
	if (line.indexOf('/') >= 0){
		// hier laeuft er nicht rein ...
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
			concatJson(count,EXTERN_BEZUG_KEY,value);
			addToArrays(count,EXTERN_BEZUG_KEY,value);
		} else if (key.startsWith(PATTERN_LIEFER_KEY)){
			concatJson(count,EXTERN_LIEFER_KEY,value);
			addToArrays(count,EXTERN_LIEFER_KEY,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1)){
			concatJson(count,EXTERN_MOMENTAN_L1,value);
			addToArrays(count,EXTERN_MOMENTAN_L1,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L2)){
			concatJson(count,EXTERN_MOMENTAN_L2,value);
			addToArrays(count,EXTERN_MOMENTAN_L2,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L3)){
			concatJson(count,EXTERN_MOMENTAN_L3,value);
			addToArrays(count,EXTERN_MOMENTAN_L3,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1_3)){
			concatJson(count,EXTERN_MOMENTAN_L1_3,value);
			addToArrays(count,EXTERN_MOMENTAN_L1_3,value);
		} else if (key.startsWith("1-0:0.0.0*255")){
			Serial.println("3 NOT MAPPED:  "+line);
		} else {
			Serial.println("2 NOT MAPPED:  "+line);
		}
		count++;
	} else {
		Serial.println("1 NOT MAPPED:  "+line);
	}

}

void PowerSerial::concatJson(int index, String jsonKey, String jsonValue){
//	Serial.print("##   MATCH:    "+jsonKey );
//	Serial.println(":"+jsonValue );
	if (index > 0){
		jsonResult.concat(",");
	} else {
		jsonResult.concat("{");
	}
	jsonResult.concat(jsonKey);
	jsonResult.concat(":");
	jsonResult.concat(jsonValue);
}

void PowerSerial::addToArrays(int index, String key, String value){
	fieldNames[index]=key;
	fieldValues[index]=value;
}
