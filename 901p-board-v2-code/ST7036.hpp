#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <Print.h>
#include <Wire.h>

// Custom driver class for the ST7036 and compatible displays. Should work
// almost exactly like the LiquidCrystal Arduino library.
class ST7036 : public Print {
public:
  ST7036(TwoWire& wire, uint8_t addr, uint8_t lines = 2, uint8_t cols = 20);

  // Perform all initialization. Returns true if successful.
  // The provided I2C bus *must* be initialized prior to calling this function.
  bool begin();

  // CURSOR OPERATIONS
  // ==================

  // Clears the display and moves the cursor to the top left.
  void clear();
  // Returns the cursor to the top left.
  void home();
  // Sets the cursor position.
  void setCursor(uint8_t col, uint8_t row);

  // Shifts the entire display and cursor one cell to the left.
  void scrollDisplayLeft();
  // Shifts the entire display and cursor one cell to the right.
  void scrollDisplayRight();

  // BOOLEAN FLAGS
  // ==================

  void cursor() {
    assert(_init);
    _cursor = true;
    syncOnOff();
  }
  void noCursor() {
    assert(_init);
    _cursor = false;
    syncOnOff();
  }
  void blink() {
    assert(_init);
    _blink = true;
    syncOnOff();
  }
  void noBlink() {
    assert(_init);
    _blink = false;
    syncOnOff();
  }
  void display() {
    assert(_init);
    _display = true;
    syncOnOff();
  }
  void noDisplay() {
    assert(_init);
    _display = false;
    syncOnOff();
  }

  void leftToRight() {
    assert(_init);
    _isDirRight = true;
    syncEntryMode();
  }

  void rightToLeft() {
    assert(_init);
    _isDirRight = false;
    syncEntryMode();
  }

  void autoscroll() {
    assert(_init);
    _autoscroll = true;
    syncEntryMode();
  }

  void noAutoscroll() {
    assert(_init);
    _autoscroll = false;
    syncEntryMode();
  }
  
  size_t write(uint8_t data) override;
  size_t write(const uint8_t* data, size_t size) override;
private:
  // Syncs _display, _cursor, and _blink to the display.
  void syncOnOff();

  // Syncs _isDirRight and _autoscroll to the display;
  void syncEntryMode();

  // parameters
  TwoWire& _wire;
  uint8_t _addr;
  uint8_t _lines;
  uint8_t _cols;
  // state
  bool _init : 1 = false;
  // Display ON/OFF bits
  bool _display : 1 = true;
  bool _cursor : 1 = false;
  bool _blink : 1 = false;
  // Entry mode bits
  bool _isDirRight : 1 = true;
  bool _autoscroll : 1 = false;
};