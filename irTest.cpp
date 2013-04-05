// Do not remove the include below
#include "irTest.h"
#include <SPI.h>
#include <Ethernet.h>
//#include <Dns.h>

byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                //Gateway ip
//byte server[] = { 213,138,101,177 };                 //Server ip

//char apiurl[] = "http://emoncms.org/api/post.json?apikey=YOURAPIKEY&json=";
//char timeurl[] = "http://emoncms.org/time/local.json?apikey=YOURAPIKEY";
// For posting to emoncms server with host name, (DNS lookup) comment out if using static IP address below
// emoncms.org is the public emoncms server. Emoncms can also be downloaded and run on any server.
char serverName[] = "emoncms.org";

char input[] = "/input/post?node=1";

char inputJson[] = "&json=";
char apikey[] = "&apikey=YOURAPIKEY";

int wasConnected;

//http://emoncms.org/input/post?node=2&json={test_bezug_zaehlerstand:0}&apikey=YOURAPIKEY

EthernetClient client;

#define port 80

unsigned long tmillis, lmillis, lastupdate;

void setup() {
	tmillis = 0;
	lmillis = 0;
	lastupdate = 0;

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println("openenergymonitor.org thrx Project");
	delay(1000);
//	if (Ethernet.begin(mac) == 0) {
//		delay(1000);
//		Serial.println("Failed to configure Ethernet using DHCP");
//    	Ethernet.begin(mac, ip); //configure manually
//	}
   	Ethernet.begin(mac, ip); //configure manually


	delay(1000);
	Serial.print("Local IP address: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}
	Serial.println();
	Serial.println(Ethernet.dnsServerIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println();


	PowerSerial::setup();
}

// The loop function is called in an endless loop
void loop() {



	// if there's incoming data from the net connection.
	// send it out the serial port. This is for debugging
	// purposes only:
	if (client.available()) {
		char c = client.read();
		Serial.print(c);
	}

	// if there's no net connection, but there was one last time
	// through the loop, then stop the client:
	if (!client.connected() && wasConnected) {
		Serial.println();
		Serial.println("disconnecting.");
		client.stop();
		wasConnected = false;
	}

	if (PowerSerial::solar.count < 0) {
		int waitTime = millis() - lastupdate;
		if (waitTime > 30000) {
			String jsonResult = PowerSerial::solar.jsonResult;
			Serial.println("transmit Every 30 seconds");
			lastupdate = millis();

			lmillis = tmillis; //timing to determine amount of time since last call
			tmillis = millis();

			delay(1000);
			if (!client.connected()) {
				Serial.println(jsonResult);
				// if there's a successful connection:
				Serial.println("!client.connected() --> 1");

//				DNSClient dns;
//				IPAddress remote_addr;
//				dns.begin(Ethernet.dnsServerIP());
//				int ret = dns.getHostByName(serverName, remote_addr);
//				  if (ret == 1) {
//				    Serial.println(serverName);
//				    Serial.println(remote_addr);
//				  } else {
//					    Serial.println("Fail !!!");
//				  }


				if (client.connect(serverName, 80)) {
					delay(1000);
					Serial.println("!client.connect() --> 2");
					wasConnected = true;
					Serial.println("connecting...");
					// send the HTTP PUT request:
					String str = "GET ";
						str.concat(input);
    					str.concat(apikey);
						str.concat(inputJson);
						str.concat(jsonResult);
						str.concat(" HTTP/1.0");

					client.println(str);
					client.println("Host: emoncms.org");
					client.println("User-Agent: arduino-ethernet");
					client.println("Connection: close");
					client.println();
					client.flush();
					Serial.println(str);
					Serial.println("Respons:");
					Serial.println(client.readString());
					PowerSerial::solar.count = 0;
				} else {
					delay(1000);
					// if you couldn't make a connection:
					Serial.println("connection failed");
					Serial.println();
					Serial.println("disconnecting.");
					client.stop();
				}

			}
		} else {
//			Serial.print("Still waiting: ");
//			Serial.println(waitTime);
		}
	} else {
		Serial.println("Powerserial has no result .... waiting: ");
	}

	if (PowerSerial::solar.count >= 0){
		PowerSerial::parse();
	}

}

