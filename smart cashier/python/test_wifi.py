import requests

ESP32_IP = "192.168.4.1"
product = "sanaa"

r = requests.get(f"http://{ESP32_IP}/post", params={"product": product}, timeout=5)

print("Sent:", product)
print("ESP32 replied:", r.text)

