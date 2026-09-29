#include <cstdint>
#include <Wire.h>

#include "ST7036.hpp"

// PIN NUMBERS
// ===========================

// I2C
constexpr uint8_t PIN_SDA = 36;
constexpr uint8_t PIN_SCL = 35;

// I2C ADDRESSES
// ===========================
constexpr uint8_t I2C_USBPD = 0x08;
constexpr uint8_t I2C_LCD = 0x3C;


// DEVICES
// ===========================
ST7036 lcd(Wire, I2C_LCD);

// MAIN PROGRAM LOOP
// ===========================
void setup() {
  // Start serial
  Serial.begin(115200);
  while (!Serial);

  // Start I2C
  Wire.setPins(PIN_SDA, PIN_SCL);
  Wire.setClock(100'000);
  Wire.begin();

  // Start LCD
  lcd.begin();
  lcd.print("Hello, world!");
}

void loop() {

}
