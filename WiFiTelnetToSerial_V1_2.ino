/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <EEPROM.h>

#include <algorithm> // std::min

//#include "index.h" // std::min
#include "index1.h" // std::min
#include "index2.h" // std::min
#include "index3.h" // std::min



#define MAX_STRING_LENGTH   32
#define EEPROMINITPHRASE  "AA55Garzobul" //for myMsg member

typedef struct { 
    char     mySSID[MAX_STRING_LENGTH];
    char     myPW[MAX_STRING_LENGTH];
    int      myPort;
    char     myMsg[MAX_STRING_LENGTH];
} Settings;


#ifndef STASSID
#define STASSID "garzol"
#define STAPSK  "password"
#endif

char statetx = 0x32;

/*
    SWAP_PINS:
   0: use Serial1 for logging (legacy example)
   1: configure Hardware Serial port on RX:GPIO13 TX:GPIO15
      and use SoftwareSerial for logging on
      standard Serial pins RX:GPIO3 and TX:GPIO1
*/

#define SWAP_PINS 1

/*
    SERIAL_LOOPBACK
    0: normal serial operations
    1: RX-TX are internally connected (loopback)
*/

#define SERIAL_LOOPBACK 0

//#define BAUD_SERIAL 115200
#define BAUD_SERIAL 300000
#define BAUD_LOGGER 115200
#define RXBUFFERSIZE 2048

////////////////////////////////////////////////////////////

#if SERIAL_LOOPBACK
#undef BAUD_SERIAL
#define BAUD_SERIAL 3000000
#include <esp8266_peri.h>
#endif

#if SWAP_PINS
#include <SoftwareSerial.h>
SoftwareSerial* logger = nullptr;
#else
#define logger (&Serial1)
#endif

#define STACK_PROTECTOR  512 // bytes

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 4
const char* ssid = STASSID;
const char* password = STAPSK;

const int port = 23;

int olddigitalread = 0;
unsigned long freq = 0L;
int pos  = 0;
unsigned long timerflash = 0L;

WiFiServer server(port);
WiFiClient serverClients[MAX_SRV_CLIENTS];

// Set AP credentials
#define AP_SSID "WIFLIP_"
#define AP_PASS "shootagain"
#define AP_CHANNEL 1
#define AP_HIDDEN true
#define AP_MAX_CON 8

// Set IP addresses (for AP mode)
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer wserver(80);



// This function is called when the sysInfo service was requested.
void handleSysInfo() {
  String result;


  result += "{\n";
  result += "  \"flashSize\": " + String(ESP.getFlashChipSize()) + ",\n";
  result += "  \"freeHeap\": " + String(ESP.getFreeHeap()) + ",\n";
  result += "}";

  wserver.sendHeader("Cache-Control", "no-cache");
  wserver.send(200, "text/javascript; charset=utf-8", result);
}  // handleSysInfo()


Settings settings = {"pipo", "poppies", 23, "Barbibol2 !"};

void setup() {
  String myHostName;

  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, INPUT_PULLUP);

  digitalWrite(5, HIGH);
  digitalWrite(2, HIGH);  //Led blue off

  EEPROM.begin(sizeof(settings));

  unsigned int addr = 0;
  EEPROM.get(addr, settings); //read data from array in ram and cast it into struct called settings

  //EEPROM.put(addr, settings); //write data to array in ram 
  //EEPROM.commit();  //write data from ram to flash memory. Do nothing if there are no changes to EEPROM data in ram

  Serial.begin(BAUD_SERIAL);
  Serial.setRxBufferSize(RXBUFFERSIZE);

  Serial.print(settings.mySSID);
  Serial.print(settings.myPort);
#if SWAP_PINS
  Serial.swap();
  // Hardware serial is now on RX:GPIO13 TX:GPIO15
  // use SoftwareSerial on regular RX(3)/TX(1) for logging
  logger = new SoftwareSerial(3, 1);
  logger->begin(BAUD_LOGGER);
  logger->enableIntTx(false);
  logger->println("\n\nUsing SoftwareSerial for logging");
#else
  logger->begin(BAUD_LOGGER);
  logger->println("\n\nUsing Serial1 for logging");
#endif
  logger->println(ESP.getFullVersion());
  logger->printf("Serial baud: %d (8n1: %d KB/s)\n", BAUD_SERIAL, BAUD_SERIAL * 8 / 10 / 1024);
  logger->printf("Serial receive buffer size: %d bytes\n", RXBUFFERSIZE);

