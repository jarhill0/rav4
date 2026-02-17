#include "can.hpp"
#include "gauge.hpp"
#include "spoof.hpp"
#include "time.hpp"

unsigned char HEALTHY_PACKET[PACKET_LEN];
unsigned char HIGH_VOLTAGE_PACKET[PACKET_LEN];

bool high_soc = false;
unsigned long soc_last_read;
constexpr unsigned long SOC_VALIDITY_MILLIS = 5000;

void setup() {
  Serial1.begin(2400, SERIAL_8N1);
  init_packet(HEALTHY_PACKET);
  init_packet(HIGH_VOLTAGE_PACKET);
  set_voltage(HIGH_VOLTAGE_PACKET, 14.5);
  soc_last_read = millis();

#if DEBUG_LOG
  Serial.begin(9600);
#endif

  init_gauge();
  init_can();
}

void loop() {
  if (Serial1.available() >= 2) {
    car_request();
  }
  float soc;
  if ((soc = read_soc()) != -1) {
    set_gauge_soc(soc);
    high_soc = (soc >= 90.0);
    soc_last_read = millis();
  }
}

void car_request() {
#if DEBUG_LOG
  Serial.println("reading bytes:");
#endif
  if (read_serial1() != 0xff) {
    return;
  }
  signed int num_bytes = read_serial1();
  if (num_bytes <= 0) {
    return;
  }
  delay(8 * num_bytes);
  char sum = num_bytes;
  int message_id = read_serial1();
  if (-1 == message_id) {
    return;
  }
  sum ^= message_id;
  for (int i = 1; i < num_bytes; i++) {
    signed int b = read_serial1();
    if (-1 == b) {
      return;
    }
    sum ^= b;
  }
  if (message_id != 0x31) {
    return;
  }
  if (sum != 0) {
    return;
  }

  if (!soc_is_recent() || high_soc) {
    Serial1.write(HIGH_VOLTAGE_PACKET, PACKET_LEN);
  } else {
    Serial1.write(HEALTHY_PACKET, PACKET_LEN);
  }

#if DEBUG_LOG
  Serial.print("wrote ");
  Serial.print(PACKET_LEN);
  Serial.println(" bytes");
#endif
}

int read_serial1() {
  int b = Serial1.read();
#if DEBUG_LOG
  Serial.print("0x");
  Serial.println(b, HEX);
#endif
  return b;
}

bool soc_is_recent() {
  const unsigned long now = millis();
  return time_between(soc_last_read, now) < SOC_VALIDITY_MILLIS;
}
