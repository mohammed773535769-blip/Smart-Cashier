#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_CASHIER";
const char* password = "12345678";
char[7];// يوجد سبع رسائل سوف استقبلها 1 other  2 sanaa 3 kinze 4 sun tap 5 haniaih 6 marrey 7 non 
char[0]="other";
char[1]="sanaa";
int sanaa_weight=500
int cuttent_weight=0
WebServer server(80);

void handlePost() {
  if (!server.hasArg("p")) {
    server.send(400, "text/plain", "Missing product");
    return;
  }

  String product = server.arg("p");
  product.trim();

  Serial.println("Product received: " + product);
  if(product==sanaa)
  // هنا سوف اطرح دالة تشيك الوزن 
  if(sanaa_weight==cuttent_weight)
  server.send(200, "text/plain", "weight is corretc");
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP()); // غالباً 192.168.4.1

  server.on("/post", handlePost);
  server.begin();
}

void loop() {
  server.handleClient();
}
