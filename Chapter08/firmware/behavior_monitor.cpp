#include "behavior_monitor.h"

#include <Arduino.h>

#include "config.h"

void BehaviorMonitor::begin(uint32_t resetReason, uint32_t bootCount) {
  resetReason_ = resetReason;
  bootCount_ = bootCount;
  sequence_ = 0;
  accumulator_.reset();
  windowStartMs_ = millis();
}

void BehaviorMonitor::sampleTelemetry() {
  enclosureOpenNow_ = telemetry_.enclosureOpen();
  maintenanceActiveNow_ = telemetry_.maintenanceActive();

  accumulator_.sampleEnclosure(enclosureOpenNow_);
  accumulator_.sampleMaintenance(maintenanceActiveNow_);

  const bool peripheralOk = telemetry_.peripheralResponding();
  accumulator_.samplePeripheral(peripheralOk);

  float movementG = 0.0f;
  if (peripheralOk && telemetry_.readMovementDynamicG(movementG)) {
    accumulator_.sampleMovement(movementG);
  }

  float batteryV = 0.0f;
  if (telemetry_.readBatteryVoltage(batteryV)) {
    accumulator_.sampleBattery(batteryV);
  }
}

void BehaviorMonitor::notePacketSent() {
  accumulator_.notePacketSent();
}

void BehaviorMonitor::recordLoopExecution(uint32_t elapsedUs) {
  accumulator_.recordLoopExecution(elapsedUs);
}

bool BehaviorMonitor::observationReady(uint32_t nowMs) const {
  return static_cast<uint32_t>(nowMs - windowStartMs_) >= Config::OBSERVATION_MS;
}

BehaviorSnapshot BehaviorMonitor::finishObservationWindow(uint32_t nowMs) {
  BehaviorSnapshot snapshot =
      accumulator_.makeSnapshot(++sequence_, resetReason_, bootCount_);
  accumulator_.reset();
  windowStartMs_ = nowMs;
  return snapshot;
}
