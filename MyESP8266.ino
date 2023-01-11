#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266FtpServer.h>
#include <ArduinoJson.h>

#define led 2

const char *ssid = "AnWiESP";

ESP8266WebServer HTTP(80);
FtpServer ftpSrv;

void setup(void){
  pinMode(led, OUTPUT);
  Serial.begin(9600);

  WiFi.softAP(ssid);

  SPIFFS.begin();
  HTTP.begin();
  ftpSrv.begin("AnWiESP", "AnWiESPPass");

  Serial.print("\nMy IP to connect via Web-Browser or FTP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("\n");

  HTTP.on("/led_switch", [](){
    HTTP.send(200, "text/plain", led_switch());
  });

  HTTP.on("/get_json", [](){
    HTTP.send(200, "application/json", getJson()); //application/json getJson()
  });

  HTTP.onNotFound([](){
    if(!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "Not Found");
  });
}
 
void loop(void){
  HTTP.handleClient();
  ftpSrv.handleFTP();
}

String led_switch() {
  byte state;
  if(digitalRead(led)) state = 0;
  else state = 1;
  digitalWrite(led, state);
  return String(state);
}

String getJson() {
  StaticJsonDocument<256> jsonDoc;

  JsonObject obj1 = jsonDoc.createNestedObject();
  obj1["info"] = "Датчик температуры в зале";
  obj1["value"] = 22;
  obj1["time"] = "15:15";

  JsonObject obj2 = jsonDoc.createNestedObject();
  obj2["info"] = "Датчик влажности в зале";
  obj2["value"] = 63;
  obj2["time"] = "15:15";

  JsonObject obj3 = jsonDoc.createNestedObject();
  obj3["info"] = "Датчик движения в зале";
  obj3["value"] = true;
  obj3["time"] = "15:15";

  char buffer[256];
  serializeJson(jsonDoc, buffer);
  return buffer;
}

bool handleFileRead(String path) {
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if(SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

