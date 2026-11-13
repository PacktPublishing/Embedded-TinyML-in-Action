#pragma once

#include <stdint.h>

struct DirectCheckState {
  bool enclosureOutsideMaintenance = false;
  bool peripheralUnavailable = false;
  uint16_t consecutivePeripheralFailures = 0;

  bool anyTriggered() const {
    return enclosureOutsideMaintenance || peripheralUnavailable;
  }
};

class DirectChecks {
 public:
  explicit DirectChecks(uint16_t peripheralFailureLimit);

  void reset();

  DirectCheckState update(bool enclosureOpen,
                          bool maintenanceActive,
                          bool peripheralResponding);

  const DirectCheckState &state() const {
    return state_;
  }

 private:
  uint16_t peripheralFailureLimit_;
  DirectCheckState state_;
};
