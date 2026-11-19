#include "event_record.h"

TamperEventRecord EventRecorder::build(
    uint32_t eventId,
    uint32_t recordedAtMs,
    const BehaviorSnapshot &snapshot,
    const DirectCheckState &direct,
    const BehaviorInferenceResult &inference,
    const EventDecision &decision) const {

  TamperEventRecord record;

  record.eventId = eventId;
  record.recordedAtMs = recordedAtMs;
  record.windowSequence = snapshot.sequence;
  record.bootCount = snapshot.bootCount;

  record.severity = decision.severity;

  record.enclosureDirect =
      direct.enclosureOutsideMaintenance;
  record.peripheralDirect =
      direct.peripheralUnavailable;

  record.tinymlValid = inference.valid;
  record.tinymlAbnormal =
      inference.valid && inference.abnormal;

  record.maintenanceActive =
      snapshot.maintenanceActive;

  record.reconstructionError =
      inference.valid ? inference.reconstructionError : 0.0f;

  record.peakMovementG =
      snapshot.peakDynamicAccelerationG;
  record.packetsSent =
      snapshot.packetsSent;
  record.peripheralFailedChecks =
      snapshot.peripheralFailedChecks;
  record.batteryDropV =
      snapshot.batteryValid ? snapshot.batteryDropV : 0.0f;

  record.shouldNotify =
      decision.shouldNotify;
  record.shouldRestrict =
      decision.shouldRestrict;
  record.shouldEnterSafeState =
      decision.shouldEnterSafeState;

  return record;
}
