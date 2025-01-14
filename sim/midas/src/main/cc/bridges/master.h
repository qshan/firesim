// See LICENSE for license details.

#ifndef __MASTER_H
#define __MASTER_H

#include "core/widget.h"

#include <cstdint>

class simif_t;

struct SIMULATIONMASTER_struct {
  uint64_t STEP;
  uint64_t DONE;
  uint64_t INIT_DONE;
};

class master_t final : public widget_t {
public:
  /// The identifier for the bridge type.
  static char KIND;

  master_t(simif_t &simif, const SIMULATIONMASTER_struct &mmio_addrs);

  /**
   * Check whether the device is initialised.
   */
  bool is_init_done();

  /**
   * Check whether the simulation is complete.
   */
  bool is_done();

  /**
   * Request the simulation to advance a given number of steps.
   */
  void step(size_t n, bool blocking);

private:
  const SIMULATIONMASTER_struct mmio_addrs;
};

#endif // __MASTER_H
