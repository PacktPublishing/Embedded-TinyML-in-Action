#include <cassert>
#include <cmath>
#include <iostream>

#include "../firmware/behavior_accumulator.h"

bool near(float a, float b, float tolerance = 0.001f) {
  return std::fabs(a - b) <= tolerance;
}

int main() {
  BehaviorAccumulator accumulator;
  accumulator.reset();

  // Window 1: normal operation.
  accumulator.sampleEnclosure(false);
  accumulator.sampleMaintenance(false);
  accumulator.sampleMovement(0.02f);
  accumulator.sampleMovement(0.05f);
  accumulator.samplePeripheral(true);
  accumulator.samplePeripheral(true);
  accumulator.sampleBattery(4.12f);
  accumulator.sampleBattery(4.08f);
  accumulator.notePacketSent();
  accumulator.notePacketSent();
  accumulator.notePacketSent();
  accumulator.recordLoopExecution(700);
  accumulator.recordLoopExecution(800);
  accumulator.recordLoopExecution(900);

  BehaviorSnapshot normal = accumulator.makeSnapshot(1, 1, 4);
  assert(!normal.enclosureOpenSeen);
  assert(normal.enclosureTransitions == 0);
  assert(!normal.maintenanceActive);
  assert(near(normal.peakDynamicAccelerationG, 0.05f));
  assert(normal.meanLoopUs == 800);
  assert(normal.maxLoopUs == 900);
  assert(normal.packetsSent == 3);
  assert(normal.peripheralFailedChecks == 0);
  assert(normal.batteryValid);
  assert(near(normal.batteryStartV, 4.12f));
  assert(near(normal.minimumBatteryV, 4.08f));
  assert(near(normal.batteryDropV, 0.04f));

  // Window 2: disturbance. Reset first to prove no stale state carries over.
  accumulator.reset();
  accumulator.sampleEnclosure(false);
  accumulator.sampleEnclosure(true);
  accumulator.sampleMaintenance(false);
  accumulator.sampleMovement(0.18f);
  accumulator.sampleMovement(0.92f);
  accumulator.samplePeripheral(false);
  accumulator.samplePeripheral(false);
  accumulator.samplePeripheral(false);
  accumulator.sampleBattery(4.10f);
  accumulator.sampleBattery(3.70f);
  for (int i = 0; i < 10; ++i) accumulator.notePacketSent();
  accumulator.recordLoopExecution(2000);
  accumulator.recordLoopExecution(3000);
  accumulator.recordLoopExecution(4000);

  BehaviorSnapshot disturbed = accumulator.makeSnapshot(2, 3, 4);
  assert(disturbed.enclosureOpenSeen);
  assert(disturbed.enclosureTransitions == 1);
  assert(near(disturbed.peakDynamicAccelerationG, 0.92f));
  assert(disturbed.meanLoopUs == 3000);
  assert(disturbed.maxLoopUs == 4000);
  assert(disturbed.packetsSent == 10);
  assert(disturbed.peripheralFailedChecks == 3);
  assert(near(disturbed.batteryStartV, 4.10f));
  assert(near(disturbed.minimumBatteryV, 3.70f));
  assert(near(disturbed.batteryDropV, 0.40f));

  // Regression test: an enclosure transition occurring at an observation
  // window boundary must still be counted in the new window.
  BehaviorAccumulator boundaryAccumulator;
  boundaryAccumulator.sampleEnclosure(false);

  BehaviorSnapshot boundaryNormal =
      boundaryAccumulator.makeSnapshot(1, 1, 1);
  assert(boundaryNormal.enclosureTransitions == 0);

  boundaryAccumulator.reset();
  boundaryAccumulator.sampleEnclosure(true);

  BehaviorSnapshot boundaryDisturbed =
      boundaryAccumulator.makeSnapshot(2, 1, 1);
  assert(boundaryDisturbed.enclosureOpenSeen);
  assert(boundaryDisturbed.enclosureTransitions == 1);

  std::cout << "PASS: behavior accumulator normal and disturbance windows\n";
  return 0;
}
