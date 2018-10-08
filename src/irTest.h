// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef irTest_H_
#define irTest_H_
#include "Arduino.h"
//add your includes for the project irTest here
#include "powerserial.h"

//#include <SPI.h>
//#include <Dns.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//end of add your includes here

//add your function definitions for the project irTest here

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
void callback(char* topic, byte* payload, unsigned int length);
void connectMqttServer();
#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* irTest_H_ */
