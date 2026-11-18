# Chapter 8: Behavior-Aware Tamper Monitoring Node

This project supports Sections 8.3 and 8.4 of *Embedded TinyML in Action*.
It implements the telemetry and direct-check layers of a cabinet-mounted ESP32 behavior-monitoring node.

## What this milestone does

The firmware collects and summarizes:

- enclosure state and transitions
- maintenance mode
- dynamic movement from an MPU6050
- peripheral-health checks
- packet activity
- ESP32 reset reason and persistent boot count
- main-loop execution time
- simulated battery voltage

Every five seconds it creates and prints one `BehaviorSnapshot`. The observation state is then cleared so the next window is independent.

The project now includes a direct-check engine for conditions that should not wait for TinyML inference. It detects an enclosure opening outside authorized maintenance and persistent peripheral unavailability. TinyML behavior inference is added in the next milestone.

## Target

- ESP32 DevKit / ESP32-DevKitC
- Arduino framework
- Serial monitor: 115200 baud
- Wokwi simulation supported

## Simulation controls

| Control | Meaning |
|---|---|
| Enclosure switch | LOW = closed, HIGH = open |
| Maintenance switch | LOW = normal operation, HIGH = maintenance mode |
| Peripheral-fault switch | LOW = MPU6050 responds normally, HIGH = inject failed peripheral checks |
| Communication-burst switch | LOW = one packet per second, HIGH = one packet every 200 ms |
| MPU6050 | Adjust acceleration to create movement |
| Potentiometer | Maps from 3.30 V to 4.20 V equivalent battery voltage |

The peripheral-fault and communication-burst switches are simulation controls. In a physical deployment, peripheral health comes from the real interface and `monitor.notePacketSent()` is called after actual successful transmissions.

## Build with PlatformIO

```bash
pio run
```

The project pins PlatformIO's ESP32 platform and the Adafruit MPU6050 library in `platformio.ini` for reproducible builds.

## Run in Wokwi

The repository includes `diagram.json` and `wokwi.toml`. After building the firmware, start the simulator with the Wokwi VS Code extension or Wokwi CLI.

Expected startup output:

```text
Chapter 8 behavior-monitoring node ready.
Observation window: 5 seconds
```

Every five seconds the serial monitor prints a snapshot.

## Host-side logic test

The accumulator logic is independent of Arduino and can be tested with a normal C++ compiler:

```bash
g++ -std=c++17 tests/behavior_accumulator_test.cpp \
    firmware/behavior_accumulator.cpp \
    -I firmware -o behavior_test
./behavior_test
```

Expected result:

```text
PASS: behavior accumulator normal and disturbance windows
```

## Physical hardware note

The physical battery input requires an appropriate voltage divider and calibration for the selected battery. The values in `config.h` are explicit defaults, not universal hardware constants. Do not connect a battery voltage above the ESP32 ADC input limit directly to GPIO34.


## Direct tamper checks

Direct checks operate on raw telemetry every 100 ms rather than waiting for the five-second observation window.

The current rules are:

- enclosure open while maintenance mode is inactive
- peripheral unavailable for five consecutive checks

Opening the enclosure during authorized maintenance does not trigger the enclosure rule. If maintenance ends while the enclosure remains open, the direct alert is raised immediately.

The five-check peripheral threshold corresponds to approximately 500 ms at the current sampling interval. It is a demonstration threshold and should be tuned for the deployed hardware and application.

The automated Wokwi scenario is available at `scenarios/direct_checks.yaml`.

It verifies unauthorized enclosure access, authorized maintenance context, persistent peripheral failure, and recovery.


## TinyML behavior inference

The Chapter 8 project combines immediate tamper checks with a lightweight TinyML anomaly detector.

The direct-check path evaluates clear conditions such as an enclosure opening outside maintenance mode and sustained peripheral communication loss. These checks operate directly on telemetry and do not wait for the observation window.

The TinyML path evaluates weaker combinations of behavior over each five-second observation window. Seven compact features are used:

- peak dynamic movement
- mean loop-time ratio
- maximum loop-time ratio
- packet-count ratio
- peripheral failure fraction
- battery-voltage drop
- enclosure transitions

A small 7-5-3-5-7 autoencoder is trained using controlled synthetic normal behavior. Reconstruction error is used as the anomaly measure. The training and evaluation data are teaching datasets and should not be interpreted as field-collected tamper data.

The final model is fully INT8 quantized and occupies approximately 3.4 KB. In the controlled evaluation dataset, the quantized model produced a 0.533% normal false-positive rate and detected 99.333% of the generated weak-abnormal windows.

Wokwi validation demonstrates both branches of the hybrid design. Clear disturbances trigger the direct checks immediately, while a weaker movement-plus-communication pattern is classified as abnormal by TinyML without triggering either direct rule.

The TinyML-enabled ESP32 build uses approximately 30,856 bytes of RAM and 356,081 bytes of flash. Observed Wokwi inference latency was approximately 340-492 microseconds per five-second observation window.

The Wokwi commissioning baseline is specific to the simulation. Physical deployments must establish their own baseline during authorized commissioning.

### Model workflow

Generate the controlled datasets:

```bash
python model/generate_behavior_data.py
python model/generate_calibration_data.py
```

Train the autoencoder:

```bash
python model/train_behavior_model.py
```

Convert and validate the INT8 model:

```bash
python model/convert_behavior_model.py
```

Generate the embedded model header:

```bash
python model/export_model_header.py
```

The final physical ESP32 validation is performed after the complete Chapter 8 detection and response pipeline has been integrated.
