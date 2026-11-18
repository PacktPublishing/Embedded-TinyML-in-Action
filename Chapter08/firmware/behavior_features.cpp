#include "behavior_features.h"

namespace {

float safeRatio(float value, float baseline) {
  return baseline > 0.0f ? value / baseline : 0.0f;
}

}  // namespace

BehaviorFeatures BehaviorFeatures::fromSnapshot(
    const BehaviorSnapshot &snapshot,
    float baselineMeanLoopUs,
    float baselineMaxLoopUs,
    float baselinePacketsPerWindow,
    float expectedTelemetrySamples) {

  BehaviorFeatures features;

  features.values[0] =
      snapshot.peakDynamicAccelerationG;

  features.values[1] =
      safeRatio(
          static_cast<float>(snapshot.meanLoopUs),
          baselineMeanLoopUs);

  features.values[2] =
      safeRatio(
          static_cast<float>(snapshot.maxLoopUs),
          baselineMaxLoopUs);

  features.values[3] =
      safeRatio(
          static_cast<float>(snapshot.packetsSent),
          baselinePacketsPerWindow);

  features.values[4] =
      safeRatio(
          static_cast<float>(snapshot.peripheralFailedChecks),
          expectedTelemetrySamples);

  features.values[5] =
      snapshot.batteryDropV;

  features.values[6] =
      static_cast<float>(
          snapshot.enclosureTransitions);

  return features;
}
