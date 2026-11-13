#include "telemetry_io.h"

#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <cmath>

#include "config.h"

bool TelemetryIO::begin() {
  pinMode(Config::PIN_ENCLOSURE, INPUT_PULLDOWN);
  pinMode(Config::PIN_MAINTENANCE, INPUT_PULLDOWN);
  pinMode(Config::PIN_BATTERY_ADC, INPUT);

#ifdef WOKWI_SIMULATION
  pinMode(Config::PIN_COMM_BURST, INPUT_PULLDOWN);
  pinMode(Config::PIN_PERIPHERAL_FAULT, INPUT_PULLDOWN);
#endif

  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);
  mpuReady_ = mpu_.begin(Config::MPU6050_ADDRESS, &Wire);

  if (mpuReady_) {
    mpu_.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu_.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  return mpuReady_;
}

bool TelemetryIO::enclosureOpen() const {
  return digitalRead(Config::PIN_ENCLOSURE) == HIGH;
}

bool TelemetryIO::maintenanceActive() const {
  return digitalRead(Config::PIN_MAINTENANCE) == HIGH;
}

bool TelemetryIO::communicationBurstEnabled() const {
#ifdef WOKWI_SIMULATION
  return digitalRead(Config::PIN_COMM_BURST) == HIGH;
#else
  return false;
#endif
}

bool TelemetryIO::peripheralResponding() {
#ifdef WOKWI_SIMULATION
  if (digitalRead(Config::PIN_PERIPHERAL_FAULT) == HIGH) {
    return false;
  }
#endif

  Wire.beginTransmission(Config::MPU6050_ADDRESS);
  return Wire.endTransmission() == 0;
}

bool TelemetryIO::readMovementDynamicG(float &dynamicAccelerationG) {
  if (!mpuReady_ || !peripheralResponding()) {
    return false;
  }

  sensors_event_t acceleration;
  sensors_event_t gyro;
  sensors_event_t temperature;
  mpu_.getEvent(&acceleration, &gyro, &temperature);

  const float magnitude = std::sqrt(
      acceleration.acceleration.x * acceleration.acceleration.x +
      acceleration.acceleration.y * acceleration.acceleration.y +
      acceleration.acceleration.z * acceleration.acceleration.z);

  dynamicAccelerationG =
      std::fabs((magnitude / Config::STANDARD_GRAVITY) - 1.0f);
  return true;
}

bool TelemetryIO::readBatteryVoltage(float &voltage) {
  const int raw = analogRead(Config::PIN_BATTERY_ADC);
  if (raw < 0) {
    return false;
  }

#ifdef WOKWI_SIMULATION
  const float normalized = static_cast<float>(raw) / 4095.0f;
  voltage = Config::SIM_BATTERY_MIN_V +
            normalized * (Config::SIM_BATTERY_MAX_V - Config::SIM_BATTERY_MIN_V);
#else
  const float adcVoltage =
      (static_cast<float>(raw) / 4095.0f) * Config::ADC_REFERENCE_V;
  voltage = adcVoltage * Config::BATTERY_DIVIDER_RATIO;
#endif

  return true;
}
