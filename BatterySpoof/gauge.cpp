#include "gauge.hpp"

analogWave wave(DAC);

void set_gauge_soc(int soc) {
  // ~47 Hz is the bottom of the SoC gauge, and 150 Hz is the top
  float freq = 47 + (1.03 * soc);
  wave.square(round(freq));
}
