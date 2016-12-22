/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
#include "FS.h"

char mode = 0;
const char *ssid = "URAQI LT";
const char *password = "12345678";

String payload = ":D";

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1


ESP8266WebServer server ( 80 );

void handleSave(){
  String qssid = server.arg("ssid");
  String qpassword = server.arg("password");
  Serial.println(qssid);
  Serial.println(qpassword);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["ssid"] = qssid;
  json["password"] = qpassword;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  json.printTo(configFile);
    
  String response = "<html>\
    <head>\
      <title>ESP8266 Demo</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
      Configuration saved ! :D\
    </body>\
  </html>";

    server.send ( 200, "text/html", response );

}

void handleConfigure() {
    
  String response = "<html>\
    <head>\
      <title>ESP8266 Demo</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
      <form method='post' action='./save'>\
        <input type='text' placeholder='ssid' name='ssid'>\
        <input type='password' placeholder='password' name='password'>\
        <input type='submit' value='GUARDAR'>\
      </form>\
    </body>\
  </html>";

    server.send ( 200, "text/html", response );

}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void handleView() {
    
  String response = "<html>\
    <head>\
      <title>ESP8266 Demo</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>";
    response.concat(payload);
    response.concat("</body></html>");
    server.send ( 200, "text/html", response );

}

void setup ( void ) {

  pinMode(D0, OUTPUT);
  pinMode(D1, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("");
  delay(1000);
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
  }else{
    size_t size = configFile.size();
    if (size > 1024) {
      Serial.println("Config file size is too large");
    }else{
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
    
      // We don't use String here because ArduinoJson library requires the input
      // buffer to be mutable. If you don't use ArduinoJson, you may as well
      // use configFile.readString instead.
      configFile.readBytes(buf.get(), size);
    
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
    
      if (!json.success()) {
        Serial.println("Failed to parse config file");
      }else{
    
        ssid = json["ssid"];
        password = json["password"];
      
        Serial.print("SSID: ");
        Serial.print(ssid);
        Serial.print("PASSWORD: ");
        Serial.print(password);
        mode = 1;
      }
      
    }
  }


  
  
  
  if (mode == 0){
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
  
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleConfigure);
    server.on("/save", handleSave);
  }else{
    WiFi.begin (ssid, password);
    int retry = 0;
    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }
  
    Serial.println ( "" );
    Serial.print ( "Connected to " );
    Serial.println ( ssid );
    Serial.print ( "IP address: " );
    Serial.println ( WiFi.localIP() );
  
    if ( MDNS.begin ( "esp8266" ) ) {
      Serial.println ( "MDNS responder started" );
    }
  
    server.on ( "/", handleView );
  }

  server.begin();
  Serial.println ( "HTTP server started" );
  server.onNotFound ( handleNotFound );

}

void loop () {
	server.handleClient();
  if(digitalRead(D1) == 0){
      File configFile = SPIFFS.open("/config.json", "r");
      if (!configFile) {
        Serial.println("Nothing to delete");
      }else{
        SPIFFS.remove("/config.json");
        Serial.println("File deleted");
      }
  }


  if (mode == 1){
    digitalWrite(D0, HIGH);
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    http.begin("http://200.200.200.200/nodemcu.php"); //HTTP
    
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {



          //
          // Step 1: Reserve memory space
          //
          StaticJsonBuffer<200> jsonBuffer;
          
          //
          // Step 2: Deserialize the JSON string
          //
          JsonObject& root = jsonBuffer.parseObject(http.getString());
          
          if (!root.success())
          {
            Serial.println("parseObject() failed");
            return;
          }
          
            //
            // Step 3: Retrieve the values
            //          
            payload = (const char*)root["time"];
            Serial.println(payload);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    
    digitalWrite(D0, LOW);
    
    delay(10000);        
  }

    
}
