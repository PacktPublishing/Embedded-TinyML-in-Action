#pragma once

#include <Arduino.h>

namespace Config {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t OBSERVATION_MS = 5000;
constexpr uint32_t TELEMETRY_SAMPLE_MS = 100;

// Direct-check threshold. Five consecutive failed checks at the current
// 100 ms sampling interval represents approximately 500 ms of unavailability.
constexpr uint16_t PERIPHERAL_FAILURE_LIMIT = 5;

constexpr uint8_t PIN_ENCLOSURE = 27;
constexpr uint8_t PIN_MAINTENANCE = 26;
constexpr uint8_t PIN_COMM_BURST = 25;       // Simulation-only fault injection
constexpr uint8_t PIN_PERIPHERAL_FAULT = 33; // Simulation-only fault injection
constexpr uint8_t PIN_BATTERY_ADC = 34;
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr float STANDARD_GRAVITY = 9.80665f;

constexpr uint32_t NORMAL_PACKET_INTERVAL_MS = 1000;
constexpr uint32_t BURST_PACKET_INTERVAL_MS = 200;

// Wokwi maps the potentiometer position to an equivalent battery voltage.
constexpr float SIM_BATTERY_MIN_V = 3.30f;
constexpr float SIM_BATTERY_MAX_V = 4.20f;

// Physical hardware path. Adjust to match the actual divider and calibration.
constexpr float ADC_REFERENCE_V = 3.30f;
constexpr float BATTERY_DIVIDER_RATIO = 2.00f;
}
