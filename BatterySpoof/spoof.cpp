#include "spoof.hpp"

unsigned char checksum(const unsigned char *arr, const int len) {
  unsigned char out = 0;
  for (int i = 0; i < len; i++) {
    out ^= arr[i];
  }
  return out;
}

void init_packet(unsigned char *packet) {
  int i = 0;
  packet[i++] = 255;            // header
  packet[i++] = PACKET_LEN - 2; // header: length
  packet[i++] = 49;             // header: unknown
  for (int j = 0; j < 24; j++) {
    // voltage: 12.8 V
    packet[i++] = 0;  // low byte
    packet[i++] = 50; // high byte
  }
  for (int j = 0; j < 4; j++) {
    // temperature: 72 F (22.222 Celsius)
    packet[i++] = 206; // low byte
    packet[i++] = 86;  // high byte
  }
  packet[i++] = 0; // padding to reach full length?
  packet[i++] = 0;

  packet[i] = checksum(packet + 1, i - 1);
  i++;
}

void set_voltage(unsigned char *packet, int voltage) {
  set_voltage(packet, static_cast<double>(voltage));
}

void set_voltage(unsigned char *packet, double v) {
  int voltage = (int)(v * 1000);
  unsigned char low = voltage & 0xff;
  unsigned char high = (voltage >> 8) & 0xff;

  int i = 3;
  for (int j = 0; j < 24; j++) {
    packet[i++] = low;
    packet[i++] = high;
  }
  packet[PACKET_LEN - 1] = checksum(packet + 1, PACKET_LEN - 2);
}

void set_temperature(unsigned char *packet, int temp, int ind) {
  set_temperature(packet, static_cast<double>(temp), ind);
}

void set_temperature(unsigned char *packet, double t, int ind) {
  int temp = (int)(t * 1000);
  unsigned char low = temp & 0xff;
  unsigned char high = (temp >> 8) & 0xff;

  int i = 3 + 24 * 2 + ind * 2;
  packet[i++] = low;
  packet[i++] = high;
  packet[PACKET_LEN - 1] = checksum(packet + 1, PACKET_LEN - 2);
}
