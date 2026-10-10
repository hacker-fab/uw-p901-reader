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

// MAIN PROGRAM LOOP
// ===========================

void setup() {

  // PD init
  setupBuses();
  setupLCD();
  
  if (!setupPD()) {
    lcd.print("PD setup failed");
    while (true) {}
  }
}

void loop() {
  
}
