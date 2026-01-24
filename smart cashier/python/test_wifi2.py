import requests

ESP32_IP = "192.168.4.1"

r = requests.get(f"http://{ESP32_IP}/get", timeout=3)
print("Received:", r.text)
# print(r)
