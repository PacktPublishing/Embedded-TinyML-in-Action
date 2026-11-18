#pragma once

#include <stddef.h>

#include "behavior_snapshot.h"

struct BehaviorFeatures {
  static constexpr size_t COUNT = 7;

  float values[COUNT] = {0};

  static BehaviorFeatures fromSnapshot(
      const BehaviorSnapshot &snapshot,
      float baselineMeanLoopUs,
      float baselineMaxLoopUs,
      float baselinePacketsPerWindow,
      float expectedTelemetrySamples);
};
