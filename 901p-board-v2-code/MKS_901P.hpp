#pragma once

#include <array>
#include <string_view>
#include <Arduino.h>

class MKS_901P {
public:
  // Indicates the lack of an address.
  static constexpr uint8_t NoAddress = 254;

  enum Error {
    // Query was too long and could not be sent
    ErrSendFailed = -2,
    // Sensor reply could not be correctly parsed
    ErrParseFailed = -1,
    // The operation succeeded
    ErrNone = 0,
    // Zero adjustment at too high pressure
    ErrVAC = 8,
    // Atmospheric adjustment at too low pressure
    ErrATM = 9,
    // Unrecognized message
    ErrBadCommand = 160,
    // Invalid argument
    ErrInvalidArg = 169,
    // Value out of Range
    ErrOutOfRange = 172,
    // Command/query character invalid
    ErrBadCmdQueryChar = 175,
    // Not in setup mode
    ErrLocked = 180,
  };

  enum Units {
    UnitsTorr = 0,
    UnitsMbar,
    UnitsPascal,
  };

  enum GasType {
    GasNitrogen = 0,
    GasAir,
    GasArgon,
    GasHelium,
    GasHydrogen,
    GasH2O,
    GasNeon,
    GasCO2,
    GasXenon
  };

  static const char* toString(Error err);
  static const char* toString(Units units);
  static const char* toString(GasType gasType);
  static int toUnits(const char* str);
  static int toGasType(const char* str);

  MKS_901P(Stream& serial = Serial, uint8_t addr = NoAddress);

  // Returns the currently-active transmission address
  uint8_t addr() const { return _addr; }

  // Performs all initialization, including resolving an address if one was not specified.
  // The provided serial bus *must* be initialized prior to calling this function.
  bool begin();

  // RAW PROTOCOL OPERATIONS
  // ==================================

  // Issues a raw query, returning a string view pointing to an internal buffer with the result.
  // The data referred to by the string view is always followed by a null terminator. However,
  // it will be invalidated on the next query or command issued.
  //
  // Returns ErrNone if succeeded, or the error code if failed.
  Error rawQuery(const char* cmd, std::string_view& result);

  // Issues a raw command with the provided parameter.
  //
  // Returns ErrNone if succeeded, or the error code if failed.
  Error rawCommand(const char* cmd, const char* param);


  // PARSED PROTOCOL OPERATIONS
  // ==================================

  // Reads the aggregate pressure from the Piezo and MicroPirani sensor combined.
  // The first value returned is the significand (* 10^3), and the 2nd is the exponent.
  Error readPR4(std::pair<int, int>& result);

  // Gets the current units being used by the sensor.
  Error readUnits(Units& units);
  // Sets the sensor's current units.
  Error writeUnits(Units units);

  // Gets the sensor's current gas type.
  Error readGasType(GasType& gas);
  // Sets the gas type.
  Error writeGasType(GasType gas);

private:
  Error transact(size_t sendLen, std::string_view* result = nullptr);

  Stream& _serial;
  uint8_t _addr;
  bool _init;
  std::array<char, 32> _sendBuf;
  std::array<char, 32> _recvBuf;
};
