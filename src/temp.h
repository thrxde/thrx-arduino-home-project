#pragma once

#include "Arduino.h"
// add your includes for the project temper_test here
#include <OneWire.h>
#include <DallasTemperature.h>
#include "mqtthandler.h"
#include "thing.h"

// end of add your includes here

// 1 -- getTemperatureSensors -----------------------------
//-- getDeviceCount: 10
//  2822C99D400F5 -- 24.81
//  2852749D400CE -- 25.37
//  28CAAA9D40022 -- 24.75
//  289AB9D400F2 -- 25.62
//  285ECC9C4004D -- 25.19
//  28418E9D400D7 -- 25.37
//  2845F9D400D -- 25.50
//  283B7A9D40039 -- 25.56
//  2817D29C40070 -- 25.19
//  2857E99D4007E -- 24.87

class Temp {

    void begin();
    void serialPrintHexAndTemp(byte addr[8]);

public:
    static Temp temperature;
    static void setup();
    void loop();
    void printAllData();
    void transmitTempDataToMqtt(Thing *things, MqttHandler mqttHandler);
};
