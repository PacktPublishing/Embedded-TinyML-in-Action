#include <cassert>
#include <iostream>

#include "../firmware/direct_checks.h"

int main() {
  DirectChecks checks(5);

  // Normal operation: cabinet closed and peripheral healthy.
  DirectCheckState state =
      checks.update(false, false, true);

  assert(!state.enclosureOutsideMaintenance);
  assert(!state.peripheralUnavailable);
  assert(state.consecutivePeripheralFailures == 0);
  assert(!state.anyTriggered());

  // Unauthorized enclosure opening.
  state = checks.update(true, false, true);

  assert(state.enclosureOutsideMaintenance);
  assert(!state.peripheralUnavailable);
  assert(state.anyTriggered());

  // The same enclosure opening is permitted during maintenance.
  state = checks.update(true, true, true);

  assert(!state.enclosureOutsideMaintenance);
  assert(!state.peripheralUnavailable);
  assert(!state.anyTriggered());

  // If maintenance ends while the cabinet remains open,
  // the enclosure condition becomes suspicious immediately.
  state = checks.update(true, false, true);

  assert(state.enclosureOutsideMaintenance);
  assert(state.anyTriggered());

  checks.reset();

  // Four consecutive peripheral failures remain below the threshold.
  for (int i = 0; i < 4; ++i) {
    state = checks.update(false, false, false);
    assert(!state.peripheralUnavailable);
  }

  assert(state.consecutivePeripheralFailures == 4);

  // The fifth consecutive failed check triggers the rule.
  state = checks.update(false, false, false);

  assert(state.consecutivePeripheralFailures == 5);
  assert(state.peripheralUnavailable);
  assert(state.anyTriggered());

  // A successful response clears the persistent-failure condition.
  state = checks.update(false, false, true);

  assert(state.consecutivePeripheralFailures == 0);
  assert(!state.peripheralUnavailable);
  assert(!state.anyTriggered());

  std::cout
      << "PASS: direct tamper checks and maintenance context\n";

  return 0;
}
