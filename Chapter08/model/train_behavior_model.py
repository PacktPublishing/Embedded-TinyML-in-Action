from pathlib import Path
import json

import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

SEED = 42

FEATURES = [
    "peak_movement_g",
    "mean_loop_ratio",
    "max_loop_ratio",
    "packet_count_ratio",
    "peripheral_failure_fraction",
    "battery_drop_v",
    "enclosure_transitions",
]

np.random.seed(SEED)
tf.keras.utils.set_random_seed(SEED)

ROOT = Path(__file__).parent


def reconstruction_error(model, x):
    reconstructed = model.predict(x, verbose=0)
    return np.mean(np.square(x - reconstructed), axis=1)


def main():
    training = pd.read_csv(ROOT / "training_data.csv")
    evaluation = pd.read_csv(ROOT / "evaluation_data.csv")

    x = training[FEATURES].to_numpy(dtype=np.float32)

    x_train, x_validation = train_test_split(
        x,
        test_size=0.20,
        random_state=SEED,
        shuffle=True,
    )

    scaler = StandardScaler()
    x_train_scaled = scaler.fit_transform(x_train).astype(np.float32)
    x_validation_scaled = scaler.transform(x_validation).astype(np.float32)

    # Tiny autoencoder: 7 -> 5 -> 3 -> 5 -> 7
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(len(FEATURES),)),
        tf.keras.layers.Dense(5, activation="relu"),
        tf.keras.layers.Dense(3, activation="relu"),
        tf.keras.layers.Dense(5, activation="relu"),
        tf.keras.layers.Dense(len(FEATURES)),
    ])

    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
        loss="mse",
    )

    early_stop = tf.keras.callbacks.EarlyStopping(
        monitor="val_loss",
        patience=10,
        restore_best_weights=True,
    )

    history = model.fit(
        x_train_scaled,
        x_train_scaled,
        validation_data=(x_validation_scaled, x_validation_scaled),
        epochs=100,
        batch_size=64,
        callbacks=[early_stop],
        verbose=0,
    )

    # Threshold comes only from held-out normal behavior.
    validation_error = reconstruction_error(
        model,
        x_validation_scaled,
    )

    threshold = float(
        np.percentile(validation_error, 99.0)
    )

    eval_x = evaluation[FEATURES].to_numpy(dtype=np.float32)
    eval_scaled = scaler.transform(eval_x).astype(np.float32)

    eval_error = reconstruction_error(model, eval_scaled)
    predicted_abnormal = eval_error > threshold

    labels = evaluation["label"].to_numpy()

    normal_mask = labels == 0
    abnormal_mask = labels == 1

    false_positive_rate = float(
        predicted_abnormal[normal_mask].mean()
    )

    detection_rate = float(
        predicted_abnormal[abnormal_mask].mean()
    )

    print("Model architecture: 7 -> 5 -> 3 -> 5 -> 7")
    print("Trainable parameters:", model.count_params())
    print("Epochs completed:", len(history.history["loss"]))
    print(f"Final training loss:   {history.history['loss'][-1]:.6f}")
    print(f"Final validation loss: {history.history['val_loss'][-1]:.6f}")
    print(f"Anomaly threshold:     {threshold:.6f}")
    print(f"Normal false-positive rate: {false_positive_rate:.3%}")
    print(f"Weak-abnormal detection rate: {detection_rate:.3%}")

    print("\nDetection by abnormal scenario:")

    abnormal_eval = evaluation.loc[abnormal_mask].copy()
    abnormal_eval["detected"] = predicted_abnormal[abnormal_mask]

    scenario_rates = (
        abnormal_eval.groupby("scenario")["detected"]
        .mean()
        .sort_index()
    )

    for scenario, rate in scenario_rates.items():
        print(f"  {scenario}: {rate:.3%}")

    model.save(ROOT / "behavior_autoencoder.keras")

    metadata = {
        "seed": SEED,
        "features": FEATURES,
        "scaler_mean": scaler.mean_.tolist(),
        "scaler_scale": scaler.scale_.tolist(),
        "anomaly_threshold": threshold,
        "model_parameters": int(model.count_params()),
        "normal_false_positive_rate": false_positive_rate,
        "weak_abnormal_detection_rate": detection_rate,
    }

    with open(ROOT / "training_metadata.json", "w") as f:
        json.dump(metadata, f, indent=2)

    scored = evaluation.copy()
    scored["reconstruction_error"] = eval_error
    scored["predicted_abnormal"] = predicted_abnormal.astype(int)
    scored.to_csv(
        ROOT / "evaluation_scores.csv",
        index=False,
    )

    print("\nSaved:")
    print("  behavior_autoencoder.keras")
    print("  training_metadata.json")
    print("  evaluation_scores.csv")


if __name__ == "__main__":
    main()
