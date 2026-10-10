#pragma once

#include <cassert>
#include <cstdint>
#include <Wire.h>

class HUSB238 {
public:
  // Voltage profile for PD.
  enum Volts {
    VNone = 0,
    V5 = 1,
    V9,
    V12,
    V15,
    V18,
    V20
  };
  // Current profile for PD.
  enum Amps {
    A0_50 = 0,
    A0_70,
    A1_00,
    A1_25,
    A1_50,
    A1_75,
    A2_00,
    A2_25,
    A2_50,
    A2_75,
    A3_00,
    A3_25,
    A3_50,
    A4_00,
    A4_50,
    A5_00,
  };

  HUSB238(TwoWire& wire, uint8_t addr);

  // Returns the current PD profile.
  std::pair<Volts, Amps> currentProfile();

  // Checks the available current profile for a given voltage.
  // Returns an Amps value representing the maximum current.
  // Returns -1 if the voltage is invalid or the profile is unavailable.
  int checkVoltage(Volts volts);

  void requestVoltage(Volts volts);

private:
  TwoWire& _wire;
  uint8_t _addr;
};