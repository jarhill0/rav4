
#include "analogWave.h" 

analogWave wave(DAC);   

int freq = 1;

void setup() {
  Serial.begin(115200);  
  wave.square(freq);       
}

void loop() {
}
