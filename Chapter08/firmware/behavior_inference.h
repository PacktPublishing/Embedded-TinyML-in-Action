#pragma once

#include <stdint.h>

#include "behavior_features.h"

struct BehaviorInferenceResult {
  bool valid = false;
  bool abnormal = false;
  float reconstructionError = 0.0f;
  uint32_t inferenceUs = 0;
};

class BehaviorInference {
 public:
  bool begin();

  BehaviorInferenceResult run(
      const BehaviorFeatures &features);

 private:
  static constexpr size_t TENSOR_ARENA_SIZE = 8192;

  alignas(16) uint8_t tensorArena_[TENSOR_ARENA_SIZE];

  bool ready_ = false;
};
