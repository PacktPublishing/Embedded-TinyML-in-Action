#include <cassert>
#include <cmath>
#include <iostream>

#include "../firmware/event_record.h"

bool near(float a, float b, float tolerance = 0.001f) {
  return std::fabs(a - b) <= tolerance;
}

int main() {
  BehaviorSnapshot snapshot;
  snapshot.sequence = 7;
  snapshot.bootCount = 3;
  snapshot.maintenanceActive = false;
  snapshot.peakDynamicAccelerationG = 0.193f;
  snapshot.packetsSent = 8;
  snapshot.peripheralFailedChecks = 0;
  snapshot.batteryValid = true;
  snapshot.batteryDropV = 0.04f;

  DirectCheckState direct;
  direct.enclosureOutsideMaintenance = true;
  direct.peripheralUnavailable = false;

  BehaviorInferenceResult inference;
  inference.valid = true;
  inference.abnormal = true;
  inference.reconstructionError = 27.2212f;

  EventDecision decision;
  decision.severity = EventSeverity::Critical;
  decision.directTriggered = true;
  decision.tinymlAbnormal = true;
  decision.maintenanceActive = false;
  decision.shouldRecord = true;
  decision.shouldNotify = true;
  decision.shouldRestrict = true;
  decision.shouldEnterSafeState = true;

  EventRecorder recorder;

  const TamperEventRecord record =
      recorder.build(
          12,
          5500,
          snapshot,
          direct,
          inference,
          decision);

  assert(record.eventId == 12);
  assert(record.recordedAtMs == 5500);
  assert(record.windowSequence == 7);
  assert(record.bootCount == 3);

  assert(record.severity == EventSeverity::Critical);
  assert(record.enclosureDirect);
  assert(!record.peripheralDirect);
  assert(record.tinymlValid);
  assert(record.tinymlAbnormal);
  assert(!record.maintenanceActive);

  assert(near(record.reconstructionError, 27.2212f));
  assert(near(record.peakMovementG, 0.193f));
  assert(record.packetsSent == 8);
  assert(record.peripheralFailedChecks == 0);
  assert(near(record.batteryDropV, 0.04f));

  assert(record.shouldNotify);
  assert(record.shouldRestrict);
  assert(record.shouldEnterSafeState);

  std::cout
      << "PASS: tamper event evidence record construction\n";

  return 0;
}
