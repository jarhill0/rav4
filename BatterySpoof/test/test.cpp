#include <cassert>
#include <cstring>
#include <iostream>

#include "../interactive.hpp"
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

void test_handle_command() {
  assert(0 == strlen(handle_command("")));
  assert(0 == strlen(handle_command("            ")));
  assert(contains(handle_command("K 2"), "Unknown command"));
  assert(contains(handle_command("V2 "), "expects a single number"));
  assert(contains(handle_command("V2"), "Expected space"));
  assert(contains(handle_command("V"), "Expected something"));
  assert(contains(handle_command("V2 a"), "expects a single number"));
  assert(contains(handle_command("V2 3 a"), "expects a single number"));
  assert(contains(handle_command("V2 3 3"), "expects a single number"));
  assert(
      contains(handle_command("V25 3"), "Expected a cell number from 1 to 24"));
  assert(contains(handle_command("V125 3"),
                  "Expected space following cell number"));
  assert(contains(handle_command("T2"), "Expected space"));
  assert(contains(handle_command("T2 "), "expects a single number"));
  assert(contains(handle_command("T 2"), "Expected temperature sensor number"));
  assert(contains(handle_command("V 2 4"), "expects a single number"));
  assert(contains(handle_command("T1 2 4"), "expects a single number"));
  assert(contains(handle_command("V 2a"), "expects a single number"));
  assert(contains(handle_command("T1 2a"), "expects a single number"));
  assert(contains(handle_command("T1 2."), "expects a single number"));
  assert(contains(handle_command("T1 .2"), "expects a single number"));
  assert(contains(handle_command("T1 ."), "expects a single number"));
  assert(contains(handle_command("  V  \t20"), "Set voltage"));
  assert(contains(handle_command("  V  \t20.002"), "Set voltage"));
  assert(contains(handle_command("\t   T1  13 "), "Set temperature"));
  assert(contains(handle_command("\t   T1  13.456 "), "Set temperature"));

  init_packet();
  handle_command("V 13");
  unsigned char *packet;
  healthy_packet(&packet);
  assert(0xc8 == packet[3]);
  assert(0x32 == packet[4]);
  assert(0xc8 == packet[3 + 23 * 2]);
  assert(0x32 == packet[4 + 23 * 2]);

  handle_command("T1 21");
  healthy_packet(&packet);
  assert(0x8 == packet[3 + 24 * 2]);
  assert(0x52 == packet[3 + 24 * 2 + 1]);

  handle_command("T1 21.001");
  healthy_packet(&packet);
  assert(0x9 == packet[3 + 24 * 2]);
  assert(0x52 == packet[3 + 24 * 2 + 1]);

  handle_command("V 13.002");
  healthy_packet(&packet);
  assert(0xca == packet[3]);
  assert(0x32 == packet[4]);

  handle_command("V1 10.001");
  handle_command("V2 11.002");
  handle_command("V24 12.003");
  healthy_packet(&packet);
  assert(0x11 == packet[3]);
  assert(0x27 == packet[4]);
  assert(0xfa == packet[5]);
  assert(0x2a == packet[6]);
  assert(0xe3 == packet[3 + 23 * 2]);
  assert(0x2e == packet[4 + 23 * 2]);
}

void run_tests() {
  test_checksum();
  test_healthy_packet();
  test_set_voltage();
  test_set_temperature();
  test_handle_command();
}

int main(int, char **) {
  run_tests();
  std::cout << "[BatterySpoof] All tests passed!" << std::endl;
}
