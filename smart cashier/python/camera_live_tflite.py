# from itertools import product
import cv2
import numpy as np
import tensorflow as tf
import requests
import time
from collections import deque

# ====== إعداداتك ======
MODEL_PATH = "model_unquant.tflite"     # أو model_unquant.tflite
LABELS_PATH = "labels.txt"      # أو labels.txt
ESP32_IP = "192.168.4.1"            # <-- حط IP حق ESP32 هنا

CONF_THRESHOLD = 0.85                 # حد الثقة للإرسال
STABLE_FRAMES = 4                     # لازم نفس المنتج يظهر 4 فريمات متتالية
COOLDOWN_SEC = 2.0                    # منع تكرار الإرسال بسرعة
CAM_INDEX = 0                         # غيّرها 1 إذا الكاميرا الخارجية هي رقم 1
can_send = True
# =====================

def load_labels(path):
    with open(path, "r", encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]


def notify_esp32(product_name: str):
    try:
            reply1_from_esp32 = requests.get(f"http://{ESP32_IP}/post", params={"product": product_name}, timeout=5)
            reply_from_esp32 = reply1_from_esp32.text.strip().lower()
            print("Sent:", product_name)
            print("ESP32 replied:", reply_from_esp32)
            return reply_from_esp32
    except requests.RequestException:
        pass


# Interpreter
interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

h = int(input_details[0]["shape"][1])
w = int(input_details[0]["shape"][2])
out_dim = int(output_details[0]["shape"][1])

class_names = load_labels(LABELS_PATH)

print("Input size:", (h, w))
print("Model classes:", out_dim)
print("Labels lines:", len(class_names))

cap = cv2.VideoCapture(CAM_INDEX, cv2.CAP_DSHOW)
if not cap.isOpened():
    raise SystemExit("❌ لم يتم فتح الكاميرا. جرّب تغيير CAM_INDEX إلى 1 أو 2")

history = deque(maxlen=STABLE_FRAMES)
last_sent_label = None
last_sent_time = 0.0

reply_from_esp32 = ""
while True:
    ret, frame = cap.read()
    if not ret:
        break

    # نفس preprocessing حق Teachable Machine (مثل كودك القديم)
    img = cv2.resize(frame, (w, h))
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    x = img.astype(np.float32)
    x = (x / 127.5) - 1.0
    x = np.expand_dims(x, axis=0)

    interpreter.set_tensor(input_details[0]["index"], x)
    interpreter.invoke()

    preds = interpreter.get_tensor(output_details[0]["index"])[0]

    idx = int(np.argmax(preds))
    conf = float(preds[idx])
    label = class_names[idx] if idx < len(class_names) else f"class_{idx}"

    # عرض على الشاشة
    text = f"{label}: {conf*100:.1f}%"
    color = (0, 255, 0) if conf >= CONF_THRESHOLD else (0, 0, 255)
    cv2.putText(frame, text, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)

    # منطق الثبات + cooldown قبل الإرسال
    if conf >= CONF_THRESHOLD:
        history.append(label)
    else:
        history.clear()

    stable = (len(history) == STABLE_FRAMES) and all(x == history[0] for x in history)

    now = time.time()
    if stable:
        stable_label = history[0]
        if (stable_label != last_sent_label) or ((now - last_sent_time) > COOLDOWN_SEC):
            if (stable_label != "other" and stable_label != "non"):
                if can_send:
                    can_send = False
                    last_sent_label = stable_label
                    last_sent_time = now
                    reply_from_esp32 = notify_esp32(stable_label)

                if reply_from_esp32 in ("ok"):
                    can_send = True


    cv2.imshow("Smart Cashier - Press Q to quit", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
