#include <cassert>
#include <iostream>

#include "../spoof.hpp"
#include "../time.hpp"

unsigned char PACKET[PACKET_LEN];

void test_checksum() {
  constexpr unsigned char arr[4] = {1, 1, 2, 4};

  assert(1 == checksum(arr, 1));
  assert(0 == checksum(arr, 2));
  assert(6 == checksum(arr, 4));
}

void test_init_packet() {
  init_packet(PACKET);

  assert(255 == PACKET[0]);
  assert(PACKET_LEN - 2 == PACKET[1]);
  assert(62 == PACKET_LEN);
  assert(0x31 == PACKET[2]);

  unsigned char sum = 0;
  for (int i = 1; i <= PACKET[1]; i++) {
    sum ^= PACKET[i];
  }
  assert(PACKET[PACKET[1] + 1] == sum);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (PACKET[i + 1] << 8) + PACKET[i];
    assert(12800 == voltage);
  }

  for (int i = 3 + 24 * 2; i < 3 + 24 * 2 + 4 * 2; i += 2) {
    const int temperature = (PACKET[i + 1] << 8) + PACKET[i];
    assert(22222 == temperature);
  }
}

void test_set_voltage() {
  init_packet(PACKET);
  set_voltage(PACKET, 13);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (PACKET[i + 1] << 8) + PACKET[i];
    assert(13000 == voltage);
  }

  assert(0xff == checksum(PACKET, PACKET_LEN));

  set_voltage(PACKET, 13.456);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (PACKET[i + 1] << 8) + PACKET[i];
    assert(13456 == voltage);
  }

  assert(0xff == checksum(PACKET, PACKET_LEN));
}

void test_set_temperature() {
  init_packet(PACKET);
  set_temperature(PACKET, 27, 0);
  set_temperature(PACKET, 35.678, 1);
  set_temperature(PACKET, 12.001, 2);
  set_temperature(PACKET, 6, 3);

  int i = 3 + 24 * 2;
  const int temperature1 = (PACKET[i + 1] << 8) + PACKET[i];
  i += 2;
  const int temperature2 = (PACKET[i + 1] << 8) + PACKET[i];
  i += 2;
  const int temperature3 = (PACKET[i + 1] << 8) + PACKET[i];
  i += 2;
  const int temperature4 = (PACKET[i + 1] << 8) + PACKET[i];

  assert(27000 == temperature1);
  assert(35678 == temperature2);
  assert(12001 == temperature3);
  assert(6000 == temperature4);

  assert(0xff == checksum(PACKET, PACKET_LEN));
}

void test_time_between() {
  assert(67 == time_between(100, 167));

  // sanity check that I understand constants and over/underflow
  unsigned long almost_overflow = -5;
  assert(almost_overflow == ULONG_MAX - 4);

  assert(6767 == time_between(almost_overflow, 6762));
}

void run_tests() {
  test_checksum();
  test_init_packet();
  test_set_voltage();
  test_set_temperature();
  test_time_between();
}

int main(int, char **) {
  run_tests();
  std::cout << "[BatterySpoof] All tests passed!" << std::endl;
}
