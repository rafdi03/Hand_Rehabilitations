#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// === Servo ===
Servo servo1, servo2, servo3, servo4, servo5; 

#define SERVO1_PIN 13
#define SERVO2_PIN 12
#define SERVO3_PIN 14
#define SERVO4_PIN 27
#define SERVO5_PIN 26

// === Flex Sensor ===
#define FLEX1_PIN 34
#define FLEX2_PIN 35
#define FLEX3_PIN 32
#define FLEX4_PIN 33
#define FLEX5_PIN 25

// === Push Button ===
#define BUTTON_PIN 4
volatile bool buttonInterrupt = false;
volatile unsigned long lastInterrupt = 0;
bool systemRunning = false, stopping = false;

// === Variabel Waktu & Siklus ===
unsigned long stateStart = 0;
unsigned long lastUpdate = 0;
unsigned long startTime = 0;
int state = 0;
unsigned long cycleCount = 0;

// === PID ===
float Kp = 0.4, Ki = 1, Kd = 0.1;
float lastErr[5] = {0}, integral[5] = {0};

// === Pulse Servo ===
int MID = 1500;
int CW_PULSE = 2000;
int CCW_PULSE = 1000;

// === Durasi Maksimal ===
unsigned long MAX_RUNTIME = 300000; // 5 menit

// === WiFi & Telegram ===
const char* ssid = "";
const char* password = "";
#define BOT_TOKEN ""
#define CHAT_ID ""

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ===== Format Waktu =====
String formatTime(unsigned long ms) {
  unsigned long sec = ms / 1000;
  unsigned long mm = sec / 60;
  unsigned long ss = sec % 60;
  char buff[10];
  sprintf(buff, "%02lu:%02lu", mm, ss);
  return String(buff);
}

// ===== PID =====
float pidCalc(int id, int v) {
  float target = 2000;
  float e = target - v;
  integral[id] += e;
  float d = e - lastErr[id];
  lastErr[id] = e;
  float out = Kp * e + Ki * integral[id] + Kd * d;
  return constrain(out, -200, 200);
}

void resetPID() {
  for (int i = 0; i < 5; i++) {
    lastErr[i] = 0;
    integral[i] = 0;
  }
}

// ===== Interrupt Tombol =====
void IRAM_ATTR isrBtn() {
  unsigned long m = millis();
  if (m - lastInterrupt > 250) {
    buttonInterrupt = true;
    lastInterrupt = m;
  }
}

// ===== Kendali Servo =====
void setPulse(int pulse) {
  servo1.writeMicroseconds(pulse);
  servo2.writeMicroseconds(pulse);
  servo3.writeMicroseconds(pulse);
  servo4.writeMicroseconds(pulse);
  servo5.writeMicroseconds(pulse);
}

// ===== Update Servo + Cetak Data =====
void updateServo(int basePulse, float pid) {
  int pulse = basePulse + pid;
  pulse = constrain(pulse, 800, 2200);
  setPulse(pulse);

  int f1 = analogRead(FLEX1_PIN);
  int f2 = analogRead(FLEX2_PIN);
  int f3 = analogRead(FLEX3_PIN);
  int f4 = analogRead(FLEX4_PIN);
  int f5 = analogRead(FLEX1_PIN) - 232;

  Serial.printf("Pulse: %d | Cycle: %lu | Flex: %d %d %d %d %d | Time: %s\n",
                pulse, cycleCount, f1, f2, f3, f4, f5,
                formatTime(millis() - startTime).c_str());
}

// ===== Kirim Pesan ke Telegram =====
void sendToTelegram(String message) {
  Serial.println("⏳ Menghubungkan ke WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("✅ WiFi connected: %s\n", WiFi.localIP().toString().c_str());
    secured_client.setInsecure(); // lewati verifikasi SSL (perlu di ESP32)
    delay(1000); // beri waktu biar koneksi stabil
    bool sent = bot.sendMessage(CHAT_ID, message, "");
    if (sent) Serial.println("📨 Telegram terkirim!");
    else Serial.println("❌ Gagal kirim ke Telegram!");
    WiFi.disconnect(true);
  } else {
    Serial.println("❌ WiFi gagal konek, tidak bisa kirim Telegram");
  }
}

// ===== STOP SISTEM =====
void stopSys() {
  systemRunning = false;
  stopping = false;
  setPulse(MID);
  unsigned long runTime = millis() - startTime;

  Serial.println("✅ STOP Selesai");
  Serial.printf("Durasi: %s | Total Siklus: %lu\n",
                formatTime(runTime).c_str(), cycleCount);

  String report = "🦾 Laporan Selesai\nDurasi: " + formatTime(runTime) +
                  "\nTotal Siklus: " + String(cycleCount);
  sendToTelegram(report);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isrBtn, FALLING);

  servo1.attach(SERVO1_PIN, 500, 2500);
  servo2.attach(SERVO2_PIN, 500, 2500);
  servo3.attach(SERVO3_PIN, 500, 2500);
  servo4.attach(SERVO4_PIN, 500, 2500);
  servo5.attach(SERVO5_PIN, 500, 2500);
  setPulse(MID);

  Serial.println("🔥 Device ON - Tekan tombol untuk START");
}

// ===== LOOP =====
void loop() {
  unsigned long now = millis();

  if (buttonInterrupt) {
    buttonInterrupt = false;

    if (stopping) {
      Serial.println("Sudah minta stop...");
      return;
    }

    if (!systemRunning) {
      systemRunning = true;
      stopping = false;
      cycleCount = 0;
      state = 1;
      stateStart = now;
      startTime = now;
      resetPID();
      Serial.println("🚀 Sistem mulai");
    } else {
      stopping = true;
      Serial.println("🛑 STOP diterima — menunggu siklus selesai");
    }
  }

  if (!systemRunning) return;

  if (now - startTime >= MAX_RUNTIME) {
    Serial.println("⏳ 5 menit selesai — sistem berhenti otomatis");
    stopSys();
    return;
  }

  if (now - lastUpdate < 150) return;
  lastUpdate = now;

  float pid = (
    pidCalc(0, analogRead(FLEX1_PIN)) +
    pidCalc(1, analogRead(FLEX2_PIN)) +
    pidCalc(2, analogRead(FLEX3_PIN)) +
    pidCalc(3, analogRead(FLEX4_PIN)) +
    pidCalc(4, analogRead(FLEX5_PIN))
  ) / 5;

  unsigned long elapsed = now - stateStart;

  switch (state) {
    case 1:
      updateServo(CW_PULSE, pid);
      if (elapsed >= 8000) {
        state = 2;
        stateStart = now;
        setPulse(MID);
      }
      break;

    case 2:
      updateServo(MID, 0);
      if (elapsed >= 2000) {
        state = 3;
        stateStart = now;
      }
      break;

    case 3:
      updateServo(CCW_PULSE, pid);
      if (elapsed >= 10000) {
        state = 4;
        stateStart = now;
        setPulse(MID);
      }
      break;

    case 4:
      updateServo(MID, 0);
      if (elapsed >= 2000) {
        cycleCount++;
        Serial.printf("✅ Cycle %lu selesai\n", cycleCount);
        resetPID();
        if (stopping) { stopSys(); return; }
        state = 1;
        stateStart = now;
      }
      break;
  }
}
