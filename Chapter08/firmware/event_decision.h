#pragma once

#include "direct_checks.h"

enum class EventSeverity {
  None = 0,
  Low,
  Medium,
  High,
  Critical
};

struct EventDecision {
  EventSeverity severity = EventSeverity::None;

  bool directTriggered = false;
  bool tinymlAbnormal = false;
  bool maintenanceActive = false;

  bool shouldRecord = false;
  bool shouldNotify = false;
  bool shouldRestrict = false;
  bool shouldEnterSafeState = false;
};

class EventDecisionEngine {
 public:
  EventDecision evaluate(
      const DirectCheckState &direct,
      bool tinymlValid,
      bool tinymlAbnormal,
      bool maintenanceActive) const;
};
