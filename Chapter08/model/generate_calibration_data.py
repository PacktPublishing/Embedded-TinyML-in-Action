from pathlib import Path
import numpy as np
import pandas as pd

SEED = 123
rng = np.random.default_rng(SEED)

ROOT = Path(__file__).parent


def clipped_normal(mean, std, low, high, n):
    return np.clip(rng.normal(mean, std, n), low, high)


def normal(n):
    return pd.DataFrame({
        "peak_movement_g": clipped_normal(0.025, 0.015, 0.0, 0.09, n),
        "mean_loop_ratio": clipped_normal(1.00, 0.05, 0.82, 1.18, n),
        "max_loop_ratio": clipped_normal(1.00, 0.12, 0.65, 1.40, n),
        "packet_count_ratio": clipped_normal(1.00, 0.07, 0.78, 1.22, n),
        "peripheral_failure_fraction": np.zeros(n),
        "battery_drop_v": np.clip(rng.exponential(0.006, n), 0.0, 0.035),
        "enclosure_transitions": np.zeros(n),
    })


def broad_behavior(n):
    return pd.DataFrame({
        "peak_movement_g": rng.uniform(0.0, 0.50, n),
        "mean_loop_ratio": rng.uniform(0.75, 1.70, n),
        "max_loop_ratio": rng.uniform(0.60, 2.20, n),
        "packet_count_ratio": rng.uniform(0.70, 2.00, n),
        "peripheral_failure_fraction": rng.uniform(0.0, 0.12, n),
        "battery_drop_v": rng.uniform(0.0, 0.20, n),
        "enclosure_transitions": rng.integers(0, 2, n).astype(float),
    })


calibration = pd.concat(
    [normal(500), broad_behavior(500)],
    ignore_index=True
)

calibration.to_csv(
    ROOT / "calibration_data.csv",
    index=False
)

print("Calibration windows:", len(calibration))
