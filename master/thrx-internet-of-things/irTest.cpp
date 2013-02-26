// Do not remove the include below
#include "irTest.h"

byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
//byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                       //Gateway ip
byte server[] = { 85, 92, 86, 84 };                     //Server ip
char serverName[] = "emoncms.org";

char input[]     =  "/input/post?node=3";

char inputJson[] =	"&json=";
char apikey[]    =  "&apikey=1e5510f5fd5f6b50ad7c1e733b240481";

//http://emoncms.org/input/post?node=2&json={test_bezug_zaehlerstand:0}&apikey=1e5510f5fd5f6b50ad7c1e733b240481


#define port 80
char str[250];

unsigned long tmillis, lmillis, lastupdate;

void setup() {
	PowerSerial::setup();
}

// The loop function is called in an endless loop
void loop() {
	PowerSerial::parse();
	if (PowerSerial::count < 0){
		String jsonResult = PowerSerial::jsonResult;
		if ((millis() - lastupdate) > 30000)  //Every 30 seconds
				{
			lastupdate = millis();

			lmillis = tmillis;  //timing to determine amount of time since last call
			tmillis = millis();

	//		sendEthernet();
		}
	}


}

