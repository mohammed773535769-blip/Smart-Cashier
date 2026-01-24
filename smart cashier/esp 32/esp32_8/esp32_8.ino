// http://192.168.4.1/
// fateh

#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <string.h>
#include "HX711.h"

#define HX711_DT   4
#define HX711_SCK  5
#define RELAY_PIN  18

HX711 scale;
float scale_factor = 376.4;

const char* ssid = "ESP32_CASHIER";
const char* password = "12345678";
WebServer server(80);

// ===================== المنتجات =====================
struct product_info {
  char  product_name[16];
  float unit_price;
  int   qty;
  float cost;
};

product_info sanaa, kinze, sun_top, marry, haniah;

struct List {
  int numOfProduct;
  product_info* things[5];
  float total;
};

int last_qty_sanaa   = 0;
int last_qty_kinza   = 0;
int last_qty_sun_top = 0;
int last_qty_marry   = 0;
int last_qty_haniah  = 0;

bool sessionON = false;

// ===================== إعدادات =====================
int time_on_motor = 2000;

String last_product = "-";
float expected_weight2 = 0;  // وزن آخر منتج للعرض
float current_weight2  = 0;
const float rang = 100;

String last_result = "waiting"; // waiting / processing / correct / not_correct / unknown

// ===================== أوزان المنتجات (جرام) =====================
const float W_SANAA   = 500;
const float W_KINZA   = 330;
const float W_SUN_TOP = 270;
const float W_HANIAH  = 250;
const float W_MARRY   = 210;

