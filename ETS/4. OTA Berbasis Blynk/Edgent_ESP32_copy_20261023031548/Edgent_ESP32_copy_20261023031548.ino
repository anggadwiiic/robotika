/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID           "TMPL6Zo-Nttax"
#define BLYNK_TEMPLATE_NAME         "iMCLab OTA"

#define BLYNK_FIRMWARE_VERSION        "0.1.1"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

#define BLYNK_WIFI_SSID             "Infinix NOTE 12"
#define BLYNK_WIFI_PASS             "Fird24067"
// Uncomment your board, or configure a custom board in Settings.h
//#define USE_ESP32_DEV_MODULE
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI

#include "BlynkEdgent.h"

// ==========================================
// KODE DARI DOSEN (MOTOR & SENSOR)
// ==========================================
int ledPin = 2;
int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

const byte pin_rpm = 13;
const int HOLES = 2;
volatile unsigned int rev = 0;
volatile unsigned long lastPulseTime = 0;
const unsigned long DEBOUNCE_US = 500;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;
const int DUTY_STEP = 10;
const int DUTY_MIN = 0;
const int DUTY_MAX = 255;
bool motorEnable = false;

float rpm = 0.0;
unsigned long old_time = 0;

BlynkTimer timerMotor; // Ganti nama jadi timerMotor

void IRAM_ATTR isr() {
  unsigned long now = micros();
  if (now - lastPulseTime > DEBOUNCE_US) {
    rev++;
    lastPulseTime = now;
  }
}

void hitungRPM() {
  unsigned long cur_time = millis();
  if (cur_time - old_time >= 1000) {
    detachInterrupt(digitalPinToInterrupt(pin_rpm));
    float rot_per_sec = (float)rev / (float)HOLES;
    rpm = rot_per_sec * 60.0;
    rev = 0;
    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
    old_time = millis();
  }
}

void applyMotor() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(pwmChannel, dutyCycle);
  digitalWrite(ledPin, dutyCycle > 0);
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, dutyCycle > 0);
}

void kirimKeBlynk() {
  Blynk.virtualWrite(V0, rpm);
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, dutyCycle > 0);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}

BLYNK_WRITE(V2) {
  dutyCycle = constrain(param.asInt(), DUTY_MIN, DUTY_MAX);
  motorEnable = (dutyCycle > 0);
  applyMotor();
  Serial.print("PWM dari Blynk = ");
  Serial.println(dutyCycle);
}

BLYNK_WRITE(V3) {
  motorEnable = param.asInt();
  if (motorEnable) {
    if (dutyCycle == 0) dutyCycle = 200;
  } else {
    dutyCycle = 0;
  }
  applyMotor();
  Serial.print("Motor ");
  Serial.println(motorEnable ? "ON" : "OFF");
}
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(pin_rpm, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(pwmChannel, 0);
  old_time = millis();

  timerMotor.setInterval(1000L, kirimKeBlynk);

  BlynkEdgent.begin();
}

void loop() {
  BlynkEdgent.run();
  timerMotor.run();
  hitungRPM();
}

