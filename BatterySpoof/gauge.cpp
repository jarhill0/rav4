#include "gauge.hpp"

analogWave wave(DAC);

void set_gauge_soc(float soc) {
  // ~47 Hz is the bottom of the SoC gauge, and 150 Hz is the top
  float freq = 47 + (1.03 * soc);
  wave.freq(freq);
}

void init_gauge_soc() {
  wave.square(40);
  set_gauge_soc(0);
}
