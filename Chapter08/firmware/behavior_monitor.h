#pragma once

#include <stdint.h>

#include "behavior_accumulator.h"
#include "behavior_snapshot.h"
#include "telemetry_io.h"

class BehaviorMonitor {
 public:
  explicit BehaviorMonitor(TelemetryIO &telemetry) : telemetry_(telemetry) {}

  void begin(uint32_t resetReason, uint32_t bootCount);
  void sampleTelemetry();
  void notePacketSent();
  void recordLoopExecution(uint32_t elapsedUs);

  bool observationReady(uint32_t nowMs) const;
  BehaviorSnapshot finishObservationWindow(uint32_t nowMs);

  bool enclosureOpenNow() const { return enclosureOpenNow_; }
  bool maintenanceActiveNow() const { return maintenanceActiveNow_; }

 private:
  TelemetryIO &telemetry_;
  BehaviorAccumulator accumulator_;

  uint32_t windowStartMs_ = 0;
  uint32_t resetReason_ = 0;
  uint32_t bootCount_ = 0;
  uint32_t sequence_ = 0;

  bool enclosureOpenNow_ = false;
  bool maintenanceActiveNow_ = false;
};
