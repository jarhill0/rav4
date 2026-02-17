#include <cassert>
#include <cstring>
#include <iostream>

#include "../spoof.hpp"

void test_checksum() {
  constexpr unsigned char arr[4] = {1, 1, 2, 4};

  assert(1 == checksum(arr, 1));
  assert(0 == checksum(arr, 2));
  assert(6 == checksum(arr, 4));
}

void test_healthy_packet() {
  init_packet();

  unsigned char *packet;
  const int length = healthy_packet(&packet);

  assert(255 == packet[0]);
  assert(length - 2 == packet[1]);
  assert(62 == length);
  assert(0x31 == packet[2]);

  unsigned char sum = 0;
  for (int i = 1; i <= packet[1]; i++) {
    sum ^= packet[i];
  }
  assert(packet[packet[1] + 1] == sum);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (packet[i + 1] << 8) + packet[i];
    assert(12800 == voltage);
  }

  for (int i = 3 + 24 * 2; i < 3 + 24 * 2 + 4 * 2; i += 2) {
    const int temperature = (packet[i + 1] << 8) + packet[i];
    assert(22222 == temperature);
  }
}

void test_set_voltage() {
  init_packet();
  set_voltage(13);

  unsigned char *packet;
  int length = healthy_packet(&packet);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (packet[i + 1] << 8) + packet[i];
    assert(13000 == voltage);
  }

  assert(0xff == checksum(packet, length));

  set_voltage((float)13.456);

  length = healthy_packet(&packet);

  for (int i = 3; i < 3 + 24 * 2; i += 2) {
    const int voltage = (packet[i + 1] << 8) + packet[i];
    assert(13456 == voltage);
  }

  assert(0xff == checksum(packet, length));
}

void test_set_temperature() {
  init_packet();
  set_temperature(27, 0);
  set_temperature((float)35.678, 1);
  set_temperature((float)12.001, 2);
  set_temperature(6, 3);

  unsigned char *packet;
  const int length = healthy_packet(&packet);

  int i = 3 + 24 * 2;
  const int temperature1 = (packet[i + 1] << 8) + packet[i];
  i += 2;
  const int temperature2 = (packet[i + 1] << 8) + packet[i];
  i += 2;
  const int temperature3 = (packet[i + 1] << 8) + packet[i];
  i += 2;
  const int temperature4 = (packet[i + 1] << 8) + packet[i];

  assert(27000 == temperature1);
  assert(35678 == temperature2);
  assert(12001 == temperature3);
  assert(6000 == temperature4);

  assert(0xff == checksum(packet, length));
}

bool contains(const char *s1, const char *s2) {
  return strstr(s1, s2) != nullptr;
}

void run_tests() {
  test_checksum();
  test_healthy_packet();
  test_set_voltage();
  test_set_temperature();
}

int main(int, char **) {
  run_tests();
  std::cout << "[BatterySpoof] All tests passed!" << std::endl;
}
