from pathlib import Path

import numpy as np
import pandas as pd

SEED = 42
rng = np.random.default_rng(SEED)

FEATURES = [
    "peak_movement_g",
    "mean_loop_ratio",
    "max_loop_ratio",
    "packet_count_ratio",
    "peripheral_failure_fraction",
    "battery_drop_v",
    "enclosure_transitions",
]


def clipped_normal(mean, std, low, high, n):
    return np.clip(rng.normal(mean, std, n), low, high)


def make_normal(n):
    data = pd.DataFrame({
        "peak_movement_g":
            clipped_normal(0.025, 0.015, 0.0, 0.09, n),

        "mean_loop_ratio":
            clipped_normal(1.00, 0.05, 0.82, 1.18, n),

        "max_loop_ratio":
            clipped_normal(1.00, 0.12, 0.65, 1.40, n),

        "packet_count_ratio":
            clipped_normal(1.00, 0.07, 0.78, 1.22, n),

        "peripheral_failure_fraction":
            np.zeros(n),

        "battery_drop_v":
            np.clip(rng.exponential(0.006, n), 0.0, 0.035),

        "enclosure_transitions":
            np.zeros(n),
    })

    # Normal operation may occasionally contain one transient failed check.
    transient = rng.random(n) < 0.05
    data.loc[transient, "peripheral_failure_fraction"] = 0.02

    # A rare enclosure transition can occur during legitimate maintenance.
    maintenance = rng.random(n) < 0.02
    data.loc[maintenance, "enclosure_transitions"] = 1.0

    return data


def make_weak_abnormal(n):
    data = make_normal(n)
    scenario = np.empty(n, dtype=object)

    for i in range(n):
        case = i % 5

        if case == 0:
            data.loc[i, "peak_movement_g"] = rng.uniform(0.10, 0.30)
            data.loc[i, "packet_count_ratio"] = rng.uniform(1.25, 1.75)
            scenario[i] = "movement_plus_comm_change"

        elif case == 1:
            data.loc[i, "mean_loop_ratio"] = rng.uniform(1.20, 1.55)
            data.loc[i, "max_loop_ratio"] = rng.uniform(1.40, 2.00)
            scenario[i] = "runtime_timing_shift"

        elif case == 2:
            data.loc[i, "packet_count_ratio"] = rng.uniform(1.30, 1.90)
            data.loc[i, "battery_drop_v"] = rng.uniform(0.04, 0.14)
            scenario[i] = "comm_plus_battery_change"

        elif case == 3:
            data.loc[i, "peripheral_failure_fraction"] = rng.uniform(0.02, 0.08)
            data.loc[i, "peak_movement_g"] = rng.uniform(0.08, 0.20)
            scenario[i] = "weak_peripheral_plus_movement"

        else:
            data.loc[i, "battery_drop_v"] = rng.uniform(0.05, 0.18)
            data.loc[i, "mean_loop_ratio"] = rng.uniform(1.15, 1.40)
            scenario[i] = "battery_plus_runtime_change"

    data["scenario"] = scenario
    return data


def main():
    output = Path(__file__).parent

    training = make_normal(6000)
    training.to_csv(output / "training_data.csv", index=False)

    normal_eval = make_normal(1500)
    normal_eval["label"] = 0
    normal_eval["scenario"] = "normal"

    abnormal_eval = make_weak_abnormal(1500)
    abnormal_eval["label"] = 1

    evaluation = pd.concat(
        [normal_eval, abnormal_eval],
        ignore_index=True
    )

    evaluation.to_csv(
        output / "evaluation_data.csv",
        index=False
    )

    print(f"Seed: {SEED}")
    print(f"Training normal windows: {len(training)}")
    print(f"Evaluation normal windows: {len(normal_eval)}")
    print(f"Evaluation weak-abnormal windows: {len(abnormal_eval)}")
    print("\nTraining feature means:")
    print(training[FEATURES].mean().round(4))


if __name__ == "__main__":
    main()
