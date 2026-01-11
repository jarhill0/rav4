
#include "analogWave.h" 

analogWave wave(DAC);   

int freq = 100;  // in hertz, change accordingly

void setup() {
  Serial.begin(115200);  
  wave.square(freq);       
}

void loop() {
}
