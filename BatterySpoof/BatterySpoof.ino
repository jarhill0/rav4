#include "can.hpp"
#include "gauge.hpp"
#include "spoof.hpp"
#if INTERACTIVE
#include "interactive.hpp"
#endif

void setup() {
  Serial1.begin(2400, SERIAL_8N1);
  init_packet();
#if DEBUG_LOG || INTERACTIVE
  Serial.begin(9600);
#endif

  set_gauge_soc(0);
  init_can();
}

void loop() {
  if (Serial1.available() >= 2) {
    car_request();
  }
  float soc;
  if ((soc = read_soc()) != -1) {
    set_gauge_soc(soc);
  }
#if INTERACTIVE
  else if (Serial.available() != 0) {
    interactive();
  }
#endif
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

  unsigned char *packet;
  int packet_len = healthy_packet(&packet);
  Serial1.write(packet, packet_len);

#if DEBUG_LOG
  Serial.print("wrote ");
  Serial.print(packet_len);
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

#if INTERACTIVE
void interactive() {
  String command = Serial.readStringUntil('\n');
  const char *response = handle_command(command.c_str());
  if (!response[0])
    return;

  Serial.println(response);
}
#endif
