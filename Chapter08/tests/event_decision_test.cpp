#include <cassert>
#include <iostream>

#include "../firmware/event_decision.h"

int main() {
  EventDecisionEngine engine;
  DirectCheckState clear;

  // 1. Normal behavior: no event.
  {
    const EventDecision d =
        engine.evaluate(clear, true, false, false);

    assert(d.severity == EventSeverity::None);
    assert(!d.shouldRecord);
    assert(!d.shouldNotify);
    assert(!d.shouldRestrict);
    assert(!d.shouldEnterSafeState);
  }

  // 2. TinyML anomaly during authorized maintenance: record only.
  {
    const EventDecision d =
        engine.evaluate(clear, true, true, true);

    assert(d.severity == EventSeverity::Low);
    assert(d.shouldRecord);
    assert(!d.shouldNotify);
    assert(!d.shouldRestrict);
    assert(!d.shouldEnterSafeState);
  }

  // 3. TinyML-only anomaly outside maintenance: suspicious.
  {
    const EventDecision d =
        engine.evaluate(clear, true, true, false);

    assert(d.severity == EventSeverity::Medium);
    assert(d.shouldRecord);
    assert(d.shouldNotify);
    assert(!d.shouldRestrict);
    assert(!d.shouldEnterSafeState);
  }

  // 4. Clear direct tamper condition: high severity.
  {
    DirectCheckState direct;
    direct.enclosureOutsideMaintenance = true;

    const EventDecision d =
        engine.evaluate(direct, true, false, false);

    assert(d.severity == EventSeverity::High);
    assert(d.shouldRecord);
    assert(d.shouldNotify);
    assert(d.shouldRestrict);
    assert(!d.shouldEnterSafeState);
  }

  // 5. Direct tamper plus TinyML anomaly: critical.
  {
    DirectCheckState direct;
    direct.peripheralUnavailable = true;

    const EventDecision d =
        engine.evaluate(direct, true, true, false);

    assert(d.severity == EventSeverity::Critical);
    assert(d.shouldRecord);
    assert(d.shouldNotify);
    assert(d.shouldRestrict);
    assert(d.shouldEnterSafeState);
  }

  // 6. Invalid TinyML output must not create an anomaly event.
  {
    const EventDecision d =
        engine.evaluate(clear, false, true, false);

    assert(d.severity == EventSeverity::None);
    assert(!d.shouldRecord);
  }

  std::cout
      << "PASS: context-aware event severity decisions\n";

  return 0;
}
