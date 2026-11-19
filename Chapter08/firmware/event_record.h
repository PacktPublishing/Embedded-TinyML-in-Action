#pragma once

#include <stdint.h>

#include "behavior_inference.h"
#include "behavior_snapshot.h"
#include "direct_checks.h"
#include "event_decision.h"

struct TamperEventRecord {
  uint32_t eventId = 0;
  uint32_t recordedAtMs = 0;
  uint32_t windowSequence = 0;
  uint32_t bootCount = 0;

  EventSeverity severity = EventSeverity::None;

  bool enclosureDirect = false;
  bool peripheralDirect = false;
  bool tinymlValid = false;
  bool tinymlAbnormal = false;
  bool maintenanceActive = false;

  float reconstructionError = 0.0f;
  float peakMovementG = 0.0f;
  uint16_t packetsSent = 0;
  uint16_t peripheralFailedChecks = 0;
  float batteryDropV = 0.0f;

  bool shouldNotify = false;
  bool shouldRestrict = false;
  bool shouldEnterSafeState = false;
};

class EventRecorder {
 public:
  TamperEventRecord build(
      uint32_t eventId,
      uint32_t recordedAtMs,
      const BehaviorSnapshot &snapshot,
      const DirectCheckState &direct,
      const BehaviorInferenceResult &inference,
      const EventDecision &decision) const;
};
