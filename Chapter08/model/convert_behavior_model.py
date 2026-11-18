from pathlib import Path
import json

import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.model_selection import train_test_split

ROOT = Path(__file__).parent

with open(ROOT / "training_metadata.json") as f:
    metadata = json.load(f)

FEATURES = metadata["features"]
MEAN = np.asarray(metadata["scaler_mean"], dtype=np.float32)
SCALE = np.asarray(metadata["scaler_scale"], dtype=np.float32)

model = tf.keras.models.load_model(
    ROOT / "behavior_autoencoder.keras"
)

calibration = pd.read_csv(
    ROOT / "calibration_data.csv"
)[FEATURES].to_numpy(dtype=np.float32)

calibration_scaled = (
    (calibration - MEAN) / SCALE
).astype(np.float32)


def representative_dataset():
    for row in calibration_scaled:
        yield [row.reshape(1, -1)]


converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [
    tf.lite.Optimize.DEFAULT
]

converter.representative_dataset = representative_dataset

converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS_INT8
]

converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

model_path = ROOT / "behavior_model_int8.tflite"
model_path.write_bytes(tflite_model)

print(
    "TFLite model size:",
    len(tflite_model),
    "bytes"
)


interpreter = tf.lite.Interpreter(
    model_path=str(model_path)
)

interpreter.allocate_tensors()

input_details = interpreter.get_input_details()[0]
output_details = interpreter.get_output_details()[0]

input_scale, input_zero = (
    input_details["quantization"]
)

output_scale, output_zero = (
    output_details["quantization"]
)

print(
    "Input quantization:",
    input_scale,
    input_zero
)

print(
    "Output quantization:",
    output_scale,
    output_zero
)


def run_tflite(x):
    q = np.round(
        x / input_scale + input_zero
    )

    q = np.clip(q, -128, 127).astype(np.int8)

    interpreter.set_tensor(
        input_details["index"],
        q.reshape(1, -1)
    )

    interpreter.invoke()

    output = interpreter.get_tensor(
        output_details["index"]
    )[0]

    return (
        output.astype(np.float32) - output_zero
    ) * output_scale


def reconstruction_errors(x):
    errors = []

    for row in x:
        reconstructed = run_tflite(row)
        errors.append(
            np.mean(
                np.square(row - reconstructed)
            )
        )

    return np.asarray(errors)


# Re-create the same held-out normal split used during training.
training = pd.read_csv(
    ROOT / "training_data.csv"
)

x = training[FEATURES].to_numpy(
    dtype=np.float32
)

_, x_validation = train_test_split(
    x,
    test_size=0.20,
    random_state=42,
    shuffle=True,
)

validation_scaled = (
    (x_validation - MEAN) / SCALE
).astype(np.float32)

validation_error = reconstruction_errors(
    validation_scaled
)

threshold = float(
    np.percentile(validation_error, 99.0)
)

evaluation = pd.read_csv(
    ROOT / "evaluation_data.csv"
)

eval_x = evaluation[FEATURES].to_numpy(
    dtype=np.float32
)

eval_scaled = (
    (eval_x - MEAN) / SCALE
).astype(np.float32)

eval_error = reconstruction_errors(
    eval_scaled
)

predicted = eval_error > threshold
labels = evaluation["label"].to_numpy()

normal_mask = labels == 0
abnormal_mask = labels == 1

false_positive_rate = float(
    predicted[normal_mask].mean()
)

detection_rate = float(
    predicted[abnormal_mask].mean()
)

print(
    f"INT8 anomaly threshold: {threshold:.6f}"
)

print(
    f"INT8 normal false-positive rate: "
    f"{false_positive_rate:.3%}"
)

print(
    f"INT8 weak-abnormal detection rate: "
    f"{detection_rate:.3%}"
)

abnormal = evaluation.loc[
    abnormal_mask
].copy()

abnormal["detected"] = predicted[
    abnormal_mask
]

print("\nINT8 detection by scenario:")

for scenario, rate in (
    abnormal.groupby("scenario")["detected"]
    .mean()
    .sort_index()
    .items()
):
    print(
        f"  {scenario}: {rate:.3%}"
    )

metadata["int8_anomaly_threshold"] = threshold
metadata["int8_input_scale"] = float(input_scale)
metadata["int8_input_zero_point"] = int(input_zero)
metadata["int8_output_scale"] = float(output_scale)
metadata["int8_output_zero_point"] = int(output_zero)
metadata["int8_model_size_bytes"] = len(tflite_model)
metadata["int8_normal_false_positive_rate"] = false_positive_rate
metadata["int8_weak_abnormal_detection_rate"] = detection_rate

with open(
    ROOT / "training_metadata.json",
    "w"
) as f:
    json.dump(metadata, f, indent=2)

