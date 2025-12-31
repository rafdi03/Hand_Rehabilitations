# ESP32 Smart Robotic Hand / Rehab Glove Controller 🤖🧤

This repository contains firmware for an **ESP32-based Robotic Hand or Rehabilitation Glove**. The system controls 5 servos using **PID logic** based on flex sensor feedback to simulate hand movements. It features automated exercise cycles and **IoT integration** via Telegram for session reporting.

## 🚀 Key Features

* **PID Control System:** Implements Proportional-Integral-Derivative (PID) algorithms to stabilize servo movements based on flex sensor readings.
* **Automated Exercise Cycle:**
    * CW Movement (Grip/Flex): 8 Seconds
    * Rest/Hold: 2 Seconds
    * CCW Movement (Release/Extend): 10 Seconds
    * Rest/Hold: 2 Seconds
* **IoT & Telemetry:** Connects to WiFi to send a summary report (Total Cycles & Duration) to a **Telegram Bot** after the session ends.
* **Safety Mechanisms:**
    * **Auto-Stop:** System automatically stops after 5 minutes (300,000 ms) of runtime.
    * **Interrupt Button:** Physical button to Start or Request Stop (finishes current cycle before stopping).
* **Multi-Servo Support:** Controls 5 independent fingers (Servos + Flex Sensors).

## 🛠 Hardware Requirements

* **Microcontroller:** ESP32 Development Board
* **Actuators:** 5x Servo Motors (e.g., MG996R, SG90)
* **Sensors:** 5x Flex Sensors
* **Input:** 1x Push Button
* **Power Supply:** External 5V/6V PSU (recommended for Servos)

## 🔌 Pin Configuration
![WhatsApp Image 2025-12-31 at 12 13 38](https://github.com/user-attachments/assets/e229dbd4-793b-4714-8d20-b602b24c0446)

| Component | ESP32 Pin |
| :--- | :--- |
| **Servo 1 - 5** | GPIO 13, 12, 14, 27, 26 |
| **Flex Sensor 1 - 5** | GPIO 34, 35, 32, 33, 25 |
| **Push Button** | GPIO 4 (Input Pullup) |

## 📦 Libraries Used

Ensure you have installed the following libraries in your Arduino IDE:
1.  **ESP32Servo** (by Kevin Harrington)
2.  **UniversalTelegramBot** (by Brian Lough)
3.  **WiFiClientSecure** (Built-in ESP32)

## ⚙️ Configuration

Before uploading, modify the following lines in `main.cpp` (or your sketch file) with your credentials:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
