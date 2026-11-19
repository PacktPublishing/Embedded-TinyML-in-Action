#include <Arduino.h>
#include <Preferences.h>
#include "esp_system.h"

#include "behavior_monitor.h"
#include "behavior_features.h"
#include "behavior_inference.h"
#include "config.h"
#include "event_decision.h"
#include "event_record.h"
#include "telemetry_io.h"

TelemetryIO telemetry;
BehaviorMonitor monitor(
    telemetry,
    Config::PERIPHERAL_FAILURE_LIMIT);

DirectCheckState lastDirectState;
DirectCheckState windowDirectEvidence;
BehaviorInference behaviorInference;
EventDecisionEngine eventDecisionEngine;
EventRecorder eventRecorder;

uint32_t nextEventId = 1;

uint32_t lastTelemetrySampleMs = 0;
uint32_t lastPacketMs = 0;

uint32_t loadAndIncrementBootCount() {
  Preferences preferences;
  if (!preferences.begin("tinyml-ch8", false)) {
    return 1;
  }

  uint32_t bootCount = preferences.getUInt("boot-count", 0) + 1;
  preferences.putUInt("boot-count", bootCount);
  preferences.end();
  return bootCount;
}

void printSnapshot(const BehaviorSnapshot &s) {
  Serial.println();
  Serial.printf("--- Observation Window %lu ---\n",
                static_cast<unsigned long>(s.sequence));
  Serial.printf("Enclosure opened:      %s\n", s.enclosureOpenSeen ? "YES" : "NO");
  Serial.printf("Enclosure transitions: %u\n", s.enclosureTransitions);
  Serial.printf("Maintenance mode:      %s\n", s.maintenanceActive ? "ON" : "OFF");
  Serial.printf("Peak movement:         %.3f g\n", s.peakDynamicAccelerationG);
  Serial.printf("Mean loop time:        %lu us\n",
                static_cast<unsigned long>(s.meanLoopUs));
  Serial.printf("Max loop time:         %lu us\n",
                static_cast<unsigned long>(s.maxLoopUs));
  Serial.printf("Packets sent:          %u\n", s.packetsSent);
  Serial.printf("Peripheral failures:   %u failed checks\n", s.peripheralFailedChecks);
  Serial.printf("Reset reason:          %lu\n",
                static_cast<unsigned long>(s.resetReason));
  Serial.printf("Boot count:            %lu\n",
                static_cast<unsigned long>(s.bootCount));

  if (s.batteryValid) {
    Serial.printf("Battery start:         %.2f V\n", s.batteryStartV);
    Serial.printf("Minimum battery:       %.2f V\n", s.minimumBatteryV);
    Serial.printf("Battery drop:          %.2f V\n", s.batteryDropV);
  } else {
    Serial.println("Battery:               unavailable");
  }
}

const char *severityName(EventSeverity severity) {
  switch (severity) {
    case EventSeverity::Low:
      return "LOW";
    case EventSeverity::Medium:
      return "MEDIUM";
    case EventSeverity::High:
      return "HIGH";
    case EventSeverity::Critical:
      return "CRITICAL";
    case EventSeverity::None:
    default:
      return "NONE";
  }
}

void printEventRecord(const TamperEventRecord &record) {
  Serial.printf("EVENT RECORD #%lu\n",
                static_cast<unsigned long>(record.eventId));
  Serial.printf("  severity:                %s\n",
                severityName(record.severity));
  Serial.printf("  window:                  %lu\n",
                static_cast<unsigned long>(record.windowSequence));
  Serial.printf("  enclosure direct:        %s\n",
                record.enclosureDirect ? "YES" : "NO");
  Serial.printf("  peripheral direct:       %s\n",
                record.peripheralDirect ? "YES" : "NO");
  Serial.printf("  TinyML abnormal:         %s\n",
                record.tinymlAbnormal ? "YES" : "NO");
  Serial.printf("  reconstruction error:    %.4f\n",
                record.reconstructionError);
  Serial.printf("  response notify:         %s\n",
                record.shouldNotify ? "YES" : "NO");
  Serial.printf("  response restrict:       %s\n",
                record.shouldRestrict ? "YES" : "NO");
  Serial.printf("  response safe state:     %s\n",
                record.shouldEnterSafeState ? "YES" : "NO");
}

void accumulateDirectEvidence(const DirectCheckState &state) {
  windowDirectEvidence.enclosureOutsideMaintenance =
      windowDirectEvidence.enclosureOutsideMaintenance ||
      state.enclosureOutsideMaintenance;

  windowDirectEvidence.peripheralUnavailable =
      windowDirectEvidence.peripheralUnavailable ||
      state.peripheralUnavailable;

  if (state.consecutivePeripheralFailures >
      windowDirectEvidence.consecutivePeripheralFailures) {
    windowDirectEvidence.consecutivePeripheralFailures =
        state.consecutivePeripheralFailures;
  }
}

