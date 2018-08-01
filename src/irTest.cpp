// Do not remove the include below
#include "irTest.h"
//#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
//#include <Dns.h>

byte mac[] = { 0x54, 0x52, 0x58, 0x10, 0x00, 0x18 }; //Ethernet shield mac address
byte ip[] = { 192, 168, 1, 8 };                     //Ethernet shield ip address
byte gateway[] = { 192, 168, 1, 1 };                //Gateway ip
byte mqttServer[] = { 192,168,1,3 };                 //Openhab / Mosquitto  ip
char mqttUser[] = "openhabian";
char mqttPass[] = "mqtt4openhab";
char mqttClientName[] = "easymeterArduinoClient";

//char apiurl[] = "http://emoncms.org/api/post.json?apikey=YOURAPIKEY&json=";
//char timeurl[] = "http://emoncms.org/time/local.json?apikey=YOURAPIKEY";
// For posting to emoncms server with host name, (DNS lookup) comment out if using static IP address below
// emoncms.org is the public emoncms server. Emoncms can also be downloaded and run on any server.
//char serverName[] = "emoncms.org";

char input[] = "/input/post?node=1";

char inputJson[] = "&json=";
char apikey[] = "&apikey=1e5510f5fd5f6b50ad7c1e733b240481";

int wasConnected;

//http://emoncms.org/input/post?node=2&json={test_bezug_zaehlerstand:0}&apikey=1e5510f5fd5f6b50ad7c1e733b240481

EthernetClient ethClient;
PubSubClient mqttClient(mqttServer, 1883, callback, ethClient);


#define port 80

unsigned long tmillis, lmillis, lastupdate;

void callback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
	// Zähler
	int i = 0;
	// Hilfsvariablen für die Convertierung der Nachricht in ein String
	char message_buff[100];
	
	Serial.println("Message arrived: topic: " + String(topic));
	Serial.println("Length: " + String(length,DEC));
	
	// Kopieren der Nachricht und erstellen eines Bytes mit abschließender \0
	for(i=0; i<length; i++) {
	message_buff[i] = payload[i];
	}
	message_buff[i] = '\0';
	
	// Konvertierung der nachricht in ein String
	String msgString = String(message_buff);
	Serial.println("Payload: " + msgString);
}

void setup() {
	tmillis = 0;
	lmillis = 0;
	lastupdate = 0;

	delay(1000);
	Serial.begin(9600);
	delay(1000);
	Serial.println("home thrx project - mqtt");
	Serial.println("Version 1.0.0");
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

  	connectMqttServer();


	PowerSerial::setup();
}

void connectMqttServer() {
   // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this,
    // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
    // PubSubClient.h
 	// Aufbau der Verbindung mit MQTT falls diese nicht offen ist.
	if (!mqttClient.connected()) {
		if (mqttClient.connect(mqttClientName, mqttUser, mqttPass)) {
			mqttClient.publish("/openHAB/connect",mqttClientName);
        	// Abonieren von Nachrichten mit dem angegebenen Topic
			mqttClient.subscribe("/openHAB/broadcast");
		}
	}
}

// The loop function is called in an endless loop
void loop() {

	connectMqttServer();

	// if there's incoming data from the net connection.
	// send it out the serial port. This is for debugging
	// purposes only:
	//if (ethClient.available()) {
	//	char c = ethClient.read();
	//	Serial.print(c);
	//}

	// if there's no net connection, but there was one last time
	// through the loop, then stop the ethClient:
	//if (!ethClient.connected() && wasConnected) {
	//	Serial.println();
	//	Serial.println("disconnecting.");
	//	ethClient.stop();
	//	wasConnected = false;
	//}

	if (PowerSerial::solar.count < 0) {
		int waitTime = millis() - lastupdate;
		if (waitTime > 15000) {
			String jsonResult = PowerSerial::solar.jsonResult;
			Serial.println("transmit Every 15 seconds");
			lastupdate = millis();

			lmillis = tmillis; //timing to determine amount of time since last call
			tmillis = millis();

			delay(1000);
			if (!ethClient.connected()) {
				Serial.println(jsonResult);
				// if there's a successful connection:
				Serial.println("!ethClient.connected() --> 1");

//				DNSClient dns;
//				IPAddress remote_addr;
//				dns.begin(Ethernet.dnsServerIP());
//				int ret = dns.getHostByName(serverName, remote_addr);
//				  if (ret == 1) {
//				    Serial.println(serverName);
//				    Serial.println(remote_addr);
//					    Serial.println("Fail !!!");
//				  }


				if (ethClient.connect(serverName, 80)) {
					delay(1000);
					Serial.println("!ethClient.connect() --> 2");
					wasConnected = true;
					Serial.println("connecting...");
					// send the HTTP PUT request:
					String str = "GET ";
						str.concat(input);
    					str.concat(apikey);
						str.concat(inputJson);
						str.concat(jsonResult);
						str.concat(" HTTP/1.0");

					ethClient.println(str);
					ethClient.println("Host: emoncms.org");
					ethClient.println("User-Agent: arduino-ethernet");
					ethClient.println("Connection: close");
					ethClient.println();
					ethClient.flush();
					Serial.println(str);
					Serial.println("Respons:");
					Serial.println(ethClient.readString());
					PowerSerial::solar.count = 0;
				} else {
					delay(1000);
					// if you couldn't make a connection:
					Serial.println("connection failed");
					Serial.println();
					Serial.println("disconnecting.");
					ethClient.stop();
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
		PowerSerial::solar.parse();
	}

//				  } else {
}