#if SERIAL_LOOPBACK
  USC0(0) |= (1 << UCLBE); // incomplete HardwareSerial API
  logger->println("Serial Internal Loopback enabled");
#endif

    if (sizeof(long) != 4)
    {
      logger->printf("sizeof long is expected to be 4, not %d\n", sizeof(long));
    }

 
  if (!strncmp(settings.myMsg, EEPROMINITPHRASE, 12))
    server = WiFiServer(settings.myPort);
  else {
    strcpy(settings.mySSID, "MySSID");
    strcpy(settings.myPW, "pwd");
    settings.myPort = 23;
  }

  WiFi.mode(WIFI_AP_STA);
  //WiFi.begin(ssid, password);
  settings.mySSID[MAX_STRING_LENGTH-1] = 0;
  settings.myPW[MAX_STRING_LENGTH-1] = 0;
  //added port number


//WiFi.mode(WIFI_STA);
// WiFi.setHostname("WiFlip_iface");
  myHostName  = AP_SSID;
  myHostName += WiFi.macAddress().substring(12,14)+WiFi.macAddress().substring(15);
  //myHostName += ".local";
  WiFi.setHostname(myHostName.c_str());
  WiFi.softAPConfig(local_IP, gateway, subnet);  
  WiFi.softAP(AP_SSID+WiFi.macAddress().substring(12,14)+WiFi.macAddress().substring(15), AP_PASS);



  WiFi.begin(settings.mySSID, settings.myPW);
  //WiFi.begin("garzol", "xxxxxxxx");
  logger->print("\nConnecting to ");
  logger->println(settings.mySSID);
  //logger->println("garzol");

 
  unsigned int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    logger->print('.');
    delay(500);
    if (timeout++ > 20)
    {
      logger->print("connection failed, please check credentials and try again (and keep in range)\nError "); 
      logger->println(WiFi.status());
      //return;
      break;
    }
  }

  logger->println();
  if (WiFi.status() != WL_CONNECTED) {
       logger->print("NOT connected , cause=");
       logger->println(WiFi.status());
     }
      else {
        //logger->print("connected on garzol, address=");
        logger->print("connected on ");
        logger->println(WiFi.SSID());
        logger->print("address=");
        logger->println(WiFi.localIP());
        logger->print("hostname=");
        logger->println(myHostName.c_str());
        logger->print("RSSI=");
        logger->println(WiFi.RSSI());
        logger->print("Port=");
        logger->println(server.port());
      }
  
    // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(myHostName)) {
    logger->println("Error setting up MDNS responder!");
  }
  logger->println("mDNS responder started");

  //start server
  server.begin();
  server.setNoDelay(true);

// Add service to MDNS-SD
  MDNS.addService("wiflip service", "tcp", port);

  logger->print("AP : ");
  logger->println(AP_SSID+WiFi.macAddress().substring(12));

  logger->print("Ready! For WiFlip AP Use web browser on");
  logger->print(" address=");
  logger->println(WiFi.softAPIP());
  //Serial.print(WiFi.localIP());
  //logger->printf(" %d' to connect\n", port);

  // Begin Access Point
  //WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, AP_HIDDEN, AP_MAX_CON);
  // Begin Access Point
   //WiFi.setHostname("barbibol");
  wserver.on("/",handle_OnConnect);
  wserver.on("/$sysinfo", HTTP_GET, handleSysInfo);
  wserver.on("/config_page", HTTP_POST, handleLogin);
  wserver.on("/gif", []() {
      static const uint8_t gif[] PROGMEM = {
        0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
        0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
        0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
      };
      char gif_colored[sizeof(gif)];
      memcpy_P(gif_colored, gif, sizeof(gif));
      // Set the background to a random set of colors
      gif_colored[16] = millis() % 256;
      gif_colored[17] = millis() % 256;
      gif_colored[18] = millis() % 256;
      wserver.send(200, "image/gif", gif_colored, sizeof(gif_colored));
    });
    wserver.begin();


 
}

