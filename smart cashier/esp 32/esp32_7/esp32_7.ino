//http://192.168.4.1/
// fateh
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>

#include "HX711.h"

#define HX711_DT  4
#define HX711_SCK 5
#define RELAY_PIN 18

HX711 scale;
int scale_factor=376.4;

const char* ssid = "ESP32_CASHIER";
const char* password = "12345678";

WebServer server(80);

/////
struct product_info{
  char product_name[16];
  float unit_price = 0;
  int qty = 0;
  float cost;
}sanaa,kinze,sun_top,marry,haniah;

struct List{
  int numOfProduct;
  struct product_info* things[5];
  float total;
};

int product_counter=0;

int last_qty_sanaa;
int last_qty_kinza;
int last_qty_sun_top ;
int last_qty_marry ;
int last_qty_haniah ;
bool sessionON;

////////////

// ===== إعدادات =====
int time_on_motor = 1000;      // زمن تشغيل الموتور

// ===== الحالة التي ستظهر في الواجهة =====
String last_product = "-";
float expected_weight2;
float current_weight2  = 0;
const float rang = 100; // سماحية
int last_qty = 0;
int last_price = 0;

String last_result  = "waiting";   // waiting / correct / not_correct / non_or_other / unknown

// ===== قراءة الوزن (مؤقت) =====
void read_weight_current2() {
  if (scale.is_ready()) {
    current_weight2 = scale.get_units(10); // متوسط 5 قراءات
    Serial.print("Weight: ");
    Serial.print(current_weight2,2);
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

void weight_check_and_update_result2() {
  read_weight_current2();
  if ((abs(current_weight2 - expected_weight2) <= rang) || (abs(current_weight2 - expected_weight2) >= rang) ) {
    last_result = "correct";
  } else {
    last_result = "not_correct";
  }
}
bool weight_check_and_update_result() {
  // delay(400);                 // وقت استقرار بسيط
  //read_weight_current();      // قراءة أولى
  //delay(200);
  //read_weight_current();      // قراءة ثانية

last_result="correct";
return true;
 
}

////////////////
struct List current_list(){
  struct List list = {0}; 
  if(last_qty_sanaa>0){
    strcpy( sanaa.product_name, "Sanaa") ;
    sanaa.unit_price=100;
    sanaa.qty=last_qty_sanaa;
    sanaa.cost=100*last_qty_sanaa;
    list.numOfProduct +=1;
    list.things[(list.numOfProduct -1)] = &sanaa;
  }
    if(last_qty_kinza>0){
    strcpy( kinze.product_name, "kinze") ;      
    kinze.unit_price=100;
    kinze.qty=last_qty_kinza;
    kinze.cost=100*last_qty_kinza;
    list.numOfProduct +=1;
    list.things[(list.numOfProduct -1)] = &kinze;
  }
    if(last_qty_sun_top>0){
    strcpy( sun_top.product_name, "sun_top") ;
    sun_top.unit_price=100;
    sun_top.qty=last_qty_sun_top;
    sun_top.cost=100*last_qty_sun_top;
    list.numOfProduct +=1;
    list.things[(list.numOfProduct -1)] = &sun_top;
  }
    if(last_qty_haniah>0){
    strcpy( haniah.product_name, "haniah") ;
    haniah.unit_price=100;
    haniah.qty=last_qty_haniah;
    haniah.cost=100*last_qty_haniah;
    list.numOfProduct +=1;
    list.things[(list.numOfProduct -1)] = &haniah;
  }
    if(last_qty_marry>0){
    strcpy( marry.product_name, "marry") ;
    marry.unit_price=100;
    marry.qty=last_qty_marry;
    marry.cost=100*last_qty_marry;
    list.numOfProduct +=1;
    list.things[(list.numOfProduct -1)] = &marry;
  }
  for(int i=0; i<list.numOfProduct;i++){
    list.total += list.things[i]->cost;
  }
  return list;
}

void display(struct List myList){
  Serial.println("hukh huikhkh hukhk bukb");

  for(int i=0;i< myList.numOfProduct; i++ ){
    Serial.print(i+1);
    Serial.print(' ');
    Serial.print(myList.things[i]->product_name);
    Serial.print(' ');
    Serial.print(myList.things[i]->unit_price);
    Serial.print(' ');
    Serial.print(myList.things[i]->qty);
    Serial.print(' ');
    Serial.print(myList.things[i]->cost);
    Serial.println();
  }
    Serial.print("        ");
    Serial.println(myList.total);

}
///////////////

// ===== صفحة الويب HTML =====

// 1) الصفحة الرئيسية
// void handleRoot() {
//   server.send(200, "text/html", MAIN_page);
// }

// 2) حالة النظام للواجهة (JSON)
// void handleStatus() {
//   String json = "{";
//   json += "\"product\":\"" + last_product + "\",";
//   json += "\"expected_weight\":" + String(expected_weight) + ",";
//   json += "\"current_weight\":" + String(current_weight) + ",";
//   json += "\"result\":\"" + last_result + "\"";
//   json += "}";
//   server.send(200, "application/json", json);
// }

// 3) استقبال المنتج من بايثون: /post?product=sanaa
void handlePost() {
  if (!server.hasArg("product")) {
    server.send(400, "text/plain", "Missing product");
    return;
  }
  if(!sessionON){
    sessionON=true;
  }
  String product = server.arg("product");
  product.trim();
  product.toLowerCase();

  last_product = product;
  last_result = "processing";
  int type=0;
  // تحديد الوزن المتوقع
  if (product == "sanaa"){
    type=1;
    // last_price = 100;
    expected_weight2 = 500;
  } else if (product == "kinza"){
    type=2;
    // last_price = 200;
    expected_weight2 = 330;
  }else if (product == "sun top"){
    type=3;
    // last_price = 200;
    expected_weight2 = 180;
  }else if (product == "haniah"){
    type=4;
    // last_price = 150;
    expected_weight2 = 250;
  }else if (product == "marry"){
    type=5;
    // last_price = 300;
    expected_weight2 = 210;
  } else {
    expected_weight2 = 0;
    last_result = "unknown";
    server.send(200, "text/plain", last_result);
    return;
}

  turn_on_motor();
  if(weight_check_and_update_result()){
    Serial.println("type");
        Serial.print(type);

    switch(type){
    case 1:
    last_qty_sanaa+=1;
    break;
    case 2:
    last_qty_kinza+=1;
    break;

    case 3:
    last_qty_sun_top+=1;
    break;

    case 4:
    last_qty_haniah+=1;
    break;

    case 5:
    last_qty_marry+=1;
    break;


    }
  }
  



  // رد لبايثون
  server.send(200, "text/plain", last_result);

  // تشخيص
  Serial.print("Product: "); Serial.print(last_product);
  Serial.print(" expected: "); Serial.print(expected_weight2);
  Serial.print(" current: "); Serial.print(current_weight2);
  Serial.print(" result: "); Serial.println(last_result);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  scale.begin(HX711_DT, HX711_SCK);

   while (!scale.is_ready()) {
    Serial.println("HX711 not found / not ready...");
    delay(500);
  }
    Serial.println("HX711 ready.");

  scale.set_scale(scale_factor);  // قيمة مبدئية
  scale.tare();


  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // غالباً 192.168.4.1

  // server.on("/", handleRoot);
  // server.on("/status", handleStatus);
  server.on("/post", handlePost);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/list", HTTP_GET, handleList);

  server.begin();

}

void loop() {
server.handleClient();
// struct List loopList= current_list();
  
if(weight_check_and_update_result()){
  display(current_list());
  delay(200);
}
// read_weight_current();
// delay(2000);
}


// ====== صفحة HTML (نفس اللي قبل + إرسال للـESP) ======
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="ar" dir="rtl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>قائمة المنتجات - لحظي</title>
  <style>
    body { font-family: system-ui, Arial; margin: 16px; }
    h2 { margin: 0 0 12px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; max-width: 700px; }
    .card { border: 1px solid #ddd; border-radius: 10px; padding: 12px; }
    label { display:block; font-size: 14px; margin-bottom: 6px; }
    input { width: 100%; padding: 10px; font-size: 16px; border: 1px solid #ccc; border-radius: 8px; }
    table { width: 100%; border-collapse: collapse; margin-top: 14px; }
    th, td { border-bottom: 1px solid #eee; padding: 10px; text-align: right; }
    th { background: #fafafa; }
    .total { font-size: 18px; font-weight: 700; margin-top: 12px; }
    .muted { color:#666; font-size: 13px; }
    .row { display:flex; gap:10px; align-items:center; justify-content:space-between; }
    button { padding:10px 14px; border-radius: 10px; border:1px solid #ccc; background:#fff; cursor:pointer; }
  </style>
</head>
<body>

<h2>واجهة تحاكي current_list() و display()</h2>
<div class="muted">كل تغيير في الكمية ينرسل للـ ESP ويحفظ last_qty_* ثم يتم حساب القائمة لحظيًا داخل الصفحة.</div>

<div class="row" style="margin-top:12px; max-width:700px;">
  <div class="muted">unit_price ثابت = 100</div>
  <button type="button" onclick="resetAll()">تصفير الكميات</button>
</div>

<div class="grid" style="margin-top:12px;">
  <div class="card"><label>كمية Sanaa</label><input id="qty_sanaa" type="number" min="0" value="0" inputmode="numeric"></div>
  <div class="card"><label>كمية kinze</label><input id="qty_kinza" type="number" min="0" value="0" inputmode="numeric"></div>
  <div class="card"><label>كمية sun_top</label><input id="qty_sun_top" type="number" min="0" value="0" inputmode="numeric"></div>
  <div class="card"><label>كمية haniah</label><input id="qty_haniah" type="number" min="0" value="0" inputmode="numeric"></div>
  <div class="card"><label>كمية marry</label><input id="qty_marry" type="number" min="0" value="0" inputmode="numeric"></div>
</div>

<table>
  <thead>
    <tr>
      <th>#</th><th>المنتج</th><th>سعر الوحدة</th><th>الكمية</th><th>التكلفة</th>
    </tr>
  </thead>
  <tbody id="tbody"></tbody>
</table>

<div class="total">الإجمالي: <span id="total">0</span></div>

<script>
  const UNIT_PRICE = 100;

  const products = [
    { key: "sanaa",   name: "Sanaa"   },
    { key: "kinza",   name: "kinze"   },
    { key: "sun_top", name: "sun_top" },
    { key: "haniah",  name: "haniah"  },
    { key: "marry",   name: "marry"   },
  ];

  function clampQty(v){
    const n = Number(v);
    if (!Number.isFinite(n) || n < 0) return 0;
    return Math.floor(n);
  }

  function current_list() {
    const list = { numOfProduct: 0, things: [], total: 0 };

    for (const p of products) {
      const qty = clampQty(document.getElementById("qty_" + p.key).value);
      if (qty > 0) {
        const item = { product_name: p.name, unit_price: UNIT_PRICE, qty, cost: UNIT_PRICE * qty };
        list.numOfProduct += 1;
        list.things.push(item);
      }
    }

    for (let i = 0; i < list.numOfProduct; i++) list.total += list.things[i].cost;
    return list;
  }

  function display(list) {
    const tbody = document.getElementById("tbody");
    tbody.innerHTML = "";
    for (let i = 0; i < list.numOfProduct; i++) {
      const it = list.things[i];
      const tr = document.createElement("tr");
      tr.innerHTML = `<td>${i+1}</td><td>${it.product_name}</td><td>${it.unit_price}</td><td>${it.qty}</td><td>${it.cost}</td>`;
      tbody.appendChild(tr);
    }
    document.getElementById("total").textContent = list.total;
  }

  // إرسال قيمة للـ ESP عشان يخزن last_qty_*
  // (Debounce بسيط عشان ما نرسل طلب مع كل رقم بسرعة)
  let t = null;
  function sendToESP(prodKey, qty){
    if (t) clearTimeout(t);
    t = setTimeout(() => {
      fetch(`/set?prod=${encodeURIComponent(prodKey)}&qty=${encodeURIComponent(qty)}`)
        .catch(()=>{});
    }, 120);
  }

  function updateUI(changedKey=null) {
    // تحديث لحظي داخل الصفحة
    const list = current_list();
    display(list);

    // إذا فيه خانة تغيرت: نرسلها للـ ESP
    if (changedKey) {
      const qty = clampQty(document.getElementById("qty_" + changedKey).value);
      sendToESP(changedKey, qty);
    }
  }

  function resetAll(){
    for (const p of products) {
      document.getElementById("qty_" + p.key).value = 0;
      sendToESP(p.key, 0);
    }
    updateUI();
  }

  // تحميل القيم المخزنة من ESP (اختياري)
  async function loadFromESP(){
    try{
      const r = await fetch('/list');
      const j = await r.json();
      // توقع JSON: { sanaa:0, kinza:0, sun_top:0, haniah:0, marry:0 }
      for (const p of products) {
        if (typeof j[p.key] === 'number') {
          document.getElementById("qty_" + p.key).value = j[p.key];
        }
      }
    }catch(e){}
    updateUI();
  }

  for (const p of products) {
    document.getElementById("qty_" + p.key).addEventListener("input", () => updateUI(p.key));
  }

  loadFromESP();
</script>
</body>
</html>
)rawliteral";

// ====== Handlers ======

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

// /set?prod=sanaa&qty=3
void handleSet() {
  if (!server.hasArg("prod") || !server.hasArg("qty")) {
    server.send(400, "text/plain", "Missing prod or qty");
    return;
  }

  String prod = server.arg("prod");
  int qty = server.arg("qty").toInt();
  if (qty < 0) qty = 0;

  if (prod == "sanaa") last_qty_sanaa = qty;
  else if (prod == "kinza") last_qty_kinza = qty;
  else if (prod == "sun_top") last_qty_sun_top = qty;
  else if (prod == "haniah") last_qty_haniah = qty;
  else if (prod == "marry") last_qty_marry = qty;
  else {
    server.send(400, "text/plain", "Unknown prod");
    return;
  }

  server.send(200, "text/plain", "OK");
}

// يرجع القيم المخزنة (مفيد عشان الصفحة تسحب آخر حالة)
void handleList() {
  String json = "{";
  json += "\"sanaa\":" + String(last_qty_sanaa) + ",";
  json += "\"kinza\":" + String(last_qty_kinza) + ",";
  json += "\"sun_top\":" + String(last_qty_sun_top) + ",";
  json += "\"haniah\":" + String(last_qty_haniah) + ",";
  json += "\"marry\":" + String(last_qty_marry);
  json += "}";
  server.send(200, "application/json", json);
}