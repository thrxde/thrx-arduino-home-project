#include "powerserial.h"

PowerSerial PowerSerial::power;
PowerSerial PowerSerial::solar;

const char *PowerSerial::PATTERN = "1-0:n.8.0*255(";

void PowerSerial::begin(const char *_name, HardwareSerial &_serial,	unsigned long _maxage) {
	name = _name;
	serial = &_serial;
	maxage = _maxage;
	serial->begin(9600, SERIAL_7E1);
	pp = PATTERN;
	count = 0;
	kwh[0] = kwh[1] = 0;
}

//!kwh[0]: kwh[1]:
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
//!kwh[0]: kwh[1]:

void PowerSerial::parseMe() {
	unsigned long newms = millis();
	if (maxage && (kwh[0] || kwh[1]) && newms > ms + maxage) {
		kwh[0] = kwh[1] = 0;
	}
	int c;
	if ((c = serial->read()) > 0) {
		Serial.print("<");
		Serial.print(*pp);
		Serial.print("-->");
		Serial.print("[");
		Serial.print(char(c));

		if (c == '/') {
			//      light.readout();

			kwhtemp[0] = kwhtemp[1] = 0;
		} else if (c == '!') {
			kwh[0] = kwhtemp[0];
			kwh[1] = kwhtemp[1];
			ms = newms;
			count++;

		} else if (*pp == '\0') {

			if (c == ')') {
				pp = PATTERN;
			} else if (c >= '0' && c <= '9') {
				kwhtemp[n] = kwhtemp[n] * 10 + (int(c) - int('0'));
			} else if (c == '.') {
			} else {
				kwhtemp[n] = 0;
				pp = PATTERN;
			}
		} else if (*pp == 'n' && c == '1') {
			n = 0;
			pp++;
		} else if (*pp == 'n' && c == '2') {
			n = 1;
			pp++;
		} else if (c == *pp) {
			pp++;
		} else {
			pp = PATTERN;
		}
	}

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
