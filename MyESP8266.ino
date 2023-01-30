#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266FtpServer.h>
#include <ArduinoJson.h>

#define led 2

const char *ssidSoftAP = "AnWiESP";
//const char *passSoftAP = "AnWiESP";

/*
// Настройки IP адреса
IPAddress local_ipSoftAP(192,168,1,1);
IPAddress gatewaySoftAP(192,168,1,1);
IPAddress subnetSoftAP(255,255,255,0);
*/

const char *ssidSTA = "AnWi";
const char *passSTA = "AnWiPass";

const char *userFtp = "AnWiESP";
const char *passFtp = "AnWiESPPass";

ESP8266WebServer HTTP(80);
FtpServer ftpSrv;

DynamicJsonDocument gDoc(1024);

String getgDoc(String id) {
  DynamicJsonDocument doc(256);

  doc["info"] = gDoc[id]["info"];
  doc["value"] = gDoc[id]["value"];
  doc["time"] = gDoc[id]["time"];

  char buffer[256];
  serializeJson(doc, buffer);
  return buffer;
} // doc[id] = serialized(getgDoc(id));

String date = "15:15";

void setup(void){
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  
  WiFi.begin(ssidSTA, passSTA);
  Serial.print("Connect to " + String(ssidSTA));
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  WiFi.setAutoReconnect(true);
  Serial.print("\nIP (STA) address: ");
  Serial.println(WiFi.localIP());
  
  //WiFi.softAP(ssidSoftAP);
  //WiFi.softAPConfig(local_ipSoftAP, gatewaySoftAP, subnetSoftAP);
  //Serial.print("IP (softAP) address: ");
  //Serial.println(WiFi.softAPIP());

  SPIFFS.begin();
  HTTP.begin();
  ftpSrv.begin(userFtp, passFtp);

  HTTP.on("/led_switch", [](){
    HTTP.send(200, "application/json", led_switch());
  });

  HTTP.on("/getFromId", [](){
    HTTP.send(200, "application/json", getFromId());
  });

  HTTP.on("/getIds", [](){
    HTTP.send(200, "application/json", getIds());
  });

  HTTP.onNotFound([](){
    if(!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "Not Found");
  });

  gDoc["000"]["info"] = "Переключить LED";
  gDoc["000"]["value"] = false;
  gDoc["000"]["time"] = date;

  gDoc["001"]["info"] = "Датчик температуры";
  gDoc["001"]["value"] = 22;
  gDoc["001"]["time"] = date;

  gDoc["002"]["info"] = "Датчик влажности";
  gDoc["002"]["value"] = 63;
  gDoc["002"]["time"] = date;

  gDoc["003"]["info"] = "Датчик освещенности";
  gDoc["003"]["value"] = false;
  gDoc["003"]["time"] = "15:15";

  gDoc["004"]["info"] = "Датчик давления";
  gDoc["004"]["value"] = 680;
  gDoc["004"]["time"] = date;

}
 
void loop(void) {
  HTTP.handleClient();
  ftpSrv.handleFTP();
}

String led_switch() {
  byte state;
  if(digitalRead(led)) state = 0;
  else state = 1;
  digitalWrite(led, state);
  
  DynamicJsonDocument doc(256);
  doc["value"] = bool(state);
  doc["time"] = date;
  
  char buffer[256];
  serializeJson(doc, buffer);
  return buffer;
}

String getIds() {
  DynamicJsonDocument doc(256);

  doc[0] = "000";
  doc[1] = "001";
  doc[2] = "002";
  doc[3] = "003";
  doc[4] = "004";
  
  char buffer[256];
  serializeJson(doc, buffer);
  return buffer;
}

String getFromId() {
  char buffer[1024];
  DynamicJsonDocument doc(512);

  for(int i = 0; i < HTTP.args(); i++) {
    if(HTTP.argName(i) != "id") {
      continue;
    }
    doc[HTTP.arg(i)] = serialized(getgDoc(HTTP.arg(i)));
  }
  serializeJson(doc, buffer);
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

