# Chapter 8: Behavior-Aware Tamper Monitoring Node

This repository milestone supports Section 8.4 of *Embedded TinyML in Action*.
It implements the telemetry layer of a cabinet-mounted ESP32 behavior-monitoring node.

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

This milestone intentionally stops before the Chapter 8 rule engine and TinyML inference. Those are added in the next stage so the telemetry path can be verified first.

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
