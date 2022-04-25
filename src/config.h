#pragma once

// check (required) parameters passed from the ini
// create your own private_config.ini with the data. See private_config.template.ini

#ifndef ONE_WIRE_BUS
#define ONE_WIRE_BUS 2 /* Default Digitalport Pin 2 */
#endif

//#ifndef SERIAL_BAUD
//#error Need to pass SERIAL_BAUD
//#endif

// MQTT Username
#ifndef MQTT_USER
//#define MQTT_USER "username"
#error Need to pass MQTT_USER
#endif

// MQTT Password
#ifndef MQTT_PASS
//#define MQTT_PASS "password"                        
#error Need to pass MQTT_PASS
#endif

