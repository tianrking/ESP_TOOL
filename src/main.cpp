#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
String ssidList = "";
bool isConnected = false;
unsigned long apDisableTime = 0;

// 扫描周边WiFi并返回结果
String scanNetworks() {
  int n = WiFi.scanNetworks();
  if (n == 0) {
    return "No networks found";
  } else {
    String result = "";
    for (int i = 0; i < n; ++i) {
      result += String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")\n";
    }
    return result;
  }
}

// 处理WiFi连接请求
void handleRoot() {
  ssidList = scanNetworks();
  String html = "<html><body>";
  html += "<h1>Available Networks</h1>";
  html += "<form action='/connect' method='POST'>";
  html += "<select name='ssid'>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
  }
  html += "</select>";
  html += "<input type='password' name='password' placeholder='Password'>";
  html += "<input type='submit' value='Connect'>";
  html += "</form>";
  html += "<pre>" + ssidList + "</pre>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// 处理WiFi连接
void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  Serial.print("Received SSID: ");
  Serial.println(ssid);
  Serial.print("Received Password: ");
  Serial.println(password);
  if (ssid.length() > 0 && password.length() > 0) {
    server.send(200, "text/html", "<html><body>Connecting to " + ssid + "</body></html>");
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(10000);  // 延迟以等待连接
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected! IP address: ");
      Serial.println(WiFi.localIP());
      server.send(200, "text/html", "<html><body>Connected! IP address: " + WiFi.localIP().toString() + "</body></html>");
      isConnected = true;
      apDisableTime = millis();  // 记录连接成功的时间
    } else {
      server.send(200, "text/html", "<html><body>Failed to connect.</body></html>");
    }
  } else {
    server.send(200, "text/html", "<html><body>SSID or password missing.</body></html>");
  }
}

// 打印系统资源信息
void printSystemInfo() {
  Serial.print("Flash Chip Size: ");
  Serial.print(ESP.getFlashChipSize() / 1024);
  Serial.println(" KB");

  Serial.print("Free Sketch Space: ");
  Serial.print(ESP.getFreeSketchSpace() / 1024);
  Serial.println(" KB");

  Serial.print("Sketch Size: ");
  Serial.print(ESP.getSketchSize() / 1024);
  Serial.println(" KB");

  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("bot_AP");
  IPAddress AP_IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(AP_IP);

  server.on("/", handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);

  server.begin();
  Serial.println("HTTP server started");

  // 打印系统资源信息
  printSystemInfo();
}

void loop() {
  server.handleClient();
  if (isConnected && millis() - apDisableTime > 60000) { // 连接后1分钟后关闭AP
    WiFi.softAPdisconnect(true);
    Serial.println("AP mode disabled to save resources.");
    isConnected = false;
  }
}

// Flash Chip Size 是总的Flash存储器容量。
// Free Sketch Space 是可以用来存储新的用户程序的剩余空间。
// Sketch Size 是当前用户程序占用的Flash存储空间。
// Free Heap 是当前可用的RAM（动态内存）大小。