void loop() {
  
  int curdigital;

  MDNS.update();

  freq++;

  if (freq >= 65535)
  {
    freq = 0L;
  }

  if (WiFi.status() == WL_CONNECTED) 
    {
      timerflash = 16384L;
    }
    else
    {
      timerflash = 128L;
    }

  if (freq < timerflash)
  {
    pos = LOW;
  }  
  else
  {
    pos = HIGH;
  }
  
  //digitalWrite(2, pos);
  curdigital = digitalRead(4);
  //digitalWrite(2, digitalRead(4));
  //digitalWrite(5, digitalRead(4));
  if (curdigital != olddigitalread)
  {
    logger->printf("btn state %d\n", digitalRead(4));

  }

  if (!curdigital)   //button is currently pressed
    digitalWrite(2, LOW);
  else
    digitalWrite(2, pos);


  olddigitalread = curdigital;

  wserver.handleClient();

  //check if there are any new clients
  if (server.hasClient()) {
    //find free/disconnected spot
    int i;
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
      if (!serverClients[i]) { // equivalent to !serverClients[i].connected()
      serverClients[i] = server.available();
        logger->print("New client: index ");
        logger->print(i);
        break;
      }

    //no free/disconnected spot so reject
    if (i == MAX_SRV_CLIENTS) {
      server.available().println("busy");
      // hints: server.available() is a WiFiClient with short-term scope
      // when out of scope, a WiFiClient will
      // - flush() - all data will be sent
      // - stop() - automatically too
      logger->printf("server is busy with %d active connections\n", MAX_SRV_CLIENTS);
    }
  }

  //check TCP clients for data
#if 1
  // Incredibly, this code is faster than the buffered one below - #4620 is needed
  // loopback/3000000baud average 348KB/s
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    while (serverClients[i].available() && Serial.availableForWrite() > 0) {
      // working char by char is not very efficient
      int rb = serverClients[i].read();
      //if (rb != 120) {
        //120 is x this is not an ident request
        Serial.write(rb);
        //logger->printf("wr: %x\n", rb);

      // } else {
      //   if (serverClients[i].availableForWrite() >= 5) {
      //     size_t tcp_sent = serverClients[i].write("IBCDE", 5);
      //     logger->printf("ident request: %x\n", rb);
      //     if (tcp_sent != 5) {
      //         logger->printf("len mismatch: big pb in the ident tcp-write:%zd\n", tcp_sent);
      //       }
      //     }      
      // }
      
    }
#else
  // loopback/3000000baud average: 312KB/s
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    while (serverClients[i].available() && Serial.availableForWrite() > 0) {
      size_t maxToSerial = std::min(serverClients[i].available(), Serial.availableForWrite());
      maxToSerial = std::min(maxToSerial, (size_t)STACK_PROTECTOR);
      uint8_t buf[maxToSerial];
      size_t tcp_got = serverClients[i].read(buf, maxToSerial);
      size_t serial_sent = Serial.write(buf, tcp_got);
      if (serial_sent != maxToSerial) {
        logger->printf("len mismatch: available:%d %zd tcp-read:%zd serial-write:%zd\n", freq, maxToSerial, tcp_got, serial_sent);
      }
    }
#endif

  // determine maximum output size "fair TCP use"
  // client.availableForWrite() returns 0 when !client.connected()
  int maxToTcp = 0;
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    if (serverClients[i]) {
      int afw = serverClients[i].availableForWrite();
      if (afw) {
        if (!maxToTcp) {
          maxToTcp = afw;
        } else {
          maxToTcp = std::min(maxToTcp, afw);
        }
      } else {
        // warn but ignore congested clients
        logger->printf("one client is congested (%d)\n\r", i);
      }
    }

  //check UART for data
  size_t len = std::min(Serial.available(), maxToTcp);
  len = std::min(len, (size_t)STACK_PROTECTOR);
  if (len) {
    uint8_t sbuf[len];
    int serial_got = Serial.readBytes(sbuf, len);
    // push UART data to all connected telnet clients
    for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      // if client.availableForWrite() was 0 (congested)
      // and increased since then,
      // ensure write space is sufficient:
      int afw = serverClients[i].availableForWrite();
      if (afw >= serial_got) {
        size_t tcp_sent = serverClients[i].write(sbuf, serial_got);
        if (tcp_sent != len) {
          // serverClients[i].setNoDelay(true);
          // serverClients[i].flush();
          serverClients[i].setTimeout(1);
          // logger->printf("len mismatch: freq=%d / available:%zd serial-read:%zd tcp-write:%zd afw:%d\n", freq, len, serial_got, tcp_sent, afw);
          // logger->printf("connected? :%d\n", serverClients[i].connected());
        }
      }
    }
  }
  else
  {
    if (freq == 0L)
    {

      //uint8_t is like a byte (8 bits)
      uint8_t sbuf[5];

      //char cbuf[5];
      //cbuf[0] = 'x';
      sbuf[0] = 120; //code of 'x'
      long *pRssi;  //if sizeof long is not 4 we will have a problem

      //cbuf[1] = 'y'; //WiFi.RSSI();
      pRssi = (long *) &(sbuf[1]);
      if (sizeof(long) != 4)
      {
        //logger->printf("sizeof long is expected to be 4, not %d\n", sizeof(long));
        sbuf[1] = 0;
        sbuf[2] = 0;
        sbuf[3] = 0;
        sbuf[4] = 0;
      }
      else
      {
        *pRssi = WiFi.RSSI();
        //*pRssi = -12345;
        //logger->printf("RSSI %d %d %d %d %d\n", *pRssi, cbuf[1], cbuf[2], cbuf[3], cbuf[4]);
      }
      // push UART data to all connected telnet clients
      for (int i = 0; i < MAX_SRV_CLIENTS; i++)
      {
        // if client.availableForWrite() was 0 (congested)
        // and increased since then,
        // ensure write space is sufficient:
        if (serverClients[i].availableForWrite() >= 5) {

            size_t tcp_sent = serverClients[i].write(sbuf, 5);
            if (tcp_sent != 5) {
              logger->printf("len2 mismatch: available:%zd serial-read:%zd tcp-write:%zd\n", 5, 5, tcp_sent);
            }
            else {
              //logger->printf("RSSI sent:%zd\n", WiFi.RSSI());
            }
        }
      }
    }
  }
}





