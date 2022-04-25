#pragma once

// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#include "Arduino.h"
//add your includes for the project "main" here
#include <Ethernet.h>

#include "config.h"
#include "powerserial.h"
#include "temp.h"
#include "thing.h"

//end of add your includes here


//add your function definitions for the project "main" here

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif