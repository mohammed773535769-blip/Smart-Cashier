[text](model/README.md)# 🛒 Smart Cashier System

Smart Cashier System is a mechatronics project that aims to reduce congestion at traditional checkout counters by using computer vision and machine learning to automatically recognize products, supported by weight verification and automated conveyor control.

---

## 🎯 Problem Statement
- Long waiting times at cashier counters
- Dependence on human cashiers and recurring salary costs
- Manual product entry errors

---

## 💡 Proposed Solution
A smart automated cashier system that:
- Recognizes products using **Machine Learning (Computer Vision)**
- Verifies products using a **weight sensor (Load Cell)**
- Controls a conveyor belt automatically
- Communicates between a PC and an **ESP32** via WiFi (HTTP)

---

## ⚙️ System Components

### Hardware
- ESP32 Microcontroller  
- Camera  
- Load Cell + HX711  
- Single-phase induction motor  
- Gearbox (10:1)  
- Relays & transistors  
- Conveyor belt  
- Mechanical frame (SolidWorks design)

### Software
- Python
- OpenCV
- TensorFlow Lite
- Arduino (ESP32)
- HTTP communication

---

## 📂 Project Structure