void handle_OnConnect()
{
  String s = MAIN_page2; //Read HTML contents

  char buffer[2048];

  Serial.print(WiFi.localIP());
  sprintf(buffer, s.c_str(), WiFi.macAddress().c_str(),
                             settings.mySSID, 
                             settings.myPort,
                             WiFi.localIP().toString().c_str());   //ssid, ip local
  //Serial.println(buffer);

  wserver.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  wserver.sendHeader("Pragma", "no-cache");
  wserver.sendHeader("Expires", "-1");
  wserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // here begin chunked transfer
  wserver.send(200, "text/html", "");
  wserver.sendContent(MAIN_page1); // here can be for example your first big string
  wserver.sendContent(buffer); // here can be for example your first big string
  wserver.sendContent(MAIN_page3); // here can be for example your first big string
  //wserver.send(200, "text/html", buffer); //Send web page
  wserver.sendContent(F("")); // this tells web client that transfer is done
  wserver.client().stop();
  }


void handleLogin() {                         
  // If a POST request is made to URI /login
  if( ! wserver.hasArg("ssid") || ! wserver.hasArg("pwd") || ! wserver.hasArg("portn") 
      || wserver.arg("ssid") == NULL || wserver.arg("pwd") == NULL || wserver.arg("portn") == NULL) { // If the POST request doesn't have username and password data
    wserver.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }


  unsigned int addr = 0;
  strcpy(settings.mySSID, wserver.arg("ssid").c_str());
  strcpy(settings.myPW, wserver.arg("pwd").c_str());
  settings.myPort = wserver.arg("portn").toInt();
  strcpy(settings.myMsg, EEPROMINITPHRASE);
  EEPROM.put(addr, settings); //write data to array in ram 
  EEPROM.commit();  //write data from ram to flash memory. Do nothing if there are no changes to EEPROM data in ram

  WiFi.begin(settings.mySSID, settings.myPW);

  unsigned int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    logger->print('.');
    delay(500);
    if (timeout++ > 20)
    {
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED) 
  {
     wserver.send(200, "text/html", "<h1>Failed to connect to " + wserver.arg("ssid") + " " + "!</h1><p>Please, check your ssid/password and try again</p>");
 }
  else
  {
     wserver.send(200, "text/html", "<h1>Registered to " + wserver.arg("ssid") + " " + "!</h1><p>you can start wiflip on this network, through: "+WiFi.localIP().toString().c_str()+"</p>");
  }
  //ESP.restart();

}