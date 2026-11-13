#pragma once

#include <stdint.h>

struct BehaviorSnapshot {
  uint32_t sequence = 0;

  bool enclosureOpenSeen = false;
  uint16_t enclosureTransitions = 0;
  bool maintenanceActive = false;

  float peakDynamicAccelerationG = 0.0f;

  uint32_t resetReason = 0;
  uint32_t bootCount = 0;

  uint32_t meanLoopUs = 0;
  uint32_t maxLoopUs = 0;

  uint16_t packetsSent = 0;
  uint16_t peripheralFailedChecks = 0;

  bool batteryValid = false;
  float batteryStartV = 0.0f;
  float minimumBatteryV = 0.0f;
  float batteryDropV = 0.0f;
};
