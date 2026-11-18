from pathlib import Path
import json

ROOT = Path(__file__).parent
FIRMWARE = ROOT.parent / "firmware"

model_bytes = (ROOT / "behavior_model_int8.tflite").read_bytes()

with open(ROOT / "training_metadata.json") as f:
    metadata = json.load(f)

features = metadata["features"]
means = metadata["scaler_mean"]
scales = metadata["scaler_scale"]

threshold = metadata["int8_anomaly_threshold"]

input_scale = metadata["int8_input_scale"]
input_zero = metadata["int8_input_zero_point"]

output_scale = metadata["int8_output_scale"]
output_zero = metadata["int8_output_zero_point"]


def float_array(values):
    return ", ".join(f"{v:.9g}f" for v in values)


byte_lines = []

for i in range(0, len(model_bytes), 12):
    chunk = model_bytes[i:i + 12]
    byte_lines.append(
        "    " + ", ".join(f"0x{b:02x}" for b in chunk)
    )

byte_blob = ",\n".join(byte_lines)

header = f"""#pragma once

#include <stddef.h>
#include <stdint.h>

// Generated from model/behavior_model_int8.tflite.
// Do not edit manually.

namespace BehaviorModel {{

constexpr size_t FEATURE_COUNT = {len(features)};

constexpr float FEATURE_MEAN[FEATURE_COUNT] = {{
    {float_array(means)}
}};

constexpr float FEATURE_SCALE[FEATURE_COUNT] = {{
    {float_array(scales)}
}};

constexpr float ANOMALY_THRESHOLD = {threshold:.9g}f;

constexpr float INPUT_QUANT_SCALE = {input_scale:.9g}f;
constexpr int32_t INPUT_ZERO_POINT = {input_zero};

constexpr float OUTPUT_QUANT_SCALE = {output_scale:.9g}f;
constexpr int32_t OUTPUT_ZERO_POINT = {output_zero};

alignas(16) const unsigned char MODEL_DATA[] = {{
{byte_blob}
}};

constexpr size_t MODEL_DATA_LEN = sizeof(MODEL_DATA);

}}  // namespace BehaviorModel
"""

output = FIRMWARE / "behavior_model_data.h"
output.write_text(header)

print("Generated:", output)
print("Features:", len(features))
print("Model bytes:", len(model_bytes))
print("INT8 threshold:", threshold)
