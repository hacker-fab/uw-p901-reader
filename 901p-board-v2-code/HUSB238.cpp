#include "HUSB238.hpp"

#include "CommUtils.hpp"

constexpr uint8_t REG_PDSTATUS_0 = 0x00;
constexpr uint8_t REG_SRC_PDO = 0x08;
constexpr uint8_t REG_GO_COMMAND = 0x09;

constexpr uint8_t CMD_REQUEST_PDO = 0x01;


HUSB238::HUSB238(TwoWire& wire, uint8_t addr) : 
  _wire(wire), _addr(addr) {}

// Returns the current PD profile.
std::pair<HUSB238::Volts, HUSB238::Amps> HUSB238::currentProfile() {
  uint8_t b = wireRegisterReadByte(_addr, 0x00, _wire);
  Volts v = (Volts) (b >> 4);
  Amps a = (Amps) (b & 0x0F);
  return {v, a};
}

// Checks the available current profile for a given voltage.
// Returns an Amps value representing the maximum current.
// Returns -1 if the voltage is invalid or the profile is unavailable.
int HUSB238::checkVoltage(HUSB238::Volts volts) {
  if (volts < V5 || volts > V20)
    return -1;

  uint8_t regAddr = uint8_t(volts) + 1;
  uint8_t data = wireRegisterReadByte(_addr, regAddr, _wire);

  // bit 7 = 0: not detected
  if (!(data & 0x80))
    return -1;

  return Amps(data & 0x0F);
}

void HUSB238::requestVoltage(HUSB238::Volts volts) {
  if (volts == VNone)
    return;
  uint8_t pdoRequest = uint8_t(volts) << 4;
  wireRegisterWriteByte(_addr, REG_SRC_PDO, pdoRequest, _wire);
  wireRegisterWriteByte(_addr, REG_GO_COMMAND, CMD_REQUEST_PDO);
}