void reportDirectCheckChanges(const DirectCheckState &state) {
  if (state.enclosureOutsideMaintenance &&
      !lastDirectState.enclosureOutsideMaintenance) {
    Serial.println(
        "DIRECT ALERT: enclosure opened outside maintenance");
  } else if (!state.enclosureOutsideMaintenance &&
             lastDirectState.enclosureOutsideMaintenance) {
    Serial.println(
        "DIRECT CLEAR: enclosure condition cleared");
  }

  if (state.peripheralUnavailable &&
      !lastDirectState.peripheralUnavailable) {
    Serial.printf(
        "DIRECT ALERT: peripheral unavailable after %u consecutive failed checks\n",
        state.consecutivePeripheralFailures);
  } else if (!state.peripheralUnavailable &&
             lastDirectState.peripheralUnavailable) {
    Serial.println(
        "DIRECT CLEAR: peripheral communication restored");
  }

  lastDirectState = state;
}

void simulateCommunication(uint32_t nowMs) {
#ifdef WOKWI_SIMULATION
  const uint32_t interval = telemetry.communicationBurstEnabled()
                                ? Config::BURST_PACKET_INTERVAL_MS
                                : Config::NORMAL_PACKET_INTERVAL_MS;

  if (static_cast<uint32_t>(nowMs - lastPacketMs) >= interval) {
    lastPacketMs = nowMs;
    monitor.notePacketSent();
  }
#else
  // On a physical application, call monitor.notePacketSent() immediately
  // after each successful packet transmission instead of using this helper.
  (void)nowMs;
#endif
}

void setup() {
  Serial.begin(Config::SERIAL_BAUD);
  delay(250);

  const bool movementSensorReady = telemetry.begin();
  if (!movementSensorReady) {
    Serial.println("WARNING: MPU6050 was not detected. Movement samples will be unavailable.");
  }

  const uint32_t bootCount = loadAndIncrementBootCount();
  const uint32_t resetReason = static_cast<uint32_t>(esp_reset_reason());
  monitor.begin(resetReason, bootCount);

  if (behaviorInference.begin()) {
    Serial.println("TinyML inference ready.");
  } else {
    Serial.println("ERROR: TinyML inference initialization failed.");
  }

  lastTelemetrySampleMs = millis();
  lastPacketMs = millis();

  Serial.println("Chapter 8 behavior-monitoring node ready.");
  Serial.println("Observation window: 5 seconds");
}

void loop() {
  const uint32_t loopStartUs = micros();
  const uint32_t nowMs = millis();

  if (static_cast<uint32_t>(nowMs - lastTelemetrySampleMs) >=
      Config::TELEMETRY_SAMPLE_MS) {
    lastTelemetrySampleMs = nowMs;

    const DirectCheckState directState =
        monitor.sampleTelemetry();

    reportDirectCheckChanges(directState);
    accumulateDirectEvidence(directState);
  }

  simulateCommunication(nowMs);

  monitor.recordLoopExecution(micros() - loopStartUs);

  if (monitor.observationReady(nowMs)) {
    const BehaviorSnapshot snapshot =
        monitor.finishObservationWindow(nowMs);

    printSnapshot(snapshot);

    const BehaviorFeatures features =
        BehaviorFeatures::fromSnapshot(
            snapshot,
            Config::BASELINE_MEAN_LOOP_US,
            Config::BASELINE_MAX_LOOP_US,
            Config::BASELINE_PACKETS_PER_WINDOW,
            Config::EXPECTED_TELEMETRY_SAMPLES);

    const BehaviorInferenceResult inference =
        behaviorInference.run(features);

    if (inference.valid) {
      Serial.printf(
          "TinyML reconstruction error: %.4f\n",
          inference.reconstructionError);

      Serial.printf(
          "TinyML classification:       %s\n",
          inference.abnormal ? "ABNORMAL" : "NORMAL");

      Serial.printf(
          "TinyML inference time:       %lu us\n",
          static_cast<unsigned long>(
              inference.inferenceUs));
    } else {
      Serial.println(
          "TinyML inference:             unavailable");
    }

    const EventDecision decision =
        eventDecisionEngine.evaluate(
            windowDirectEvidence,
            inference.valid,
            inference.abnormal,
            snapshot.maintenanceActive);

    Serial.printf(
        "Event severity:             %s\n",
        severityName(decision.severity));

    if (decision.shouldRecord) {
      const TamperEventRecord record =
          eventRecorder.build(
              nextEventId++,
              nowMs,
              snapshot,
              windowDirectEvidence,
              inference,
              decision);

      printEventRecord(record);
    }

    windowDirectEvidence = DirectCheckState{};
  }
}
