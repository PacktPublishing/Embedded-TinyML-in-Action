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
