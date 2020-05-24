
#define _FW_VERSION "v1.0.0 (23-05-2020)"

#define _HOSTNAME   "ESP32framework"
#include "ESP32_Framework.h"

#define LED_BUILTIN 2
//    #define LED_ON      HIGH
//    #define LED_OFF     LOW


// WiFi Server object and parameters
WiFiServer server(80);


//=====================================================================
void setup()
{
  Serial.begin(115200);
  while(!Serial) { /* wait a bit */ }

  lastReset     = ((String)esp_reset_reason()).c_str();
  
  DebugTln("\r\n[ESP32_Framework]\r\n");
  DebugTf("Booting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());
  
//================ SPIFFS ===========================================
  if (SPIFFS.begin()) 
  {
    DebugTln(F("SPIFFS Mount succesfull\r"));
    SPIFFSmounted = true;
  } else { 
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
  }

  readSettings(true);

  // attempt to connect to Wifi network:
  DebugTln("Attempting to connect to WiFi network\r");
  int t = 0;
  while ((WiFi.status() != WL_CONNECTED) && (t < 25))
  {
    delay(500);
    Serial.print(".");
    t++;
  }
  Debugln();
  if ( WiFi.status() != WL_CONNECTED) 
  {
    sprintf(cMsg, "Connect to AP '%s' and configure WiFi on  192.168.4.1   ", _HOSTNAME);
    DebugTln(cMsg);
  }
  // Connect to and initialise WiFi network
  digitalWrite(LED_BUILTIN, HIGH);
  startWiFi(_HOSTNAME, 240);  // timeout 4 minuten
  digitalWrite(LED_BUILTIN, LOW);

  startMDNS(settingHostname);

  //NTP syncing
  // Uncomment the line below to see what it does behind the scenes
  setDebug(INFO);  
  waitForSync(); 
  Timezone CET;
  CET.setLocation(F("Europe/Amsterdam"));
  CET.setDefault(); 
  
  Serial.println("UTC time: "+ UTC.dateTime());
  Serial.println("CET time: "+ CET.dateTime());

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", ((String)esp_reset_reason()).c_str());
  DebugTln(cMsg);

  startTelnet();

  Serial.print("\nGebruik 'telnet ");
  Serial.print (WiFi.localIP());
  Serial.println("' voor verdere debugging\r\n");

//================ Start HTTP Server ================================
  setupFSexplorer();
  httpServer.serveStatic("/FSexplorer.png",   SPIFFS, "/FSexplorer.png");
  httpServer.on("/",          sendIndexPage);
  httpServer.on("/index",     sendIndexPage);
  httpServer.on("/index.html",sendIndexPage);
  httpServer.serveStatic("/index.css", SPIFFS, "/index.css");
  httpServer.serveStatic("/index.js",  SPIFFS, "/index.js");
  // all other api calls are catched in FSexplorer onNotFounD!
  httpServer.on("/api", HTTP_GET, processAPI);


  httpServer.begin();
  DebugTln("\nServer started\r");
  
  // Set up first message as the IP address
  sprintf(cMsg, "%03d.%03d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  DebugTf("\nAssigned IP[%s]\r\n", cMsg);
  
} // setup()


//=====================================================================
void loop()
{
  events(); //handle NTP events
  httpServer.handleClient();
  //MDNS.update();

  //--- Eat your hart out!
  
} // loop()
