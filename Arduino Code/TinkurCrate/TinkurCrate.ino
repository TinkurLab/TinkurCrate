/*

Published:  2014
Author:     Adam - TINKURLAB
Web:        www.TinkurLab.com

Copyright:  This work is licensed under a Creative Commons Attribution-NonCommercial 4.0 International License. http://creativecommons.org/licenses/by-nc/4.0/

Contributors:
-Xively code based on http://arduino.cc/en/Tutorial/XivelyClient 
-Median library http://playground.arduino.cc/Main/RunningMedian

 */

#include <SPI.h>
#include <Ethernet.h>
#include "RunningMedian.h"

#define APIKEY         "xxxxxxxxxxx" // replace your Xively API Key here
#define FEEDID         xxxxxxxxxxx // replace your Xively Feed ID here
#define USERAGENT      "TinkurCrate" // user agent is the project name

// assign a MAC address for the ethernet controller.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// fill in your address here:
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // replace your Mac Address here

// fill in an available IP address on your network here,
// for manual configuration:
//IPAddress ip(10,0,1,20);
// initialize the library instance:
EthernetClient client;

char server[] = "api.xively.com";   // name address for xively API

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to Xively.com

int modeSwitch = 1;

int incrate = 0;

RunningMedian proximityLast10 = RunningMedian(10);

int timesincrate = 0;

int sensorReading = 0;

int sensorReadingMedian = 0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  delay(2000);

 // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
}

void loop() {
  
  if (modeSwitch == 1)
  {
    // read the analog sensor:
    sensorReading = analogRead(A5); 
   
    proximityLast10.add(sensorReading);
   
    sensorReadingMedian = proximityLast10.getMedian();
    
    Serial.println();
    
    Serial.print("Proximity: ");
    Serial.println(sensorReading); 
    
    Serial.print("Median Proximity: ");
    Serial.print(sensorReadingMedian);
    Serial.print(" w/ ");
    Serial.print(proximityLast10.getCount()); 
    Serial.println(" samples");
    
    
    delay(1000);
  
    if (sensorReadingMedian > 160)
    {
      incrate = 1;
    }
    
    if (sensorReadingMedian < 100)
    {
      incrate = 1;
    }
    
    Serial.print("Is In Crate: ");
    Serial.println(incrate);
    
    Serial.println();
    
  }
  
  // convert the data to a String
  String dataString = "proximity,";
  dataString += String(sensorReadingMedian);

  // you can append multiple readings to this String to 
  // send the xively feed multiple values
  dataString += "\nincrate,";
  dataString += String(incrate);

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    
    resetMode();
    
    client.stop();

  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    
    modeSwitch = 2;
    
    sendData(dataString);
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
  
}

// this method makes a HTTP connection to the server:
void sendData(String thisData) {
 
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.0");
    client.println("Host: api.xively.com");
    client.print("X-ApiKey: ");
    client.println(APIKEY);
    client.print("Content-Length: ");
    client.println(thisData.length());

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.println(thisData);
    Serial.println(thisData);
    
    client.println();
  
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}

void resetMode()
{
 modeSwitch = 1;
 incrate = 0;
}
