#include "time.hpp"

unsigned long time_between(const unsigned long a, const unsigned long b) {
  if (b >= a) {
    return b - a;
  }
  // clock overflow?
  return (ULONG_MAX - a) + b + 1;
}
