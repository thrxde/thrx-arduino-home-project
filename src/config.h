#pragma once

// check (required) parameters passed from the ini
// create your own private_config.ini with the data. See private_config.template.ini
//#ifndef SERIAL_BAUD
//#error Need to pass SERIAL_BAUD
//#endif

// MQTT Username
#ifndef MQTT_USER
#error Need to pass "mqtt_user" in private_config.ini
#endif

// MQTT Password
#ifndef MQTT_PASS
#error Need to pass "mqtt_pass" in private_config.ini
#endif

// Ethernet shield mac address
#ifndef MAC_ADDRESS
#error Need to pass "mac_address" in private_config.ini
#endif

// Ethernet shield ip address
#ifndef IP_ADDRESS
#error Need to pass "ip_address" in private_config.ini
#endif

// Mosquitto  ip (Openhab)
#ifndef MQTT_SERVER_IP
#error Need to pass "mqtt_server_ip" in private_config.ini
#endif
