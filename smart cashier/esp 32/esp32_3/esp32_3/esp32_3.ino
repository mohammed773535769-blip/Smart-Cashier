//http://192.168.4.1/
// fateh
#include <WiFi.h>
#include <WebServer.h>

#include "HX711.h"

#define HX711_DT  4
#define HX711_SCK 5
#define RELAY_PIN 18

HX711 scale;
int scale_factor=300;

const char* ssid = "ESP32_CASHIER";
const char* password = "12345678";

WebServer server(80);

// ===== إعدادات =====
const int rang = 100;          // سماحية
int time_on_motor = 2000;      // زمن تشغيل الموتور

// ===== الحالة التي ستظهر في الواجهة =====
String last_product = "-";
int expected_weight = 0;
int current_weight  = 0;
String last_result  = "waiting";   // waiting / correct / not_correct / non_or_other / unknown

// ===== قراءة الوزن (مؤقت) =====
void read_weight_current() {
  if (scale.is_ready()) {
    current_weight = scale.get_units(5); // متوسط 5 قراءات
    Serial.print("Weight: ");
    Serial.print(current_weight);
    Serial.println(" g");
  } else {
    Serial.println("HX711 not ready");
  }  
}

void turn_on_motor() {
  digitalWrite(RELAY_PIN, HIGH);
  delay(time_on_motor);
  digitalWrite(RELAY_PIN, LOW);
}

void weight_check_and_update_result() {
  read_weight_current();
  if (abs(current_weight - expected_weight) <= rang) {
    last_result = "correct";
  } else {
    last_result = "not_correct";
  }
}

// ===== صفحة الويب HTML =====
const char MAIN_page[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Smart Cashier</title>
  <style>
    body{font-family:Arial; padding:16px;}
    .card{border:1px solid #ddd; border-radius:12px; padding:14px; max-width:420px;}
    .row{display:flex; justify-content:space-between; padding:8px 0; border-bottom:1px dashed #eee;}
    .row:last-child{border-bottom:none;}
    .ok{color:green; font-weight:700;}
    .bad{color:red; font-weight:700;}
    .muted{color:#666;}
  </style>
</head>
<body>
  <h2>Smart Cashier - ESP32</h2>

  <div class="card">
    <div class="row"><span class="muted">Product</span><span id="p">-</span></div>
    <div class="row"><span class="muted">Expected (g)</span><span id="ew">0</span></div>
    <div class="row"><span class="muted">Current (g)</span><span id="cw">0</span></div>
    <div class="row"><span class="muted">Result</span><span id="r">waiting</span></div>
  </div>

  <script>
    async function refresh(){
      try{
        const res = await fetch('/status');
        const j = await res.json();
        document.getElementById('p').textContent  = j.product;
        document.getElementById('ew').textContent = j.expected_weight;
        document.getElementById('cw').textContent = j.current_weight;

        const rEl = document.getElementById('r');
        rEl.textContent = j.result;

        rEl.className = '';
        if(j.result === 'correct') rEl.classList.add('ok');
        if(j.result === 'not_correct') rEl.classList.add('bad');
      }catch(e){
        // تجاهل
      }
    }
    setInterval(refresh, 500); // تحديث كل نصف ثانية
    refresh();
  </script>
</body>
</html>
)rawliteral";

// ===== Endpoints =====

// 1) الصفحة الرئيسية
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

// 2) حالة النظام للواجهة (JSON)
void handleStatus() {
  String json = "{";
  json += "\"product\":\"" + last_product + "\",";
  json += "\"expected_weight\":" + String(expected_weight) + ",";
  json += "\"current_weight\":" + String(current_weight) + ",";
  json += "\"result\":\"" + last_result + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// 3) استقبال المنتج من بايثون: /post?product=sanaa
void handlePost() {
  if (!server.hasArg("product")) {
    server.send(400, "text/plain", "Missing product");
    return;
  }

  String product = server.arg("product");
  product.trim();
  product.toLowerCase();

  last_product = product;
  last_result = "processing";

  // تحديد الوزن المتوقع
  if (product == "sanaa") expected_weight = 500;
  else if (product == "kinza") expected_weight = 330;
  else if (product == "sun top") expected_weight = 180;
  else if (product == "haniah") expected_weight = 250;
  else if (product == "marry") expected_weight = 210;
  else {
    last_result = "unknown";
    server.send(400, "text/plain", "Unknown product");
    return;
  }

  // تشغيل الموتور فقط لو المنتج له وزن (حسب منطقك)
    turn_on_motor();

  // فحص الوزن وتحديث النتيجة
  weight_check_and_update_result();

  // رد لبايثون
  server.send(200, "text/plain", last_result);

  // تشخيص
  Serial.print("Product: "); Serial.print(last_product);
  Serial.print(" expected: "); Serial.print(expected_weight);
  Serial.print(" current: "); Serial.print(current_weight);
  Serial.print(" result: "); Serial.println(last_result);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  scale.begin(HX711_DT, HX711_SCK);

  Serial.println("Remove weight...");
  delay(2000);
  scale.set_scale(scale_factor);  // قيمة مبدئية
  scale.tare();


  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // غالباً 192.168.4.1

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/post", handlePost);

  server.begin();
}

void loop() {
  server.handleClient();
}
