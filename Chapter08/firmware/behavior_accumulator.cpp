#include "behavior_accumulator.h"

#include <algorithm>

void BehaviorAccumulator::reset() {
  // Preserve the last enclosure state across observation windows so that
  // a state change at a window boundary is still counted as a transition.
  enclosureOpenSeen_ = false;
  enclosureTransitions_ = 0;
  maintenanceActive_ = false;

  peakDynamicAccelerationG_ = 0.0f;

  loopTimeTotalUs_ = 0;
  loopSamples_ = 0;
  maxLoopUs_ = 0;

  packetsSent_ = 0;
  peripheralFailedChecks_ = 0;

  batteryValid_ = false;
  batteryStartV_ = 0.0f;
  minimumBatteryV_ = 0.0f;
}

void BehaviorAccumulator::sampleEnclosure(bool open) {
  if (!enclosureSampled_) {
    enclosureSampled_ = true;
    enclosureOpenNow_ = open;
  } else if (open != enclosureOpenNow_) {
    enclosureOpenNow_ = open;
    ++enclosureTransitions_;
  }

  enclosureOpenSeen_ = enclosureOpenSeen_ || open;
}

void BehaviorAccumulator::sampleMaintenance(bool active) {
  maintenanceActive_ = active;
}

void BehaviorAccumulator::sampleMovement(float dynamicAccelerationG) {
  peakDynamicAccelerationG_ =
      std::max(peakDynamicAccelerationG_, dynamicAccelerationG);
}

void BehaviorAccumulator::samplePeripheral(bool responding) {
  if (!responding) {
    ++peripheralFailedChecks_;
  }
}

void BehaviorAccumulator::sampleBattery(float voltage) {
  if (!batteryValid_) {
    batteryValid_ = true;
    batteryStartV_ = voltage;
    minimumBatteryV_ = voltage;
    return;
  }

  minimumBatteryV_ = std::min(minimumBatteryV_, voltage);
}

void BehaviorAccumulator::notePacketSent() {
  ++packetsSent_;
}

void BehaviorAccumulator::recordLoopExecution(uint32_t elapsedUs) {
  loopTimeTotalUs_ += elapsedUs;
  ++loopSamples_;
  maxLoopUs_ = std::max(maxLoopUs_, elapsedUs);
}

BehaviorSnapshot BehaviorAccumulator::makeSnapshot(uint32_t sequence,
                                                   uint32_t resetReason,
                                                   uint32_t bootCount) const {
  BehaviorSnapshot snapshot;
  snapshot.sequence = sequence;
  snapshot.enclosureOpenSeen = enclosureOpenSeen_;
  snapshot.enclosureTransitions = enclosureTransitions_;
  snapshot.maintenanceActive = maintenanceActive_;
  snapshot.peakDynamicAccelerationG = peakDynamicAccelerationG_;
  snapshot.resetReason = resetReason;
  snapshot.bootCount = bootCount;
  snapshot.meanLoopUs = loopSamples_ > 0
                            ? static_cast<uint32_t>(loopTimeTotalUs_ / loopSamples_)
                            : 0;
  snapshot.maxLoopUs = maxLoopUs_;
  snapshot.packetsSent = packetsSent_;
  snapshot.peripheralFailedChecks = peripheralFailedChecks_;
  snapshot.batteryValid = batteryValid_;
  snapshot.batteryStartV = batteryStartV_;
  snapshot.minimumBatteryV = minimumBatteryV_;
  snapshot.batteryDropV =
      batteryValid_ && batteryStartV_ > minimumBatteryV_
          ? batteryStartV_ - minimumBatteryV_
          : 0.0f;
  return snapshot;
}
