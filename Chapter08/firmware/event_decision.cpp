#include "event_decision.h"

EventDecision EventDecisionEngine::evaluate(
    const DirectCheckState &direct,
    bool tinymlValid,
    bool tinymlAbnormal,
    bool maintenanceActive) const {

  EventDecision decision;

  decision.directTriggered = direct.anyTriggered();
  decision.tinymlAbnormal =
      tinymlValid && tinymlAbnormal;
  decision.maintenanceActive =
      maintenanceActive;

  // No evidence of abnormal behavior.
  if (!decision.directTriggered &&
      !decision.tinymlAbnormal) {
    return decision;
  }

  // Maintenance can explain unusual behavior that is not
  // supported by an immediate direct tamper condition.
  if (maintenanceActive &&
      !decision.directTriggered &&
      decision.tinymlAbnormal) {
    decision.severity = EventSeverity::Low;
    decision.shouldRecord = true;
    return decision;
  }

  // A weak combined pattern detected only by TinyML.
  if (!decision.directTriggered &&
      decision.tinymlAbnormal) {
    decision.severity = EventSeverity::Medium;
    decision.shouldRecord = true;
    decision.shouldNotify = true;
    return decision;
  }

  // A clear direct tamper condition.
  if (decision.directTriggered &&
      !decision.tinymlAbnormal) {
    decision.severity = EventSeverity::High;
    decision.shouldRecord = true;
    decision.shouldNotify = true;
    decision.shouldRestrict = true;
    return decision;
  }

  // Independent direct and TinyML evidence agree.
  decision.severity = EventSeverity::Critical;
  decision.shouldRecord = true;
  decision.shouldNotify = true;
  decision.shouldRestrict = true;
  decision.shouldEnterSafeState = true;

  return decision;
}
