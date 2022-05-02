/**
 *  _____           _    _____            _             _ 
 * |  __ \         | |  / ____|          | |           | |
 * | |__) |__  _ __| |_| |     ___  _ __ | |_ _ __ ___ | |
 * |  ___/ _ \| '__| __| |    / _ \| '_ \| __| '__/ _ \| |
 * | |  | (_) | |  | |_| |___| (_) | | | | |_| | | (_) | |
 * |_|   \___/|_|   \__|\_____\___/|_| |_|\__|_|  \___/|_|                                                     
 * 
 * Webinterface made for the ESP32 Development board to test circuits.
 * 
 * Author: Lucas Birkert aka. KekOnTheWorld
 */


// #### CONFIGURATION ####

// Port for the http server (80 is the default for the http protocol).
#define PORT_HTTP 80

// NOTICE: DNS only works with the ACCESS POINT

// Port for the dns server (53 is the default for the dns protocol).
#define PORT_DNS 53
// Domain which can be used to access the webinterface.
#define DOMAIN "ct.rl"


/**
 * ACCESS POINT
 * 
 * You can access the webinterface simply by connecting to the access point via WLAN.
 */
 
// Comment out this line to disable the access point.
#define AP_ENABLE
// The SSID of the AP (name of the network).
#define AP_SSID "PortControl"
// The password of the AP.
#define AP_PASSWORD "123456789"
// The ip of the AP sepperated by commas.
#define AP_IP 8,8,4,4


/**
 * WLAN Connection
 * 
 * You can let the ESP32 connect to a WLAN and access the webinterface using its local ip.
 */
 
// Uncomment this line to enable the WLAN connection.
// #define WLAN_ENABLE
// The SSID of the WLAN (name of the network).
#define WLAN_SSID "FritzBox! xyz"
// The password of the WLAN.
#define WLAN_PASSWORD "myVeryCoolPassword"


/**
 * Serial
 */
#define SERIAL_ENABLE

#define SERIAL_BAUD 115200



// #### CODE ####


// Detect if WLAN Connection and ACCESS POINT are both enabled.
#ifdef WLAN_ENABLE
  #ifdef AP_ENABLE
    #error WLAN Connection and ACCESS POINT may not be enabled at the same time.
  #endif
#endif


#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <uri/UriRegex.h>

// The files containing the HTML, CSS and JS are stored in header files. 'pages.h' includes all of them
#include "web/pages.h"



#ifdef SERIAL_ENABLE
  #define SERIAL_PRINTLN(x) Serial.println(x)
  #define SERIAL_PRINT(x) Serial.print(x)
  #define SERIAL_BEGIN(x) Serial.begin(x)
#else
  #define SERIAL_PRINTLN(x)
  #define SERIAL_PRINT(x)
  #define SERIAL_BEGIN(x)
#endif

#define STR_(X) #X
#define STR(X) STR_(X)



WebServer server(PORT_HTTP);

#ifdef AP_ENABLE
  IPAddress apIP(AP_IP);
  DNSServer dnsServer;
#endif


void setup_routes() {
  server.on("/", []() {
    server.send(200, "text/html", page_index);
  });

  server.on(UriRegex("^\\/aread\\/([0-9]+)$"), []() {
    int pin = server.pathArg(0).toInt();
    int value = analogRead(pin);
    server.send(200, "text/plain", String(value));
  });

  server.on(UriRegex("^\\/dread\\/([0-9]+)$"), []() {
    int pin = server.pathArg(0).toInt();
    boolean value = digitalRead(pin);
    server.send(200, "text/plain", value?"1":"0");
  });

  server.on(UriRegex("^\\/input\\/([0-9]+)$"), []() {
    int pin = server.pathArg(0).toInt();
    pinMode(pin, INPUT);
    server.send(200, "text/plain", "OK");
  });

  server.on(UriRegex("^\\/output\\/([0-9]+)$"), []() {
    int pin = server.pathArg(0).toInt();
    pinMode(pin, OUTPUT);
    server.send(200, "text/plain", "OK");
  });

  server.on(UriRegex("^\\/dwrite\\/([0-9]+)/(0|1)$"), []() {
    int pin = server.pathArg(0).toInt();
    boolean value = server.pathArg(1).toInt() == 1;
    digitalWrite(pin, value);
    server.send(200, "text/plain", "OK");
  });

  server.on(UriRegex("^\\/awrite\\/([0-9]+)/(0|1)$"), []() {
    int pin = server.pathArg(0).toInt();
    int value = server.pathArg(1).toInt();
    analogWrite(pin, value);
    server.send(200, "text/plain", "OK");
  });
}


void setup() {
  SERIAL_BEGIN(SERIAL_BAUD);
  

  #ifdef AP_ENABLE
    SERIAL_PRINTLN("Starting AP...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.softAPsetHostname(DOMAIN);

    delay(100);

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    SERIAL_PRINT("AP started on ");
    SERIAL_PRINTLN(apIP);

    SERIAL_PRINTLN("Starting DNS...");

    dnsServer.start(PORT_DNS, DOMAIN, apIP);

    SERIAL_PRINTLN("DNS started on port " STR(PORT_DNS) "!");
  #endif

  #ifdef WLAN_ENABLE
    SERIAL_PRINTLN("Connecting to WiFi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);

    while ( WiFi.status() != WL_CONNECTED ) {
      delay(500);
      SERIAL_PRINT(".");
    }

    SERIAL_PRINTLN("WiFi Connected!");
    SERIAL_PRINT("IP Address: ");
    SERIAL_PRINTLN(WiFi.localIP());
  #endif

  SERIAL_PRINTLN("Starting HTTP server...");

  setup_routes();

  server.enableCORS();
  server.begin();

  SERIAL_PRINTLN("HTTP server started on port " STR(PORT_HTTP) "!");
}


void loop() {
  #ifdef AP_ENABLE
    dnsServer.processNextRequest();
  #endif
  
  server.handleClient();
}
