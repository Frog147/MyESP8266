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

void setup(void){
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  
  WiFi.begin(ssidSTA, passSTA);  //Connect to the WiFi network
  Serial.print("Waiting to connect...");
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
    delay(500);
    Serial.print("...");
  }
  Serial.print("\nIP (STA) address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  WiFi.softAP(ssidSoftAP);
  //WiFi.softAPConfig(local_ipSoftAP, gatewaySoftAP, subnetSoftAP);

  SPIFFS.begin();
  HTTP.begin();
  ftpSrv.begin(userFtp, passFtp);

  Serial.print("IP (softAP) address: ");
  Serial.println(WiFi.softAPIP());

  HTTP.on("/led_switch", [](){
    HTTP.send(200, "text/plain", led_switch());
  });

  HTTP.on("/get_json", [](){
    HTTP.send(200, "application/json", getJson());
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
  JsonObject root = jsonDoc.to<JsonObject>();

  JsonObject sensorLED = root.createNestedObject("000");
  sensorLED["info"] = "Переключить LED";
  sensorLED["value"] = (bool)digitalRead(led);
  sensorLED["time"] = "15:15";

  JsonObject sensorT = root.createNestedObject("001");
  sensorT["info"] = "Датчик температуры в зале";
  sensorT["value"] = 22;
  sensorT["time"] = "15:15";

  JsonObject sensorH = root.createNestedObject("002");
  sensorH["info"] = "Датчик влажности в зале";
  sensorH["value"] = 63;
  sensorH["time"] = "15:15";

  JsonObject sensorF = root.createNestedObject("003");
  sensorF["info"] = "Датчик движения в зале";
  sensorF["value"] = true;
  sensorF["time"] = "15:15";

  JsonObject sensorD = root.createNestedObject("004");
  sensorD["info"] = "Датчик давления в зале";
  sensorD["value"] = 680;
  sensorD["time"] = "15:15";

  char buffer[1024];
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

