#include "direct_checks.h"

#include <limits.h>

DirectChecks::DirectChecks(uint16_t peripheralFailureLimit)
    : peripheralFailureLimit_(peripheralFailureLimit == 0
                                  ? 1
                                  : peripheralFailureLimit) {}

void DirectChecks::reset() {
  state_ = DirectCheckState{};
}

DirectCheckState DirectChecks::update(bool enclosureOpen,
                                      bool maintenanceActive,
                                      bool peripheralResponding) {
  state_.enclosureOutsideMaintenance =
      enclosureOpen && !maintenanceActive;

  if (peripheralResponding) {
    state_.consecutivePeripheralFailures = 0;
  } else if (state_.consecutivePeripheralFailures < UINT16_MAX) {
    ++state_.consecutivePeripheralFailures;
  }

  state_.peripheralUnavailable =
      state_.consecutivePeripheralFailures >= peripheralFailureLimit_;

  return state_;
}
