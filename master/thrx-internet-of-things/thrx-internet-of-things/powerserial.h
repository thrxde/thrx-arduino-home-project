#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// #include "wattlight.h"

class PowerSerial {

	HardwareSerial *serial;

	static const char *PATTERN;
	const char *name;
	const char *pp;
	unsigned long kwhtemp[2];
	unsigned long wh_phase_temp[3];
	int n;
	unsigned long ms;
	unsigned long maxage;
public:
//  Wattlight light;
	unsigned long kwh[2], count;
	void begin(const char* _name, HardwareSerial& _serial,	unsigned long _maxage);
	void parseMe();
	static PowerSerial power, solar;
	static void setup();
	static void parse();
};

#define FROMNET PowerSerial::power.kwh[0]
#define INTONET PowerSerial::power.kwh[1]
#define SOLAR PowerSerial::solar.kwh[1]
