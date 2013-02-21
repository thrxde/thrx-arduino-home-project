// Do not remove the include below
#include "irTest.h"

//The setup function is called once at startup of the sketch
void setup() {
// Add your initialization code here
//	  S0.begin(12);
	PowerSerial::setup();
}

// The loop function is called in an endless loop
void loop() {
//Add your repeated code here
	PowerSerial::parse();
//	  S0.tick();
}

//void sendEmoncmsData(){
//  char dataBuf[100];
//  char *Dbuf;
//  char *Dbuf2;
//  double realPower;
//  int powerFactor;
//
//  if(emoncms.connected()) // already trying to get data, just leave
//    return;
//  // construct the data buffer so we know how long it is
//  strcpy_P(Dbuf2, PSTR("{RealPower:%d, PowerFactor:0.%d, PowerVoltage:%d.%02d, PowerFrequency:%d.%02d, InsideTemp:%d, OutsideTemp:%d}"));
//	sprintf(dataBuf, Dbuf2,
//    (int)round(realPower),
//    (int)(powerFactor*100),
//    (int)rmsVoltage,
//    (int)(((rmsVoltage+0.005) - (int)rmsVoltage) * 100),
//    (int)(frequency),
//    (int)(((frequency+0.005) - (int)frequency) * 100),
//    (ThermoData[0].currentTemp + ThermoData[1].currentTemp)/2,
//    (int)round(outsideSensor.temp));
//    Serial.println(dataBuf); // take this out when you've got it working
////    return;  // so you can see the buffer before you actually send it
//  strcpy_P(Dbuf,PSTR("emoncms Connecting..."));
//  Serial.print(Dbuf);
//  if(emoncms.connect()){ // set a limit on how long the connect will wait
//    strcpy_P(Dbuf,PSTR("OK..."));
//    Serial.print(Dbuf);
//    tNow = now();
//    strcpy_P(Dbuf,PSTR("GET http://emoncms.org/input/post?apikey=secretnumbers &json="));
//    emoncms.write(Dbuf);
//    emoncms.write(dataBuf);
//    emoncms.write("\n");
//    emoncms.stop();
//  }
//  else {
//    strcpy_P(Dbuf,PSTR("failed..."));
//    Serial.print(Dbuf);
//    emoncms.stop();
//    while(emoncms.status() != 0){
//      delay(5);
//    }
//  }
//}
