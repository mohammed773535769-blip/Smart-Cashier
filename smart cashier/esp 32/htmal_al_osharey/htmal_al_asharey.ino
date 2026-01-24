#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ap_ssid = "SMART_CASHIER";
const char* ap_pass = "12345678";

WebServer server(80);

// =======================
// عدّل هنا المنتجات بسهولة
// =======================
String x = "حليب نادك 1 لتر";
String y = "خبز توست";

float  p1_price = 6.50;
int    p1_qty   = 1;

float  p2_price = 4.00;
int    p2_qty   = 1;

// =======================
// آخر منتج يظهر في الكرت
// =======================
String last_name  = y;
float  last_price = p2_price;
int    last_qty   = p2_qty;

String last_image = "/img/bread.jpg";
bool   last_weight_ok = true;
bool   last_scale_error = false;
String last_msg = "تمت الإضافة إلى السلة ✅";

// الحالة الحالية التي يقرأها المتصفح
String latestJson;

// ====== أدوات ======
String fmt2(float v){
  char b[24];
  snprintf(b, sizeof(b), "%.2f", v);
  return String(b);
}

// ====== حالة فاضية (Reset) ======
String buildEmptyJson(){
  return R"({
    "last":{
      "name":"بانتظار منتج...",
      "price":0,
      "qty":0,
      "image":"/img/p1.jpg",
      "weight_ok":true,
      "scale_error":false,
      "msg":"ضع المنتج أمام الكاميرا"
    },
    "cart":[],
    "total":0
  })";
}

// ====== مثال سلة فيها منتجين ======
String buildJson(){
  String p1_name = x;
  String p2_name = y;

  float total = (p1_price * p1_qty) + (p2_price * p2_qty);

  String j = "{";

  j += "\"last\":{";
  j += "\"name\":\"" + last_name + "\",";
  j += "\"price\":" + fmt2(last_price) + ",";
  j += "\"qty\":" + String(last_qty) + ",";
  j += "\"image\":\"" + last_image + "\",";
  j += "\"weight_ok\":" + String(last_weight_ok ? "true" : "false") + ",";
  j += "\"scale_error\":" + String(last_scale_error ? "true" : "false") + ",";
  j += "\"msg\":\"" + last_msg + "\"";
  j += "},";

  j += "\"cart\":[";
  j += "{\"name\":\"" + p1_name + "\",\"price\":" + fmt2(p1_price) + ",\"qty\":" + String(p1_qty) + "},";
  j += "{\"name\":\"" + p2_name + "\",\"price\":" + fmt2(p2_price) + ",\"qty\":" + String(p2_qty) + "}";
  j += "],";

  j += "\"total\":" + fmt2(total);
  j += "}";
  server.send(200, "application/json", j);

}

