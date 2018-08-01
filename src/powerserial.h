#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
// #include "wattlight.h"
#include "HardwareSerial.h"

class PowerSerial {

	HardwareSerial *serial;

	static const String PATTERN_BEZUG_KEY;
	static const String EXTERN_BEZUG_KEY;
	static const String PATTERN_LIEFER_KEY;
	static const String EXTERN_LIEFER_KEY;

	static const String PATTERN_MOMENTAN_L1; //Momentanleistung-L1 W
	static const String PATTERN_MOMENTAN_L2; //Momentanleistung-L2 W
	static const String PATTERN_MOMENTAN_L3; //Momentanleistung-L3 W
	static const String PATTERN_MOMENTAN_L1_3; //Momentanleistung- L1 - L3 W

	static const String EXTERN_MOMENTAN_L1; //Momentanleistung-L1 W
	static const String EXTERN_MOMENTAN_L2; //Momentanleistung-L2 W
	static const String EXTERN_MOMENTAN_L3; //Momentanleistung-L3 W
	static const String EXTERN_MOMENTAN_L1_3; //Momentanleistung- L1 - L3 W

	String fieldNames[];
	String fieldValues[];

	const char *name;
	unsigned long ms;
	unsigned long maxage;


public:
//  Wattlight light;
	int count;
	String jsonResult;
	void begin(const char* _name, HardwareSerial& _serial,	unsigned long _maxage);
	void parseMe();
	void processLine(String line);
	void concatJson(int index, String key, String value);
	void addToArrays(int index, String key, String value);
	static PowerSerial power, solar;
	static void setup();
	//static void parse();
	int getCount();
};

#define FROMNET PowerSerial::power.kwh[0]
#define INTONET PowerSerial::power.kwh[1]
#define SOLAR PowerSerial::solar.kwh[1]
