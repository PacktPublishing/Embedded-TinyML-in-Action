#pragma once

#include <stdint.h>

#include "behavior_accumulator.h"
#include "behavior_snapshot.h"
#include "direct_checks.h"
#include "telemetry_io.h"

class BehaviorMonitor {
 public:
  BehaviorMonitor(TelemetryIO &telemetry,
                  uint16_t peripheralFailureLimit)
      : telemetry_(telemetry),
        directChecks_(peripheralFailureLimit) {}

  void begin(uint32_t resetReason, uint32_t bootCount);

  DirectCheckState sampleTelemetry();

  void notePacketSent();
  void recordLoopExecution(uint32_t elapsedUs);

  bool observationReady(uint32_t nowMs) const;
  BehaviorSnapshot finishObservationWindow(uint32_t nowMs);

  bool enclosureOpenNow() const { return enclosureOpenNow_; }
  bool maintenanceActiveNow() const { return maintenanceActiveNow_; }

  const DirectCheckState &directCheckState() const {
    return directChecks_.state();
  }

 private:
  TelemetryIO &telemetry_;
  BehaviorAccumulator accumulator_;
  DirectChecks directChecks_;

  uint32_t windowStartMs_ = 0;
  uint32_t resetReason_ = 0;
  uint32_t bootCount_ = 0;
  uint32_t sequence_ = 0;

  bool enclosureOpenNow_ = false;
  bool maintenanceActiveNow_ = false;
};
