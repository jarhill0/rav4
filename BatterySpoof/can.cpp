#include "can.hpp"

bool can_initialized = false;

void init_can() {
  for (unsigned int mailbox = 0;
       mailbox < R7FA4M1_CAN::CAN_MAX_NO_STANDARD_MAILBOXES; mailbox++) {
    CAN.setFilterId_Standard(mailbox, 0x6b0);
  }

  can_initialized = CAN.begin(CanBitRate::BR_250k);
  if (!can_initialized) {
#if DEBUG_LOG
    Serial.println("Failed to initialize CAN.");
#endif
  }
}

// Reads the state-of-charge (SoC) from the CAN bus.
// Returns a float representing the SoC as a percentage (0.5% resolution).
// Returns -1 if no SoC message was available to read, if CAN was
// not initialized, or if there was an error condition.
float read_soc() {
  if (!can_initialized) {
    return -1;
  }
  if (!CAN.available()) {
    return -1;
  }
  CanMsg const msg = CAN.read();
  // msg.id is 0x6b0, due to filtering
#if DEBUG_LOG
  // TODO: verify that msg.is is always 0x6b0
  Serial.print("Read message from CAN with ID: ");
  Serial.println(msg.id);
#endif
  if (msg.data_length < 5) {
    return -1;
  }
#if DEBUG_LOG
  Serial.print("Read SoC from CAN bus: ");
  Serial.println(msg.data[4] / 2.0);
#endif
  return msg.data[4] / 2.0;
}
