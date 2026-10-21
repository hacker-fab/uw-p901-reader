// #define NDEBUG

#include <cstdint>
#include <Arduino.h>
#include <Wire.h>

#include "WireExt.hpp"
#include "HUSB238.hpp"
#include "MKS_901P.hpp"
#include "ST7036.hpp"

// PIN NUMBERS
// ===========================

// I2C
constexpr uint8_t PIN_SDA = 36;
constexpr uint8_t PIN_SCL = 35;

// Serial
constexpr uint8_t PIN_TX1 = 39;
constexpr uint8_t PIN_RX1 = 40;

// I2C ADDRESSES
// ===========================
constexpr uint8_t I2C_PD = 0x08;
constexpr uint8_t I2C_LCD = 0x3C;

// DEVICES
// ===========================
ST7036 lcd {Wire, I2C_LCD};
HUSB238 pdo {Wire, I2C_PD};
MKS_901P sensor {Serial1};

// SETUP FUNCTIONS
// ===========================

void setupBuses() {
  // Start CDC serial
  Serial.begin(115200);
  while (!Serial && millis() < 500) {}

  // Start UART to sensor
  Serial1.begin(9600, SERIAL_8N1, PIN_RX1, PIN_TX1);

  // Start I2C
  Wire.setPins(PIN_SDA, PIN_SCL);
  Wire.setClock(100'000);
  Wire.setBufferSize(256);
  Wire.begin();
}

void setupLCD() {
  // Start LCD
  if (!wirePing(I2C_LCD))
    abort();
  lcd.begin();
}

bool setupPD() {
  auto [volts, amps] = pdo.currentProfile();
  // if we're already above 5V then everything's bougie
  if (volts > HUSB238::V5) {
    Serial.printf("profile: %u %u\n", volts, amps);
    return true;
  }

  if (pdo.checkVoltage(HUSB238::V12) >= 0) {
    Serial.println("requesting 12 V");
    pdo.requestVoltage(HUSB238::V12);
    auto [newVolts, newAmps] = pdo.currentProfile();
    return newVolts == HUSB238::V12;
  }

  if (pdo.checkVoltage(HUSB238::V9) >= 0) {
    Serial.println("requesting 9 V");
    pdo.requestVoltage(HUSB238::V9);
    auto [newVolts, newAmps] = pdo.currentProfile();
    return newVolts == HUSB238::V9;
  }

  return false;
}

bool setupSensor() {
  if (!sensor.begin())
    return false;

  // sensor.rawCommand("RSD", "ON");
  sensor.writeUnits(MKS_901P::UnitsTorr);
  sensor.writeGasType(MKS_901P::GasAir);
  return true;
}

void setup() {
  setupBuses();

  setupLCD();
  Serial.println("LCD done");

  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
  if (!setupPD())
    Serial.println("PDO error!");
  Serial.println("PDO done");

  if (!setupSensor())
    Serial.println("901P error!");
  Serial.println("901P done");
}

// MAIN PROGRAM LOOP
// ===========================

void displayPressureScaled(int mantissa, int exponent, const char* trailing) {
  assert(0 <= exponent && exponent < 3);

  constexpr int pow10[] = {1, 10, 100, 1000, 10'000};

  int fDigits = 3 - exponent;
  int fFactor = pow10[fDigits];

  uint iPart = mantissa / fFactor;
  uint fPart = mantissa % fFactor;

  lcd.setCursor(0, 0);
  size_t printLen = lcd.printf("%0*u.%0*u %s", 4 - fDigits, iPart, fDigits, fPart, trailing);
  lcd.printf("%*s", int(20 - printLen), "");
}
void displayPressureScientific(int mantissa, int exponent, const char* trailing) {
  int iPart = mantissa / 1000;
  int fPart = mantissa % 1000;

  lcd.setCursor(0, 0);
  size_t printLen = lcd.printf("%01u.%03ue%d %s", iPart, fPart, exponent, trailing);
  lcd.printf("%*s", int(20 - printLen), "");
}

void displayPressure(int mantissa, int exponent) {
  if (exponent < -3 || exponent >= 3)
    displayPressureScientific(mantissa, exponent, "torr");
  else if (exponent < 0)
    displayPressureScaled(mantissa, exponent + 3, "mtorr");
  else if (exponent < 3)
    displayPressureScaled(mantissa, exponent, "torr");
}

void tryReadPressure() {
  std::pair<int, int> pressure = {};
  if (auto res = sensor.readPR4(pressure); res != MKS_901P::ErrNone) {
    return;
  }

  auto [mantissa, exponent] = pressure;
  displayPressure(mantissa, exponent);
}

int sendBegin = 0;

void serialPassthru() {
  if (Serial.available()) {
    int c = Serial.read();
    if (c == '@') sendBegin = micros();
    Serial1.write(c);
  }
  if (Serial1.available()) {
    int c = Serial1.read();
    if (c == '@') {
      int time = micros() - sendBegin;
      Serial.printf("[delay %d]", time);
    }
    Serial.write(c);
    
  }
}

void loop() {
  // Serial1.write("@253GT!AIR;FF");
  // while (Serial1.read() != '@') {}
  // auto s = Serial1.readStringUntil(';');
  // while (!Serial1.available()); Serial1.read();
  // while (!Serial1.available()); Serial1.read();

  // Serial.print("read: ");
  // Serial.println(s);

  // delay(500);
  tryReadPressure();
  delay(250);
}
