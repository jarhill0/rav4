#ifndef RAV4_SPOOF_HPP
#define RAV4_SPOOF_HPP

constexpr int PACKET_LEN = 62;

void init_packet(unsigned char *);
unsigned char checksum(const unsigned char *arr, int len);
void set_voltage(unsigned char *, int);
void set_voltage(unsigned char *, double);
void set_voltage(unsigned char *, double, int);
void set_temperature(unsigned char *, int, int);
void set_temperature(unsigned char *, double, int);

#endif