// ====== صفحة الويب ======
String page(){
  return R"rawliteral(
<!doctype html>
<html lang="ar" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>الكاشير الذكي</title>

<style>
:root{
  --navy:#071a33;
  --navy2:#0b2344;
  --navy3:#0f2b52;

  --orange:#ff8a00;
  --orange2:#ffc178;

  --text:#ffffff;
  --muted:#b9c7da;

  --ok:#26d07c;
  --bad:#ff4d4d;

  --border:1px solid rgba(255,255,255,.10);
  --shadow:0 18px 40px rgba(0,0,0,.35);
  --r:18px;
}

*{box-sizing:border-box}

body{
  margin:0;
  font-family:Arial, Tahoma, sans-serif;
  color:var(--text);
  background:radial-gradient(1200px 700px at 20% 0%, #163f73, var(--navy));
}

.wrap{
  max-width:1280px;
  margin:0 auto;
  padding:18px 18px 110px;
}

/* HEADER */
.top{
  display:flex;
  justify-content:space-between;
  align-items:center;
  gap:12px;
  padding:16px 18px;
  border-radius:24px;
  background:linear-gradient(135deg, var(--navy3), var(--navy2));
  border:2px solid rgba(255,138,0,.45);
  box-shadow:var(--shadow);
}

.brand{display:flex;align-items:center;gap:14px;}

.logo{
  width:46px;height:46px;border-radius:16px;
  background:linear-gradient(135deg, var(--orange), var(--orange2));
  color:#081a31;
  display:grid;place-items:center;
  font-weight:1000;
}

.title{font-size:22px;font-weight:1000;margin:0;}
.sub{color:var(--orange);font-size:12px;font-weight:800;}

.pill{
  padding:10px 16px;
  border-radius:999px;
  font-size:13px;
  font-weight:1000;
  background:rgba(255,138,0,.18);
  border:2px solid rgba(255,138,0,.45);
  color:var(--orange);
}

/* GRID */
.grid{
  margin-top:18px;
  display:grid;
  gap:18px;
  grid-template-columns: 1.3fr 0.7fr;
}

/* CARDS */
.card{
  position:relative;
  background:linear-gradient(180deg, rgba(255,255,255,.07), rgba(255,255,255,.03));
  border:var(--border);
  border-radius:var(--r);
  box-shadow:var(--shadow);
  padding:18px;
}

.card::before{
  content:"";
  position:absolute;
  top:0; left:0; right:0;
  height:4px;
  background:linear-gradient(90deg, transparent, var(--orange), transparent);
}

/* LAST PRODUCT */
.last{
  display:grid;
  grid-template-columns: 1fr 1.2fr;
  gap:18px;
}

.imgBox{
  border-radius:20px;
  overflow:hidden;
  border:2px solid rgba(255,138,0,.45);
  background:#081a31;
  min-height:320px;
}

.imgBox img{width:100%;height:100%;object-fit:cover;}

.info{display:flex;flex-direction:column;gap:12px;}

.name{font-size:28px;font-weight:1000;}

.price{
  font-size:40px;
  font-weight:1000;
  color:var(--orange);
  text-shadow:0 10px 26px rgba(255,138,0,.35);
}

.krow{display:flex;gap:10px;flex-wrap:wrap;}

.chip{
  padding:8px 14px;
  border-radius:999px;
  font-size:13px;
  font-weight:900;
  background:rgba(255,255,255,.08);
  border:var(--border);
  color:var(--muted);
}

.chip.ok{background:rgba(38,208,124,.18); color:#c9ffe4;}
.chip.bad{background:rgba(255,77,77,.18); color:#ffd6d6;}

.msg{
  margin-top:auto;
  padding:14px;
  border-radius:16px;
  background:linear-gradient(135deg, rgba(255,138,0,.22), rgba(7,26,51,.55));
  border:2px solid rgba(255,138,0,.45);
  font-weight:1000;
}

/* CART */
table{width:100%;border-collapse:collapse;}
th,td{padding:12px;border-bottom:1px solid rgba(255,255,255,.08);font-size:14px;}
th{background:rgba(255,138,0,.18);color:#fff;}

.totalBox{
  margin-top:16px;
  padding:18px;
  border-radius:20px;
  background:linear-gradient(135deg, rgba(255,138,0,.28), rgba(7,26,51,.65));
  border:2px solid rgba(255,138,0,.45);
  display:flex;
  justify-content:space-between;
  align-items:center;
}

.tVal{font-size:42px;font-weight:1000;color:var(--orange);}

/* BOTTOM BAR */
.bottomBar{
  position:fixed;
  left:0; right:0; bottom:0;
  background:linear-gradient(135deg, var(--navy3), #081a31);
  border-top:3px solid var(--orange);
  padding:16px;
}

.bottomInner{
  max-width:1280px;
  margin:0 auto;
  display:flex;
  justify-content:space-between;
  align-items:center;
  gap:12px;
}

.btns{display:flex;gap:14px;}

button{
  border:none;
  cursor:pointer;
  padding:14px 20px;
  border-radius:16px;
  font-weight:1000;
  font-size:16px;
  min-width:180px;
}

.pay{
  background:linear-gradient(135deg, var(--orange), var(--orange2));
  color:#081a31;
  box-shadow:0 14px 30px rgba(255,138,0,.45);
}

.reject{
  background:rgba(255,77,77,.22);
  border:1px solid rgba(255,77,77,.45);
  color:#ffd6d6;
}

.retry{
  background:rgba(255,255,255,.10);
  border:1px solid rgba(255,255,255,.18);
  color:#eaf2ff;
}

@media(max-width:980px){
  .grid{grid-template-columns:1fr;}
  .last{grid-template-columns:1fr;}
}

/* ===== Popup + Arrow ===== */
.overlay{
  position:fixed; inset:0;
  background:rgba(0,0,0,.55);
  backdrop-filter: blur(4px);
  z-index:9998;
}

.popup{
  position:fixed;
  left:50%; top:50%;
  transform:translate(-50%,-50%);
  width:min(520px,92vw);
  border-radius:22px;
  padding:18px;
  background:linear-gradient(135deg, rgba(255,255,255,.10), rgba(255,255,255,.06));
  border:2px solid rgba(255,138,0,.55);
  box-shadow:0 22px 60px rgba(0,0,0,.5);
  z-index:9999;
}

.popTitle{
  font-size:20px;
  font-weight:1000;
  margin-bottom:8px;
  color:var(--orange);
}

.popMsg{
  font-size:16px;
  font-weight:900;
  color:#eaf2ff;
  line-height:1.7;
}

.popBtns{
  margin-top:14px;
  display:flex;
  justify-content:flex-end;
}

.popOk{
  padding:12px 16px;
  border-radius:14px;
  font-weight:1000;
  background:linear-gradient(135deg,var(--orange),var(--orange2));
  color:#081a31;
  min-width:120px;
}

.arrow{
  position:fixed;
  z-index:10000;
  font-size:28px;
  font-weight:1000;
  filter: drop-shadow(0 10px 20px rgba(0,0,0,.6));
  animation: bob .7s ease-in-out infinite alternate;
}

@keyframes bob{
  from{ transform:translateY(0); }
  to{ transform:translateY(10px); }
}

.hidden{ display:none; }

.glow{
  outline:4px solid rgba(255,138,0,.65);
  box-shadow:0 0 0 6px rgba(255,138,0,.18), 0 20px 40px rgba(255,138,0,.35);
}
</style>
</head>

<body>
<div class="wrap">
  <div class="top">
    <div class="brand">
      <div class="logo">SC</div>
      <div>
        <div class="title">الكاشير الذكي</div>
        <div class="sub">آخر منتج + السلة النهائية</div>
      </div>
    </div>
    <div class="pill" id="status">جاهز</div>
  </div>

  <div class="grid">
    <div class="card">
      <div class="last">
        <div class="imgBox">
          <img id="pimg" src="/img/p1.jpg">
        </div>
        <div class="info">
          <div class="name" id="pname">-</div>
          <div class="price"><span id="pprice">0.00</span> ريال</div>
          <div class="krow">
            <span class="chip" id="pqty">العدد: 0</span>
            <span class="chip" id="wchip">الوزن</span>
            <span class="chip" id="schip">الميزان</span>
          </div>
          <div class="msg" id="msg">-</div>
        </div>
      </div>
    </div>

    <div class="card">
      <table>
        <thead>
          <tr><th>المنتج</th><th>سعر</th><th>عدد</th><th>مجموع</th></tr>
        </thead>
        <tbody id="cartBody">
          <tr><td colspan="4">السلة فارغة</td></tr>
        </tbody>
      </table>

      <div class="totalBox">
        <div>الإجمالي</div>
        <div class="tVal" id="total">0.00</div>
      </div>
    </div>
  </div>
</div>

<div class="bottomBar">
  <div class="bottomInner">
    <div>أزرار التحكم</div>
    <div class="btns">
      <button id="btnRetry" class="retry" onclick="act('retry')">إعادة</button>
      <button id="btnReject" class="reject" onclick="act('reject')">رفض</button>
      <button id="btnPay" class="pay" onclick="act('pay')">دفع</button>
    </div>
  </div>
</div>

<!-- POPUP + ARROW -->
<div id="overlay" class="overlay hidden" onclick="hidePopup()"></div>

<div id="popup" class="popup hidden" role="dialog" aria-modal="true">
  <div class="popTitle" id="popTitle">-</div>
  <div class="popMsg" id="popMsg">-</div>
  <div class="popBtns">
    <button class="popOk" onclick="hidePopup()">موافق</button>
  </div>
</div>

<div id="arrow" class="arrow hidden">⬇</div>

<script>
function money(x){return Number(x||0).toFixed(2)}

function showPopup(title, msg, btnEl){
  popTitle.innerText = title;
  popMsg.innerText = msg;

  overlay.classList.remove('hidden');
  popup.classList.remove('hidden');

  document.querySelectorAll('.glow').forEach(e=>e.classList.remove('glow'));
  arrow.classList.add('hidden');

  if(btnEl){
    btnEl.classList.add('glow');
    const r = btnEl.getBoundingClientRect();
    arrow.style.left = (r.left + r.width/2) + "px";
    arrow.style.top  = (r.top - 38) + "px";
    arrow.style.transform = "translateX(-50%)";
    arrow.classList.remove('hidden');
  }
}

function hidePopup(){
  overlay.classList.add('hidden');
  popup.classList.add('hidden');
  arrow.classList.add('hidden');
  document.querySelectorAll('.glow').forEach(e=>e.classList.remove('glow'));
}

async function act(a){
  await fetch('/api/action',{
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({action:a})
  });

  if(a === 'pay'){
    showPopup("✅ عملية الدفع", "تم الدفع بنجاح (وسيتم تصفير السلة)", btnPay);
  }else if(a === 'reject'){
    showPopup("😅 تم الرفض", "حرام عليك", btnReject);
  }else if(a === 'retry'){
    showPopup("🔁 إعادة", "تمت إعادة الضبط (تصفير البيانات)", btnRetry);
  }
}

function setChip(el, ok, okText, badText){
  el.classList.remove('ok','bad')
  if(ok === true){ el.classList.add('ok'); el.innerText = okText }
  else if(ok === false){ el.classList.add('bad'); el.innerText = badText }
  else { el.innerText = okText }
}

async function refresh(){
  try{
    const r = await fetch('/api/state',{cache:'no-store'})
    const j = await r.json()
    const l = j.last || {}

    pname.innerText  = l.name || "-"
    pprice.innerText = money(l.price)
    pqty.innerText   = "العدد: "+(l.qty||0)
    msg.innerText    = l.msg || "-"
    total.innerText  = money(j.total)

    pimg.src = (l.image||"/img/p1.jpg")+"?t="+Date.now()

    setChip(wchip, !!l.weight_ok, "الوزن: مطابق", "الوزن: غير مطابق")
    setChip(schip, !l.scale_error, "الميزان: سليم", "الميزان: خطأ")

    const ok = (!!l.weight_ok) && (!l.scale_error)
    status.innerText = ok ? "جاهز" : "تحقّق"

    const cart = Array.isArray(j.cart) ? j.cart : []
    if(cart.length === 0){
      cartBody.innerHTML = `<tr><td colspan="4">السلة فارغة</td></tr>`
    }else{
      cartBody.innerHTML = cart.map(it=>{
        const name = it.name || "-"
        const price = Number(it.price||0)
        const qty = Number(it.qty||0)
        const sum = price * qty
        return `<tr>
          <td>${name}</td>
          <td>${money(price)}</td>
          <td>${qty}</td>
          <td>${money(sum)}</td>
        </tr>`
      }).join("")
    }
  }catch(e){
    status.innerText = "غير متصل"
  }
}

setInterval(refresh,600); refresh()
</script>
</body>
</html>
)rawliteral";
}

// ====== Handlers ======
void handleRoot(){ server.send(200,"text/html; charset=utf-8",page()); }
void handleState(){ server.send(200,"application/json", latestJson); }

void handleUpdate(){
  if(!server.hasArg("plain")){ server.send(400,"text/plain","Missing body"); return; }
  latestJson = server.arg("plain");
  server.send(200,"text/plain","OK");
}

// Reset عند pay و retry
void handleAction(){
  if(!server.hasArg("plain")){
    server.send(400,"text/plain","Missing body");
    return;
  }

  String body = server.arg("plain"); // {"action":"pay"}
  String action = "";

  int i = body.indexOf("\"action\"");
  if(i >= 0){
    int c  = body.indexOf(":", i);
    int q1 = body.indexOf("\"", c);
    int q2 = body.indexOf("\"", q1+1);
    if(q1 >= 0 && q2 > q1) action = body.substring(q1+1, q2);
  }

  if(action == "retry" || action == "pay"){
    latestJson = buildEmptyJson();  // ✅ تصفير البيانات
  }

  server.send(200,"text/plain","OK");
}

// ====== LittleFS static files ======
String contentType(const String& p){
  if(p.endsWith(".html")) return "text/html; charset=utf-8";
  if(p.endsWith(".css"))  return "text/css";
  if(p.endsWith(".js"))   return "application/javascript";
  if(p.endsWith(".jpg"))  return "image/jpeg";
  if(p.endsWith(".png"))  return "image/png";
  return "application/octet-stream";
}

bool handleFile(String path){
  if(!LittleFS.exists(path)) return false;
  File f = LittleFS.open(path,"r");
  server.streamFile(f, contentType(path));
  f.close();
  return true;
}

void setup(){
  Serial.begin(115200);
  LittleFS.begin(true);
  WiFi.softAP(ap_ssid, ap_pass);

  // ابدأ بمثال (منتجين)
  latestJson = buildJson();

  // لو تبغى يبدأ فاضي بدل المثال، استخدم هذا:
  // latestJson = buildEmptyJson();

  server.on("/", handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/update", HTTP_POST, handleUpdate);
  server.on("/api/action", HTTP_POST, handleAction);

  server.onNotFound([](){
    if(!handleFile(server.uri())) server.send(404,"text/plain","Not Found");
  });

  server.begin();
}

void loop(){
  server.handleClient();
}