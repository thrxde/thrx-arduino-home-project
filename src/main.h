// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef main_H_
#define main_H_
#include "Arduino.h"
//add your includes for the project "main" here
#include "config.h"
#include "powerserial.h"
#include <Ethernet.h>

//end of add your includes here

//add your function definitions for the project "main" here

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
void callback(char* topic, byte* payload, unsigned int length);
bool connectMqttServer();
bool reconnect();
#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* main_H_ */
