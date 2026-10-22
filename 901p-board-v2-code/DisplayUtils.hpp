#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iterator>

// Prints a sensor reading (mantissa (* 1000) and exponent) with units using SI prefixes.
// Supports values between 1.000e-18 and 9.999e+20; returns error via errno in case of failure.
inline int strfReadingSI(char* output, size_t len, int mantissa, int exponent, const char* unit) {
  constexpr char prefixes[] = {'a', 'f', 'p', 'n', 'u', 'm', '\0', 'k', 'M', 'G', 'T', 'P', 'E'};
  constexpr int pow10[] = {1, 10, 100, 1000};

  int prefixIndex = (exponent + 18) / 3;
  if (prefixIndex < 0 || prefixIndex >= std::size(prefixes)) {
    errno = E2BIG;
    return -1;
  }

  int iDigits = (exponent + 18) % 3 + 1;
  int fDigits = 4 - iDigits;

  // 1: 1.234 -> 3
  // 2: 12.34 -> 2
  // 3: 123.4 -> 1

  int divFactor = pow10[fDigits];
  int iPart = mantissa / divFactor;
  int fPart = mantissa % divFactor;

  char prefix = prefixes[prefixIndex];
  if (prefix == '\0')
    return snprintf(output, len, "%0*u.%0*u %s", iDigits, iPart, fDigits, fPart, unit);

  return snprintf(output, len, "%0*u.%0*u %c%s", iDigits, iPart, fDigits, fPart, prefix, unit);
}
