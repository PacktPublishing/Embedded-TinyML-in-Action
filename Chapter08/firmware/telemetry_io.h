#pragma once

#include <Adafruit_MPU6050.h>
#include <Arduino.h>

class TelemetryIO {
 public:
  bool begin();

  bool enclosureOpen() const;
  bool maintenanceActive() const;
  bool communicationBurstEnabled() const;

  bool peripheralResponding();
  bool readMovementDynamicG(float &dynamicAccelerationG);
  bool readBatteryVoltage(float &voltage);

 private:
  Adafruit_MPU6050 mpu_;
  bool mpuReady_ = false;
};
