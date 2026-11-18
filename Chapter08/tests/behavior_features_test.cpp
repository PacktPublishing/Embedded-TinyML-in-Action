#include <cassert>
#include <cmath>
#include <iostream>

#include "../firmware/behavior_features.h"

bool near(float a, float b, float tolerance = 0.001f) {
  return std::fabs(a - b) <= tolerance;
}

int main() {
  BehaviorSnapshot normal;
  normal.peakDynamicAccelerationG = 0.02f;
  normal.meanLoopUs = 20;
  normal.maxLoopUs = 4400;
  normal.packetsSent = 5;
  normal.peripheralFailedChecks = 0;
  normal.batteryDropV = 0.01f;
  normal.enclosureTransitions = 0;

  BehaviorFeatures normalFeatures =
      BehaviorFeatures::fromSnapshot(
          normal,
          20.0f,
          4400.0f,
          5.0f,
          50.0f);

  assert(near(normalFeatures.values[0], 0.02f));
  assert(near(normalFeatures.values[1], 1.0f));
  assert(near(normalFeatures.values[2], 1.0f));
  assert(near(normalFeatures.values[3], 1.0f));
  assert(near(normalFeatures.values[4], 0.0f));
  assert(near(normalFeatures.values[5], 0.01f));
  assert(near(normalFeatures.values[6], 0.0f));

  BehaviorSnapshot disturbed;
  disturbed.peakDynamicAccelerationG = 0.20f;
  disturbed.meanLoopUs = 26;
  disturbed.maxLoopUs = 6600;
  disturbed.packetsSent = 8;
  disturbed.peripheralFailedChecks = 5;
  disturbed.batteryDropV = 0.08f;
  disturbed.enclosureTransitions = 1;

  BehaviorFeatures abnormalFeatures =
      BehaviorFeatures::fromSnapshot(
          disturbed,
          20.0f,
          4400.0f,
          5.0f,
          50.0f);

  assert(near(abnormalFeatures.values[0], 0.20f));
  assert(near(abnormalFeatures.values[1], 1.30f));
  assert(near(abnormalFeatures.values[2], 1.50f));
  assert(near(abnormalFeatures.values[3], 1.60f));
  assert(near(abnormalFeatures.values[4], 0.10f));
  assert(near(abnormalFeatures.values[5], 0.08f));
  assert(near(abnormalFeatures.values[6], 1.0f));

  std::cout
      << "PASS: behavior feature construction and baseline ratios\n";

  return 0;
}
