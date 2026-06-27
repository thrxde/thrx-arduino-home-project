#ifndef main_H_
#define main_H_

#include "Arduino.h"
#include "config.h"
#include "powerserial.h"
#include <Ethernet.h>
#include <avr/wdt.h>

void loop();
void setup();
void callback(char* topic, byte* payload, unsigned int length);
bool connectMqttServer();
bool reconnect();
void hardwareReset();

#endif /* main_H_ */
