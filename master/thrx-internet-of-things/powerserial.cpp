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
}

///ESY5Q3DA1024 V3.03
//
//1-0:0.0.0*255(112940679)
//1-0:1.8.0*255(00001013.0368091*kWh)
//1-0:2.8.0*255(00000376.0128508*kWh)
//1-0:21.7.255*255(000046.04*W)
//1-0:41.7.255*255(000122.30*W)
//1-0:61.7.255*255(000079.03*W)
//1-0:1.7.255*255(000247.37*W)
//1-0:96.5.5*255(82)
//0-0:96.1.255*255(1ESY1233002534)

void PowerSerial::parseMe() {
	if (count < 0){
		Serial.println("Waiting ...");
		return;
	}

	String line = serial->readStringUntil('\n');
	if (line.endsWith("\r")){
		line = line.substring(0,line.indexOf('\r'));
	}
	if (line.indexOf('/') >= 0){
		jsonResult = "";
		count = 0;
	} else if (line.indexOf('!') >= 0){
		jsonResult +="}";
		Serial.println("jsonResult: "+jsonResult);
		count = -1;
	} else if (line.indexOf('(') > 0){
		String key = line.substring(0, line.indexOf('('));
		String value = "";
			if (line.indexOf('*',line.indexOf('(')) > 0){
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf('*'));
			} else {
				value = line.substring(line.indexOf('(')+1, line.lastIndexOf(')'));
			}
		if (key.startsWith("1-0:0.0.0*255")){
			Serial.print("Ignoring: ["+key+"]");
			Serial.println("Value: ["+value+"]");
		} else if (key.startsWith(PATTERN_BEZUG_KEY)){
			concatJSON(EXTERN_BEZUG_KEY,value);
		} else if (key.startsWith(PATTERN_LIEFER_KEY)){
			concatJSON(EXTERN_LIEFER_KEY,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1)){
			concatJSON(EXTERN_MOMENTAN_L1,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L2)){
			concatJSON(EXTERN_MOMENTAN_L2,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L3)){
			concatJSON(EXTERN_MOMENTAN_L3,value);
		} else if (key.startsWith(PATTERN_MOMENTAN_L1_3)){
			concatJSON(EXTERN_MOMENTAN_L1_3,value);
		} else {
			Serial.print("NOT FOUND Key: ["+key+"]");
			Serial.println("Value: ["+value+"]");
			Serial.println("Line ->: ["+line+"]");
		}
	} else {
		Serial.println("-->");
		Serial.println("   Line:    "+line);
		Serial.println("<--");
	}

}

String PowerSerial::getJsonResult(){

}

void PowerSerial::concatJSON(String jsonKey, String jsonValue){
	Serial.print("##   MATCH: "+jsonKey );
	Serial.println(":"+jsonValue );

	if (count++ > 0){
		jsonResult.concat(",");
	} else {
		jsonResult.concat("{");
	}
	jsonResult.concat(jsonKey);
	jsonResult.concat(":");
	jsonResult.concat(jsonValue);

}

void interrupt0() {
//  PowerSerial::power.light.interrupt();
}

void interrupt1() {
//  PowerSerial::solar.light.interrupt();
}

void PowerSerial::setup() {
	Serial.begin(9600);

	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}

	Serial.println("Goodnight moon 1!");

//  power.begin("Power",Serial1,4500,UCSR1C,UCSZ10,UPM11);
//	solar.begin("Solar", Serial2, 4500, UCSR2C, UCSZ20, UPM21);
	solar.begin("Solar", Serial2, 4500);
//  attachInterrupt(0,interrupt0,RISING);
//  attachInterrupt(1,interrupt1,RISING);
}

void PowerSerial::parse() {
//  power.parseMe();
	solar.parseMe();
}
