#include "behavior_inference.h"

#include <Arduino.h>
#include <Chirale_TensorFlowLite.h>

#include <cmath>

#include "behavior_model_data.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace {

const tflite::Model *gModel = nullptr;
tflite::MicroInterpreter *gInterpreter = nullptr;
TfLiteTensor *gInput = nullptr;
TfLiteTensor *gOutput = nullptr;

tflite::MicroMutableOpResolver<1> gResolver;

}  // namespace

bool BehaviorInference::begin() {
  gModel = tflite::GetModel(
      BehaviorModel::MODEL_DATA);

  if (gModel == nullptr) {
    return false;
  }

  if (gModel->version() != TFLITE_SCHEMA_VERSION) {
    return false;
  }

  if (gResolver.AddFullyConnected() != kTfLiteOk) {
    return false;
  }

  static tflite::MicroInterpreter interpreter(
      gModel,
      gResolver,
      tensorArena_,
      TENSOR_ARENA_SIZE);

  gInterpreter = &interpreter;

  if (gInterpreter->AllocateTensors() != kTfLiteOk) {
    return false;
  }

  gInput = gInterpreter->input(0);
  gOutput = gInterpreter->output(0);

  if (gInput == nullptr || gOutput == nullptr) {
    return false;
  }

  if (gInput->type != kTfLiteInt8 ||
      gOutput->type != kTfLiteInt8) {
    return false;
  }

  ready_ = true;
  return true;
}

BehaviorInferenceResult BehaviorInference::run(
    const BehaviorFeatures &features) {

  BehaviorInferenceResult result;

  if (!ready_ ||
      gInterpreter == nullptr ||
      gInput == nullptr ||
      gOutput == nullptr) {
    return result;
  }

  float normalized[BehaviorModel::FEATURE_COUNT];

  for (size_t i = 0;
       i < BehaviorModel::FEATURE_COUNT;
       ++i) {

    normalized[i] =
        (features.values[i] -
         BehaviorModel::FEATURE_MEAN[i]) /
        BehaviorModel::FEATURE_SCALE[i];

    const float quantized =
        normalized[i] /
            gInput->params.scale +
        gInput->params.zero_point;

    int32_t q =
        static_cast<int32_t>(
            std::round(quantized));

    if (q < -128) q = -128;
    if (q > 127) q = 127;

    gInput->data.int8[i] =
        static_cast<int8_t>(q);
  }

  const uint32_t startUs = micros();

  if (gInterpreter->Invoke() != kTfLiteOk) {
    return result;
  }

  result.inferenceUs = micros() - startUs;

  float squaredError = 0.0f;

  for (size_t i = 0;
       i < BehaviorModel::FEATURE_COUNT;
       ++i) {

    const float reconstructed =
        (static_cast<int32_t>(
             gOutput->data.int8[i]) -
         gOutput->params.zero_point) *
        gOutput->params.scale;

    const float error =
        normalized[i] - reconstructed;

    squaredError += error * error;
  }

  result.reconstructionError =
      squaredError /
      static_cast<float>(
          BehaviorModel::FEATURE_COUNT);

  result.abnormal =
      result.reconstructionError >
      BehaviorModel::ANOMALY_THRESHOLD;

  result.valid = true;

  return result;
}
