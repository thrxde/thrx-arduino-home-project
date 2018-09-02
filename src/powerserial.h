#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
// #include "wattlight.h"
#include "HardwareSerial.h"

class PowerSerial {

	HardwareSerial *serial;


	const String PATTERN_BEZUG_KEY  = "1-0:1.8.0*255"; //Bezugsregister kWh
	const String PATTERN_LIEFER_KEY = "1-0:2.8.0*255"; //Lieferregister kWh

	const String PATTERN_MOMENTAN_L1    = "1-0:21.7.255*255"; //Momentanleistung-L1 W
	const String PATTERN_MOMENTAN_L2    = "1-0:41.7.255*255"; //Momentanleistung-L2 W
	const String PATTERN_MOMENTAN_L3    = "1-0:61.7.255*255"; //Momentanleistung-L3 W
	const String PATTERN_MOMENTAN_L1_3  = "1-0:1.7.255*255"; //Momentanleistung- L1 - L3 W

	const String EXTERN_BEZUG_KEY   = "zaehler/strom/stand/bezug";
	const String EXTERN_LIEFER_KEY  = "zaehler/strom/stand/lieferung";

	const String EXTERN_MOMENTAN_L1     = "zaehler/strom/leistung/phase/1"; //Momentanleistung-L1 W
	const String EXTERN_MOMENTAN_L2     = "zaehler/strom/leistung/phase/2"; //Momentanleistung-L2 W
	const String EXTERN_MOMENTAN_L3     = "zaehler/strom/leistung/phase/3"; //Momentanleistung-L3 W
	const String EXTERN_MOMENTAN_L1_3   = "zaehler/strom/leistung/phasen"; //Momentanleistung- L1 - L3 W

 
	const char *name;
	unsigned long ms;
	unsigned long maxage;


public:
	int count;
    const char *mqttPrefix;
	String jsonResult;
	String fieldNames[10];
	String fieldValues[10];
	void begin(const char* _name, HardwareSerial& _serial, unsigned long _maxage, const char *_mqttPrefix);
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
