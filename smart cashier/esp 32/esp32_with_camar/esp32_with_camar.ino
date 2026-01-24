#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Fateh";
const char* password = "700900600";

WebServer server(80);

const int RELAY_PIN = 12;         // غيّرها حسب توصيلك


void handleNotify() {
  // if (!server.hasArg("p")&&!server.hasArg("s")) {
  //   server.send(400, "text/plain", "Missing p");
  //   return;
  // }

  String product = server.arg("p");
  product.trim();

  Serial.println("Received: " + product);
  server.send(200, "text/plain", "OK " + product);

  String product1 = server.arg("s");
  product.trim();

  Serial.println("Received: " + product1);



  //relayOn();
  digitalWrite(RELAY_PIN, HIGH);
  delay(3000);
  //relayOff();
  digitalWrite(RELAY_PIN,LOW );

  server.send(200, "text/plain", "OK " + product);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  // relayOff();
  digitalWrite(RELAY_PIN,LOW );

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/notify", handleNotify); // مثال: /notify?p=KINZA
  server.begin();
}

void loop() {
  server.handleClient();
}