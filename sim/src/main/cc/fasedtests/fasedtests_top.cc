// See LICENSE for license details.

#include "bridges/fased_memory_timing_model.h"
#include "bridges/fpga_model.h"
#include "bridges/peek_poke.h"
#include "bridges/reset_pulse.h"
#include "core/bridge_driver.h"
#include "core/simulation.h"
#include "core/systematic_scheduler.h"
#include "test_harness_bridge.h"

class fasedtests_top_t : public systematic_scheduler_t, public simulation_t {
public:
  fasedtests_top_t(const std::vector<std::string> &args, simif_t &sim);
  ~fasedtests_top_t() override = default;

  void simulation_init() override;
  void simulation_finish() override;
  int simulation_run() override;

private:
  // profile interval: # of cycles to advance before profiling instrumentation
  // registers in models
  uint64_t profile_interval = -1;
  uint64_t profile_models();

  // Returns true if any bridge has signaled for simulation termination
  bool simulation_complete();
  // Returns the error code of the first bridge for which it is non-zero
  int exit_code();
};

fasedtests_top_t::fasedtests_top_t(const std::vector<std::string> &args,
                                   simif_t &sim)
    : simulation_t(sim, args) {
  max_cycles = -1;
  profile_interval = max_cycles;

  for (auto &arg : args) {
    if (arg.find("+max-cycles=") == 0) {
      max_cycles = atoi(arg.c_str() + 12);
    }
    if (arg.find("+profile-interval=") == 0) {
      profile_interval = atoi(arg.c_str() + 18);
    }
  }

  auto *model = sim.get_registry().get_all_models()[0];
  sim.get_registry().add_widget(
      new test_harness_bridge_t(sim,
                                sim.get_registry().get_widget<peek_poke_t>(),
                                model->get_addr_map(),
                                args));
}

bool fasedtests_top_t::simulation_complete() {
  bool is_complete = false;
  for (auto &e : sim.get_registry().get_all_bridges()) {
    is_complete |= e->terminate();
  }
  return is_complete;
}

uint64_t fasedtests_top_t::profile_models() {
  for (auto &mod : sim.get_registry().get_all_models()) {
    mod->profile();
  }
  return profile_interval;
}

int fasedtests_top_t::exit_code() {
  for (auto &e : sim.get_registry().get_all_bridges()) {
    if (e->exit_code())
      return e->exit_code();
  }
  return 0;
}

void fasedtests_top_t::simulation_init() {
  // Add functions you'd like to periodically invoke on a paused simulator here.
  if (profile_interval != -1) {
    register_task([this]() { return this->profile_models(); }, 0);
  }

  for (auto *bridge : sim.get_registry().get_all_bridges()) {
    bridge->init();
  }
  for (auto *model : sim.get_registry().get_all_models()) {
    model->init();
  }
}

int fasedtests_top_t::simulation_run() {
  while (!simulation_complete() && !finished_scheduled_tasks()) {
    run_scheduled_tasks();
    sim.take_steps(get_largest_stepsize(), false);
    while (!sim.done() && !simulation_complete()) {
      for (auto &e : sim.get_registry().get_all_bridges())
        e->tick();
    }
  }

  return exit_code();
}

void fasedtests_top_t::simulation_finish() {
  for (auto *bridge : sim.get_registry().get_all_bridges()) {
    bridge->finish();
  }
  for (auto *model : sim.get_registry().get_all_models()) {
    model->finish();
  }
}

std::unique_ptr<simulation_t>
create_simulation(const std::vector<std::string> &args, simif_t &sim) {
  return std::make_unique<fasedtests_top_t>(args, sim);
}
