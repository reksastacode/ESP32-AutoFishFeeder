/*
 * TEST BRING-UP — Pakan Ikan Otomatis
 * ESP32 Dev Module + OLED SSD1306 + DS18B20 + TDS Meter V1.0
 * + VL53L0X ToF + Servo SG90 + tombol + buzzer pasif
 *
 * Target board : ESP32 Dev Module
 * ESP32 core   : 2.0.17 atau 3.x (API LEDC dipilih otomatis)
 * Serial       : 115200 baud
 *
 * Perintah Serial Monitor:
 *   h = help
 *   i = scan I2C
 *   d = baca DS18B20 sekarang
 *   v = baca VL53L0X sekarang
 *   t = baca TDS sekarang
 *   b = tes buzzer
 *   s = servo 0 -> 90 -> 0 (aman untuk mekanisme)
 *   x = servo 0 -> 180 -> 0 (LEPAS mekanisme dulu!)
 *   f = feed test: buzzer + servo 0 -> 90 -> 0
 *   m = print semua status
 *   o = refresh OLED
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <esp_arduino_version.h>
#include <math.h>
#include <ctype.h>

// ---------------- PIN MAP ----------------
const uint8_t PIN_SDA       = 21;
const uint8_t PIN_SCL       = 22;
const uint8_t PIN_DS18B20   = 4;
const uint8_t PIN_TDS       = 34;   // input-only, ADC1
const uint8_t PIN_SERVO     = 18;
const uint8_t PIN_BUZZER    = 25;
const uint8_t PIN_BUTTON    = 27;

// ---------------- OLED ----------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oledReady = false;
uint8_t oledAddress = 0x3C;

// ---------------- VL53L0X ----------------
Adafruit_VL53L0X vl53l0x;

bool tofReady = false;
bool tofValid = false;
uint16_t lastDistanceMm = 0;
uint8_t lastRangeStatus = 255;
unsigned long lastTofReadMs = 0;
const unsigned long TOF_READ_MS = 200;

// ---------------- DS18B20 ----------------
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);

bool dsReady = false;
float lastTempC = NAN;
unsigned long lastTempRequestMs = 0;
const unsigned long TEMP_CONVERSION_MS = 750;

// ---------------- TDS ----------------
float lastTdsPpm = NAN;
uint32_t lastTdsMv = 0;

// ---------------- SERVO ----------------
Servo servo;
int servoChannel = -1;
const int SERVO_MIN_US   = 544;   // aman untuk SG90
const int SERVO_MAX_US   = 2400;
const int SERVO_HOME_DEG = 0;     // ubah sesuai posisi rail
const int SERVO_FEED_DEG = 90;    // ubah sesuai langkah rail

// ---------------- BUZZER ----------------
const uint8_t  BUZZER_LEDC_CHANNEL = 15;
const uint16_t BUZZER_FREQ         = 2000;

// ---------------- BUTTON ----------------
const unsigned long DEBOUNCE_MS   = 50;
const unsigned long LONG_PRESS_MS = 800;

bool lastButtonReading = HIGH;
bool buttonStable      = HIGH;
unsigned long lastButtonChangeMs = 0;
unsigned long buttonPressStartMs  = 0;
bool longPressHandled = false;

// ---------------- TIMING ----------------
unsigned long lastTdsReadMs = 0;
unsigned long lastDisplayMs = 0;

// ---------------- FORWARD DECLARATIONS ----------------
void scanI2C();
void initOLED();
void updateDisplay();
void initTOF();
void updateTOF();
void readTOFNow();
const char* tofStatusText(uint8_t status);
void updateDS18B20();
void readDS18B20Now();
void updateTDS();
void updateButton();
void buzzerInit();
void buzzerDeinit();
void beep(uint8_t count, uint16_t durationMs);
void runBuzzerTest();
bool attachServo();
void printServoFreq();
void runServoSafe();
void runServoFull();
void runFeedSequence();
void printAll();
void printHelp();
void handleCommand(char c);

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==========================================");
  Serial.println("  TEST BRING-UP PAKAN IKAN OTOMATIS");
  Serial.println("  ESP32 + OLED + DS18B20 + TDS + VL53L0X");
  Serial.println("  Servo SG90 + tombol + buzzer");
  Serial.println("==========================================");
  Serial.println();

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  // I2C
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000); // 100 kHz, aman untuk breadboard
  scanI2C();

  // OLED
  initOLED();

  // VL53L0X
  initTOF();

  // DS18B20
  ds18b20.begin();
  ds18b20.setResolution(12);
  ds18b20.setWaitForConversion(false);

  int dsCount = ds18b20.getDeviceCount();
  Serial.printf("DS18B20: %d device terdeteksi.\n", dsCount);
  dsReady = (dsCount > 0);

  if (dsReady) {
    ds18b20.requestTemperatures();
    lastTempRequestMs = millis();
  } else {
    Serial.println("DS18B20 TIDAK terdeteksi. Cek wiring dan pull-up.");
  }

  // TDS
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_TDS, ADC_11db);
  Serial.println("TDS: GPIO34, ADC 12-bit, attenuasi 11 dB.");
  Serial.println("TDS VCC disarankan 3.3 V untuk tes awal.");

  // Servo
  // Sengaja tidak di-attach saat boot supaya servo tidak bergerak sendiri.
  // Servo baru bergerak saat perintah 's', 'f', atau 'x' dikirim.
  Serial.println("Servo: belum di-attach (tekan 's' untuk tes).");

  // Buzzer
  buzzerInit();

  Serial.println();
  printHelp();

  beep(2, 120);
  updateTOF();
  updateDisplay();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  updateButton();
  updateDS18B20();
  updateTOF();

  if (millis() - lastTdsReadMs >= 2000) {
    updateTDS();
    lastTdsReadMs = millis();
  }

  if (millis() - lastDisplayMs >= 500) {
    updateDisplay();
    lastDisplayMs = millis();
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') continue;
    handleCommand(c);
  }

  delay(5);
}

// ============================================================
// I2C
// ============================================================
void scanI2C() {
  Serial.println("Scan I2C...");
  uint8_t found = 0;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Ditemukan 0x%02X\n", addr);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("  Tidak ada perangkat I2C.");
  } else {
    Serial.printf("  Total: %u perangkat.\n", (unsigned)found);
  }
}

// ============================================================
// OLED
// ============================================================
void initOLED() {
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAddress = 0x3C;
    oledReady = true;
  } else if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    oledAddress = 0x3D;
    oledReady = true;
  } else {
    oledReady = false;
  }

  if (!oledReady) {
    Serial.println("OLED TIDAK terdeteksi di 0x3C/0x3D.");
    return;
  }

  Serial.printf("OLED: OK di 0x%02X.\n", oledAddress);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEST BRING-UP");
  display.println("OLED OK");
  display.display();
  delay(800);
}

void updateDisplay() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("BRING-UP 0x");
  display.println(oledAddress, HEX);

  display.print("Suhu  : ");
  if (isnan(lastTempC)) {
    display.println("--");
  } else {
    display.print(lastTempC, 1);
    display.println(" C");
  }

  display.print("TDS   : ");
  if (isnan(lastTdsPpm)) {
    display.println("--");
  } else {
    display.print(lastTdsPpm, 0);
    display.println(" ppm");
  }

  display.print("TDS mV: ");
  display.println((unsigned long)lastTdsMv);

  display.print("ToF   : ");
  if (!tofReady) {
    display.println("N/A");
  } else if (!tofValid) {
    display.println(lastRangeStatus == 4 ? "OUT" : "ERR");
  } else {
    display.print(lastDistanceMm / 10.0, 1);
    display.println(" cm");
  }

  display.print("Tombol: ");
  display.println(buttonStable == LOW ? "TEKAN" : "LEPAS");

  display.print("Servo : ");
  display.println("IDLE");

  display.display();
}

// ============================================================
// DS18B20
// ============================================================
void updateDS18B20() {
  if (!dsReady) return;
  if (millis() - lastTempRequestMs < TEMP_CONVERSION_MS) return;

  float t = ds18b20.getTempCByIndex(0);

  if (t == DEVICE_DISCONNECTED_C || t < -50.0 || t > 100.0) {
    lastTempC = NAN;
    Serial.println("DS18B20: pembacaan tidak valid / terputus.");
  } else {
    lastTempC = t;
  }

  ds18b20.requestTemperatures();
  lastTempRequestMs = millis();
}

void readDS18B20Now() {
  if (!dsReady) {
    Serial.println("DS18B20 tidak siap.");
    return;
  }

  ds18b20.setWaitForConversion(true);
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  ds18b20.setWaitForConversion(false);

  if (t == DEVICE_DISCONNECTED_C || t < -50.0 || t > 100.0) {
    Serial.println("DS18B20: pembacaan tidak valid / terputus.");
    lastTempC = NAN;
  } else {
    lastTempC = t;
    Serial.printf("DS18B20: %.2f C\n", t);
  }

  // Lanjutkan konversi background supaya pembacaan otomatis tetap jalan.
  ds18b20.requestTemperatures();
  lastTempRequestMs = millis();
}

// ============================================================
// VL53L0X
// ============================================================
void initTOF() {
  Serial.println("VL53L0X: init di 0x29...");

  if (!vl53l0x.begin()) {
    tofReady = false;
    Serial.println("VL53L0X TIDAK terdeteksi. Cek 3.3V, GND, SDA/SCL, dan XSHUT.");
    return;
  }

  tofReady = true;
  Serial.println("VL53L0X: OK di 0x29.");
}

const char* tofStatusText(uint8_t status) {
  switch (status) {
    case 0:   return "valid";
    case 1:   return "sigma fail";
    case 2:   return "signal fail";
    case 3:   return "min range";
    case 4:   return "out of range";
    case 5:   return "hardware fail";
    case 255: return "I2C error";
    default:  return "unknown";
  }
}

bool readTofMeasurement(uint16_t &distanceMm, uint8_t &rangeStatus) {
  if (!tofReady) return false;

  VL53L0X_RangingMeasurementData_t measure;
  VL53L0X_Error err = vl53l0x.rangingTest(&measure, false);

  if (err != VL53L0X_ERROR_NONE) {
    rangeStatus = 255;
    return false;
  }

  rangeStatus = measure.RangeStatus;
  if (rangeStatus != 0) return false;

  distanceMm = measure.RangeMilliMeter;
  return true;
}

void updateTOF() {
  if (!tofReady) return;
  if (millis() - lastTofReadMs < TOF_READ_MS) return;

  lastTofReadMs = millis();

  uint16_t distance = 0;
  uint8_t status = 255;
  bool valid = readTofMeasurement(distance, status);

  lastRangeStatus = status;

  if (valid) {
    lastDistanceMm = distance;
    tofValid = true;
  } else {
    tofValid = false;
  }
}

void readTOFNow() {
  if (!tofReady) {
    Serial.println("VL53L0X tidak siap. Cek wiring dan alamat 0x29.");
    return;
  }

  uint16_t distance = 0;
  uint8_t status = 255;
  bool valid = readTofMeasurement(distance, status);

  lastRangeStatus = status;
  lastTofReadMs = millis();

  if (valid) {
    lastDistanceMm = distance;
    tofValid = true;
    Serial.printf("ToF: %u mm (%.1f cm), status=%u (%s)\n",
                  (unsigned)distance, distance / 10.0,
                  status, tofStatusText(status));
  } else {
    tofValid = false;
    Serial.printf("ToF: tidak valid, status=%u (%s)\n",
                  status, tofStatusText(status));
  }
}

// ============================================================
// TDS
// ============================================================
void updateTDS() {
  const int N = 15;
  uint32_t samples[N];

  for (int i = 0; i < N; i++) {
    samples[i] = analogReadMilliVolts(PIN_TDS);
    delay(2);
  }

  // Median sederhana
  for (int i = 1; i < N; i++) {
    uint32_t key = samples[i];
    int j = i - 1;
    while (j >= 0 && samples[j] > key) {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = key;
  }

  lastTdsMv = samples[N / 2];

  float voltage = lastTdsMv / 1000.0;
  float tempC = isnan(lastTempC) ? 25.0 : lastTempC;

  // Kompensasi suhu sederhana (lihat catatan DFRobot)
  float compensationCoefficient = 1.0 + 0.02 * (tempC - 25.0);
  float compensationVoltage = voltage / compensationCoefficient;

  float tds = (133.42 * pow(compensationVoltage, 3)
             - 255.86 * pow(compensationVoltage, 2)
             + 857.39 * compensationVoltage) * 0.5;

  if (tds < 0) tds = 0;
  if (tds > 5000) tds = 5000; // jaga-jaga kalau wiring/tegangan aneh

  lastTdsPpm = tds;

  Serial.printf("TDS: %lu mV | V=%.3f | T=%.1f C | ppm=%.0f\n",
                (unsigned long)lastTdsMv, voltage, tempC, tds);
}

// ============================================================
// BUTTON
// ============================================================
void updateButton() {
  bool reading = digitalRead(PIN_BUTTON);

  if (reading != lastButtonReading) {
    lastButtonChangeMs = millis();
    lastButtonReading = reading;
  }

  if (millis() - lastButtonChangeMs > DEBOUNCE_MS) {
    if (reading != buttonStable) {
      buttonStable = reading;

      if (buttonStable == LOW) {
        buttonPressStartMs = millis();
        longPressHandled = false;
        Serial.println("Tombol: DITEKAN");
      } else {
        Serial.println("Tombol: DILEPAS");
        if (!longPressHandled) {
          Serial.println("Tombol: short press (tidak memicu feed).");
        }
      }
    }
  }

  if (buttonStable == LOW && !longPressHandled &&
      (millis() - buttonPressStartMs >= LONG_PRESS_MS)) {
    longPressHandled = true;
    Serial.println("Tombol: LONG PRESS -> feed test.");
    runFeedSequence();
  }
}

// ============================================================
// BUZZER
// ============================================================
// ESP32 core 2.x memakai channel LEDC.
// ESP32 core 3.x memakai pin LEDC.
//
// Buzzer sengaja memakai channel 15 (group 1) yang terpisah dari
// channel 0 (group 0) yang dipakai library servo. Setelah beep,
// channel buzzer dilepas supaya tidak mengganggu timer servo.
void buzzerInit() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttachChannel(PIN_BUZZER, BUZZER_FREQ, 8, BUZZER_LEDC_CHANNEL);
  ledcWrite(PIN_BUZZER, 0);
#else
  ledcSetup(BUZZER_LEDC_CHANNEL, BUZZER_FREQ, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
  ledcWrite(BUZZER_LEDC_CHANNEL, 0);
#endif
}

void buzzerDeinit() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcDetach(PIN_BUZZER);
#else
  ledcDetachPin(PIN_BUZZER);
#endif
}

void buzzerToneOn() {
  // ledcWriteTone sudah mengatur duty 50%, baik di core 2.x maupun 3.x.
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(PIN_BUZZER, BUZZER_FREQ);
#else
  ledcWriteTone(BUZZER_LEDC_CHANNEL, BUZZER_FREQ);
#endif
}

void buzzerToneOff() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_BUZZER, 0);
#else
  ledcWrite(BUZZER_LEDC_CHANNEL, 0);
#endif
}

void beep(uint8_t count, uint16_t durationMs) {
  buzzerInit();

  for (uint8_t i = 0; i < count; i++) {
    buzzerToneOn();
    delay(durationMs);
    buzzerToneOff();

    if (i + 1 < count) delay(100);
  }

  // Penting: lepas buzzer dari LEDC agar channel/timer bebas untuk servo.
  buzzerDeinit();
}

void runBuzzerTest() {
  Serial.println("Buzzer: 3 beep 2 kHz.");
  beep(3, 200);
  Serial.println("Buzzer: selesai.");
}

// ============================================================
// SERVO
// ============================================================
bool attachServo() {
  servo.setPeriodHertz(50);
  servoChannel = servo.attach(PIN_SERVO, SERVO_MIN_US, SERVO_MAX_US);

  bool ok = servo.attached();
  Serial.printf("Servo: signal GPIO%d, attach channel=%d, attached=%s\n",
                PIN_SERVO, servoChannel, ok ? "YA" : "TIDAK");

  return ok;
}

void printServoFreq() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  Serial.printf("Servo: PWM freq=%lu Hz (harus 50 Hz)\n",
                (unsigned long)ledcReadFreq(PIN_SERVO));
#else
  if (servoChannel >= 0) {
    Serial.printf("Servo: PWM freq=%lu Hz (harus 50 Hz)\n",
                  (unsigned long)ledcReadFreq(servoChannel));
  }
#endif
}

void runServoSafe() {
  Serial.println("Servo: 0 -> 90 -> 0 (safe sweep).");

  if (!attachServo()) {
    Serial.println("Servo: GAGAL attach. Cek kabel signal ke GPIO18.");
    return;
  }

  servo.write(SERVO_HOME_DEG);
  delay(400);
  printServoFreq();

  for (int angle = SERVO_HOME_DEG; angle <= SERVO_FEED_DEG; angle += 5) {
    servo.write(angle);
    delay(30);
  }

  delay(500);

  for (int angle = SERVO_FEED_DEG; angle >= SERVO_HOME_DEG; angle -= 5) {
    servo.write(angle);
    delay(30);
  }

  delay(300);
  servo.detach();
  servoChannel = -1;
  Serial.println("Servo: safe sweep selesai.");
}

void runServoFull() {
  Serial.println("Servo: 0 -> 180 -> 0. Pastikan mekanisme TIDAK terpasang!");

  if (!attachServo()) {
    Serial.println("Servo: GAGAL attach. Cek kabel signal ke GPIO18.");
    return;
  }

  servo.write(0);
  delay(400);
  printServoFreq();

  for (int angle = 0; angle <= 180; angle += 5) {
    servo.write(angle);
    delay(30);
  }

  delay(500);

  for (int angle = 180; angle >= 0; angle -= 5) {
    servo.write(angle);
    delay(30);
  }

  delay(300);
  servo.detach();
  servoChannel = -1;
  Serial.println("Servo: full sweep selesai.");
}

void runFeedSequence() {
  Serial.println("Feed test: buzzer peringatan...");
  beep(3, 300);
  delay(900); // total peringatan ±2 detik

  Serial.println("Feed test: servo bergerak.");

  if (!attachServo()) {
    Serial.println("Feed test: servo GAGAL attach, feed dibatalkan.");
    return;
  }

  servo.write(SERVO_HOME_DEG);
  delay(400);
  printServoFreq();
  servo.write(SERVO_FEED_DEG);
  delay(800);
  servo.write(SERVO_HOME_DEG);
  delay(500);
  servo.detach();
  servoChannel = -1;

  Serial.println("Feed test: selesai.");
}

// ============================================================
// STATUS & COMMANDS
// ============================================================
void printAll() {
  Serial.println("---------- STATUS ----------");

  if (isnan(lastTempC)) {
    Serial.println("Suhu   : --");
  } else {
    Serial.printf("Suhu   : %.2f C\n", lastTempC);
  }

  if (isnan(lastTdsPpm)) {
    Serial.println("TDS    : --");
  } else {
    Serial.printf("TDS    : %.0f ppm (%lu mV)\n",
                  lastTdsPpm, (unsigned long)lastTdsMv);
  }

  if (!tofReady) {
    Serial.println("ToF    : TIDAK ADA");
  } else if (!tofValid) {
    Serial.printf("ToF    : tidak valid (status %u: %s)\n",
                  lastRangeStatus, tofStatusText(lastRangeStatus));
  } else {
    Serial.printf("ToF    : %u mm (%.1f cm)\n",
                  (unsigned)lastDistanceMm, lastDistanceMm / 10.0);
  }

  Serial.printf("Tombol : %s\n", buttonStable == LOW ? "TEKAN" : "LEPAS");
  Serial.printf("OLED   : %s (0x%02X)\n", oledReady ? "OK" : "TIDAK ADA", oledAddress);
  Serial.printf("DS18B20: %s\n", dsReady ? "OK" : "TIDAK ADA");
  Serial.println("----------------------------");
}

void printHelp() {
  Serial.println("Perintah Serial:");
  Serial.println("  h = help");
  Serial.println("  i = scan I2C");
  Serial.println("  d = baca DS18B20 sekarang");
  Serial.println("  v = baca VL53L0X sekarang");
  Serial.println("  t = baca TDS sekarang");
  Serial.println("  b = tes buzzer");
  Serial.println("  s = servo 0 -> 90 -> 0 (aman)");
  Serial.println("  x = servo 0 -> 180 -> 0 (lepas mekanisme dulu)");
  Serial.println("  f = feed test: buzzer + servo 0 -> 90 -> 0");
  Serial.println("  m = print semua status");
  Serial.println("  o = refresh OLED");
  Serial.println();
}

void handleCommand(char c) {
  switch (toupper(c)) {
    case 'H':
      printHelp();
      break;

    case 'I':
      scanI2C();
      break;

    case 'D':
      readDS18B20Now();
      break;

    case 'V':
      readTOFNow();
      break;

    case 'T':
      updateTDS();
      break;

    case 'B':
      runBuzzerTest();
      break;

    case 'S':
      runServoSafe();
      break;

    case 'X':
      runServoFull();
      break;

    case 'F':
      runFeedSequence();
      break;

    case 'M':
      printAll();
      break;

    case 'O':
      updateDisplay();
      Serial.println("OLED: refresh.");
      break;

    default:
      Serial.printf("Perintah '%c' tidak dikenal. Ketik 'h' untuk help.\n", c);
      break;
  }
}
