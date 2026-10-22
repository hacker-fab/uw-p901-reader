// #define NDEBUG

#include <cstdint>
#include <cstring>
#include <Arduino.h>
#include <Wire.h>

#include "HUSB238.hpp"
#include "MKS_901P.hpp"
#include "ST7036.hpp"

#include "CommUtils.hpp"
#include "DisplayUtils.hpp"

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
  lcd.setContrast(24); // this seems to be nice
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

  if (!setupPD())
    Serial.println("PDO error!");
  Serial.println("PDO done");

  if (!setupSensor())
    Serial.println("901P error!");
  Serial.println("901P done");
}

// MAIN PROGRAM LOOP
// ===========================

// CLI line buffer
static char inBuffer[256] = {};
static size_t inIdx = 0;
constexpr size_t inLastIndex = sizeof(inBuffer) - 1;

// pressure reading
static char lcdBuffer[41] = {};
static int msPressureLast = 0;


static const char* helpMessage[] = {
  "Available commands:\n"
  "help                  Show this help message\n"
  "set-gas <gas>         Set the sensor gas type (refer to MKS 901P manual)\n"
  "contrast <contrast>   Set the LCD contrast (0 - 63)\n"
};

void handleCLI() {
  char arg_str[16] = {};
  int arg_int = 0;

  if (strncmp(inBuffer, "help", 4) == 0) {
    // HELP
    for (const char* line: helpMessage) {
      Serial.print(line);
    }

  } else if (sscanf(inBuffer, "set-gas %15s", arg_str) == 1) {
    // SET GAS TYPE
    int type = MKS_901P::toGasType(arg_str);
    if (type < 0) {
      Serial.println("set-gas: invalid gas type");
      return;
    }
    auto res = sensor.writeGasType((MKS_901P::GasType) type);
    if (res == MKS_901P::ErrNone) {
      Serial.println("success");
    } else {
      Serial.print("error: ");
      Serial.println(MKS_901P::toString(res));
    }
  } else if (sscanf(inBuffer, "contrast %d", &arg_int) == 1) {
    // SET DISPLAY CONTRAST
    const int contrast = arg_int;
    if (contrast < 0 || contrast > 63) {
      Serial.println("contrast: invalid contrast value");
    }
    lcd.setContrast(contrast);
    Serial.println("success");
  } else {
    Serial.println("Unrecognized command. Type `help` for help.");
  }
}

void tryReadCLI() {
  while (Serial.available() > 0) {
    int next = Serial.read();

    if (inIdx == inLastIndex) {
      if (next == '\n') {
        memset(inBuffer, 0, sizeof(inBuffer));
        inIdx = 0;
        Serial.println("Line too long, try again.");
      }
      continue;
    }

    if (next == '\b') {
      if (inIdx > 0)
        inBuffer[--inIdx] = '\0';
      Serial.print("\b \b");
      continue;
    }

    Serial.write((char) next);

    if (next == '\n') {
      handleCLI();
      memset(inBuffer, 0, sizeof(inBuffer));
      inIdx = 0;
    } else {
      inBuffer[inIdx++] = next;
    }
  }
}

void tryReadPressure() {
  static std::pair<int, int> pressure = {};
  static MKS_901P::GasType gasType = {};

  if (sensor.readPR4(pressure) != MKS_901P::ErrNone)
    return;
  if (sensor.readGasType(gasType) != MKS_901P::ErrNone)
    return;

  {
    // first line
    auto [mantissa, exponent] = pressure;
    strfReadingSI(lcdBuffer, sizeof(lcdBuffer), mantissa, exponent, "torr");
    lcd.setCursor(0, 0);
    lcd.printf("%-20s", lcdBuffer);
  }
  {
    snprintf(lcdBuffer, sizeof(lcdBuffer), "gas: %s", MKS_901P::toString(gasType));
    lcd.setCursor(0, 1);
    lcd.printf("%-20s", lcdBuffer);
  }
}

void loop() {
  int now = millis();
  if (now - msPressureLast > 250) {
    msPressureLast = now;
    tryReadPressure();
  }
  tryReadCLI();
}