// ===================== واجهة ويب =====================
const char LIST_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="ar" dir="rtl">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Cashier</title>
<style>
  body{
    font-family:system-ui,Arial;
    background:#e3f2fd; /* أزرق فاتح */
    margin:12px
  }

  .card{
    background:#ffffff;
    padding:12px;
    border-radius:14px;
    box-shadow:0 6px 20px rgba(30,136,229,.18);
    margin-bottom:10px;
    border-top:4px solid #1e88e5; /* أزرق */
  }

  table{
    width:100%;
    border-collapse:collapse;
    margin-top:10px
  }

  th,td{
    padding:10px;
    border-bottom:1px solid #e0e0e0;
    text-align:right
  }

  th{
    background:#e3f2fd;
    color:#1e88e5;
    font-weight:800
  }

  .row{display:flex; gap:10px; flex-wrap:wrap}

  .k{
    color:#1565c0;
    font-size:13px;
    font-weight:700
  }

  .v{
    font-size:20px;
    font-weight:800;
    color:#0d47a1
  }

  .badge{
    display:inline-block;
    padding:6px 12px;
    border-radius:999px;
    font-weight:800;
    color:#fff
  }

  .waiting{background:#90a4ae}
  .processing{background:#fb8c00}   /* برتقالي */
  .correct{background:#43a047}      /* أخضر */
  .not_correct{background:#e53935}  /* أحمر */
  .unknown{background:#5e35b1}

  button{
    padding:10px 16px;
    border:0;
    border-radius:14px;
    font-weight:900;
    cursor:pointer;
    color:#fff;
    background:linear-gradient(135deg,#1e88e5,#1565c0);
    box-shadow:0 4px 12px rgba(30,136,229,.4)
  }

  button#btnReset{
    background:linear-gradient(135deg,#fb8c00,#ef6c00);
    box-shadow:0 4px 12px rgba(251,140,0,.45)
  }

  button:disabled{
    opacity:.6;
    cursor:not-allowed
  }

  .small{
    color:#424242;
    font-size:13px
  }
</style>

</head>

<body>

  <div class="card">
    <div class="row">
      <div style="flex:1; min-width:160px">
        <div class="k">آخر منتج</div>
        <div class="v" id="product">-</div>
      </div>
      <div style="flex:1; min-width:160px">
        <div class="k">النتيجة</div>
        <div class="v"><span id="badge" class="badge waiting">waiting</span></div>
      </div>
    </div>
    <div class="row" style="margin-top:10px">
      <div style="flex:1; min-width:160px">
        <div class="k">الوزن المتوقع</div>
        <div class="v"><span id="ew">0</span> g</div>
      </div>
      <div style="flex:1; min-width:160px">
        <div class="k">الوزن الحالي</div>
        <div class="v"><span id="cw">0</span> g</div>
      </div>
    </div>
  </div>

  <div class="card">
    <div class="k">السلة (حسب current_list)</div>
    <table>
      <thead>
        <tr><th>#</th><th>المنتج</th><th>السعر</th><th>الكمية</th><th>التكلفة</th></tr>
      </thead>
      <tbody id="tbody"></tbody>
      <tfoot>
        <tr><th colspan="4">المجموع</th><th id="total">0</th></tr>
      </tfoot>
    </table>
  </div>

  <div class="card">
    <div class="row" style="align-items:center; justify-content:space-between;">
      <div>
        <div class="k">التحكم</div>
        <div class="small">تأكيد: يتحقق بعد 4 ثواني — مسح: يلغي الجلسة ويمسح السلة</div>
      </div>
      <div class="row" style="gap:8px">
        <button id="btnConfirm" onclick="confirmNow()">تأكيد</button>
        <button id="btnReset" onclick="resetAll()">مسح الكل</button>
      </div>
    </div>
    <div class="small" id="msg"></div>
  </div>

<script>
function setBadge(text){
  const b = document.getElementById('badge');
  b.textContent = text;
  b.className = 'badge ' + text;
}

async function refresh(){
  try{
    const r = await fetch('/list', {cache:'no-store'});
    const j = await r.json();

    document.getElementById('product').textContent = j.product ?? '-';
    document.getElementById('ew').textContent = Number(j.expected_weight ?? 0).toFixed(2);
    document.getElementById('cw').textContent = Number(j.current_weight ?? 0).toFixed(2);
    document.getElementById('total').textContent = Number(j.total ?? 0).toFixed(2);
    setBadge(j.result ?? 'waiting');

    const tb = document.getElementById('tbody');
    tb.innerHTML = '';
    (j.items || []).forEach((it, idx)=>{
      const tr = document.createElement('tr');
      tr.innerHTML = `
        <td>${idx+1}</td>
        <td>${it.name}</td>
        <td>${Number(it.price).toFixed(2)}</td>
        <td>${it.qty}</td>
        <td>${Number(it.cost).toFixed(2)}</td>
      `;
      tb.appendChild(tr);
    });
  }catch(e){}
}

async function confirmNow(){
  const btn = document.getElementById('btnConfirm');
  const msg = document.getElementById('msg');

  btn.disabled = true;
  msg.textContent = 'تم الإرسال... سيتم التحقق بعد 4 ثواني';

  try{
    const r = await fetch('/confirm', {cache:'no-store'});
    const t = await r.text();
    msg.textContent = t;
  }catch(e){
    msg.textContent = 'خطأ في الاتصال';
  }finally{
    btn.disabled = false;
  }
}

async function resetAll(){
  const btn = document.getElementById('btnReset');
  const msg = document.getElementById('msg');

  if(!confirm('متأكد تريد مسح كل البيانات؟')) return;

  btn.disabled = true;
  msg.textContent = 'جاري مسح البيانات...';

  try{
    const r = await fetch('/reset', {cache:'no-store'});
    const t = await r.text();
    msg.textContent = t;
  }catch(e){
    msg.textContent = 'خطأ في الاتصال';
  }finally{
    btn.disabled = false;
  }
}

refresh();
setInterval(refresh, 500);
</script>

</body>
</html>
)rawliteral";


// ===================== وزن + موتور =====================
void read_weight_current2() {
  if (scale.is_ready()) {
    current_weight2 = scale.get_units(10);
  }
}

bool turn_on_motor() {
  digitalWrite(RELAY_PIN, HIGH);
  delay(time_on_motor);
  digitalWrite(RELAY_PIN, LOW);
  return true;
}

float expected_session_weight() {
  float sum = 0;
  sum += (float)last_qty_sanaa   * W_SANAA;
  sum += (float)last_qty_kinza   * W_KINZA;
  sum += (float)last_qty_sun_top * W_SUN_TOP;
  sum += (float)last_qty_haniah  * W_HANIAH;
  sum += (float)last_qty_marry   * W_MARRY;
  return sum;
}

// ===================== current_list() =====================
List current_list() {
  List list;
  memset(&list, 0, sizeof(list));

  if (last_qty_sanaa > 0) {
    strcpy(sanaa.product_name, "Sanaa");
    sanaa.unit_price = 100;
    sanaa.qty = last_qty_sanaa;
    sanaa.cost = sanaa.unit_price * sanaa.qty;
    list.things[list.numOfProduct++] = &sanaa;
  }

  if (last_qty_kinza > 0) {
    strcpy(kinze.product_name, "kinze");
    kinze.unit_price = 250;
    kinze.qty = last_qty_kinza;
    kinze.cost = kinze.unit_price * kinze.qty;
    list.things[list.numOfProduct++] = &kinze;
  }

  if (last_qty_sun_top > 0) {
    strcpy(sun_top.product_name, "sun_top");
    sun_top.unit_price = 200;
    sun_top.qty = last_qty_sun_top;
    sun_top.cost = sun_top.unit_price * sun_top.qty;
    list.things[list.numOfProduct++] = &sun_top;
  }

  if (last_qty_haniah > 0) {
    strcpy(haniah.product_name, "haniah");
    haniah.unit_price = 150;
    haniah.qty = last_qty_haniah;
    haniah.cost = haniah.unit_price * haniah.qty;
    list.things[list.numOfProduct++] = &haniah;
  }

  if (last_qty_marry > 0) {
    strcpy(marry.product_name, "marry");
    marry.unit_price = 300;
    marry.qty = last_qty_marry;
    marry.cost = marry.unit_price * marry.qty;
    list.things[list.numOfProduct++] = &marry;
  }

  for (int i = 0; i < list.numOfProduct; i++) {
    list.total += list.things[i]->cost;
  }
  return list;
}
void handleReset() {
  // مسح الكميات
  last_qty_sanaa   = 0;
  last_qty_kinza   = 0;
  last_qty_sun_top = 0;
  last_qty_marry   = 0;
  last_qty_haniah  = 0;

  // مسح حالة الجلسة/الواجهة
  sessionON = false;
  last_product = "-";
  expected_weight2 = 0;
  current_weight2  = 0;
  last_result = "waiting";

  server.send(200, "text/plain", "تم مسح كل البيانات وإلغاء الجلسة ✅");

  Serial.println("=== RESET ALL ===");
}

void display(List myList) {
  Serial.println("---- CART ----");
  for (int i = 0; i < myList.numOfProduct; i++) {
    Serial.print(i + 1); Serial.print(' ');
    Serial.print(myList.things[i]->product_name); Serial.print(' ');
    Serial.print(myList.things[i]->unit_price); Serial.print(' ');
    Serial.print(myList.things[i]->qty); Serial.print(' ');
    Serial.print(myList.things[i]->cost);
    Serial.println();
  }
  Serial.print("TOTAL: ");
  Serial.println(myList.total);
}

// ===================== Routes =====================
void handleRoot() {
  server.send(200, "text/html", LIST_PAGE);
}

void handleList() {
  if (scale.is_ready()) current_weight2 = scale.get_units(3);

  List L = current_list();

  String json = "{";
  json += "\"product\":\"" + last_product + "\",";
  json += "\"expected_weight\":" + String(expected_weight2, 2) + ",";
  json += "\"current_weight\":" + String(current_weight2, 2) + ",";
  json += "\"result\":\"" + last_result + "\",";
  json += "\"total\":" + String(L.total, 2) + ",";
  json += "\"items\":[";

  for (int i = 0; i < L.numOfProduct; i++) {
    json += "{";
    json += "\"name\":\"" + String(L.things[i]->product_name) + "\",";
    json += "\"price\":" + String(L.things[i]->unit_price, 2) + ",";
    json += "\"qty\":" + String(L.things[i]->qty) + ",";
    json += "\"cost\":" + String(L.things[i]->cost, 2);
    json += "}";
    if (i < L.numOfProduct - 1) json += ",";
  }

  json += "]}";
  server.send(200, "application/json", json);
}

// إضافة منتج: /post?product=sanaa
void handlePost() {
  if (!server.hasArg("product")) {
    server.send(400, "text/plain", "Missing product");
    return;
  }

  String product = server.arg("product");
  product.trim();
  product.toLowerCase();

  last_product = product;
  last_result  = "processing";

  int type = 0;

  if (product == "sanaa") {
    type = 1; expected_weight2 = W_SANAA;
  } else if (product == "kinza") {
    type = 2; expected_weight2 = W_KINZA;
  } else if (product == "sun top" || product == "sun_top") {
    type = 3; expected_weight2 = W_SUN_TOP;
  } else if (product == "haniah") {
    type = 4; expected_weight2 = W_HANIAH;
  } else if (product == "marry") {
    type = 5; expected_weight2 = W_MARRY;
  } else {
    expected_weight2 = 0;
    last_result = "unknown";
    server.send(200, "text/plain", last_result);
    return;
  }

  // تشغيل الموتور أولاً
  bool ok = turn_on_motor();

  // الجلسة تبدأ بعد تشغيل الموتور
  if (ok && !sessionON) sessionON = true;

  // زيادة الكميات بدون تحقق وزن
  if (ok) {
    switch (type) {
      case 1: last_qty_sanaa++; break;
      case 2: last_qty_kinza++; break;
      case 3: last_qty_sun_top++; break;
      case 4: last_qty_haniah++; break;
      case 5: last_qty_marry++; break;
    }
    last_result = "waiting";
  }

  server.send(200, "text/plain", "ok");
  display(current_list());
}

// زر التأكيد: ينتظر 4 ثواني ثم يتحقق
void handleConfirm() {
  if (!sessionON) {
    last_result = "waiting";
    server.send(200, "text/plain", "لا توجد جلسة مفتوحة");
    return;
  }

  last_result = "processing";

  delay(4000);          // ✅ 4 ثواني
  turn_on_motor();
  read_weight_current2();

  float expected = expected_session_weight();
  float diff = fabs(current_weight2 - expected);

  if (diff <= rang) last_result = "correct";
  else last_result = "not_correct";

  sessionON = false;

  String reply = "result=" + last_result
              + ", expected=" + String(expected, 2)
              + ", current=" + String(current_weight2, 2);

  server.send(200, "text/plain", reply);

  Serial.println("=== CONFIRM ===");
  Serial.println(reply);
}

// ===================== setup/loop =====================
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

  scale.set_scale(scale_factor);
  scale.tare();

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/list", handleList);
  server.on("/post", handlePost);
  server.on("/confirm", handleConfirm);
  server.on("/reset", handleReset);
  server.begin();

  Serial.println("Open: http://192.168.4.1/");
}

void loop() {
  server.handleClient();
}
