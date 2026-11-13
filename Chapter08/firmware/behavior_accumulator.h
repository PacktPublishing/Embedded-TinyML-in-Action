#pragma once

#include <stdint.h>
#include "behavior_snapshot.h"

class BehaviorAccumulator {
 public:
  void reset();

  void sampleEnclosure(bool open);
  void sampleMaintenance(bool active);
  void sampleMovement(float dynamicAccelerationG);
  void samplePeripheral(bool responding);
  void sampleBattery(float voltage);
  void notePacketSent();
  void recordLoopExecution(uint32_t elapsedUs);

  BehaviorSnapshot makeSnapshot(uint32_t sequence,
                                uint32_t resetReason,
                                uint32_t bootCount) const;

 private:
  bool enclosureSampled_ = false;
  bool enclosureOpenNow_ = false;
  bool enclosureOpenSeen_ = false;
  uint16_t enclosureTransitions_ = 0;
  bool maintenanceActive_ = false;

  float peakDynamicAccelerationG_ = 0.0f;

  uint64_t loopTimeTotalUs_ = 0;
  uint32_t loopSamples_ = 0;
  uint32_t maxLoopUs_ = 0;

  uint16_t packetsSent_ = 0;
  uint16_t peripheralFailedChecks_ = 0;

  bool batteryValid_ = false;
  float batteryStartV_ = 0.0f;
  float minimumBatteryV_ = 0.0f;
};
