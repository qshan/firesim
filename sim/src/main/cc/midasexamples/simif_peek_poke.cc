// See LICENSE for license details.

#include "simif_peek_poke.h"

simif_peek_poke_t::simif_peek_poke_t() {
  PEEKPOKEBRIDGEMODULE_0_substruct_create;
  this->defaultiowidget_mmio_addrs = PEEKPOKEBRIDGEMODULE_0_substruct;
}

void simif_peek_poke_t::target_reset(int pulse_length) {
  poke(reset, 1);
  take_steps(pulse_length, true);
  poke(reset, 0);
}

static const size_t data_t_chunks = sizeof(data_t) / sizeof(uint32_t);

void simif_peek_poke_t::step(uint32_t n, bool blocking) {
  if (n == 0) return;
  // take steps
  if (log) fprintf(stderr, "* STEP %d -> %llu *\n", n, (t + n));
  take_steps(n, blocking);
  t += n;
}

void simif_peek_poke_t::poke(size_t id, data_t value, bool blocking) {
  if (blocking && !wait_on_ready(10.0)) {
    if (log) {
      std::string fmt = "* FAIL : POKE on %s.%s has timed out. %s : FAIL\n";
      fprintf(stderr, fmt.c_str(), TARGET_NAME, (const char*) INPUT_NAMES[id], blocking_fail.c_str());
    }
    throw;
  }
  if (log)
    fprintf(stderr, "* POKE %s.%s <- 0x%x *\n", TARGET_NAME, INPUT_NAMES[id], value);
  write(INPUT_ADDRS[id], value);
}

data_t simif_peek_poke_t::peek(size_t id, bool blocking) {
  if (blocking && !wait_on_ready(10.0)) {
    if (log) {
      std::string fmt = "* FAIL : PEEK on %s.%s has timed out. %s : FAIL\n";
      fprintf(stderr, fmt.c_str(), TARGET_NAME, (const char*) INPUT_NAMES[id], blocking_fail.c_str());
    }
    throw;
  }
  if (log && blocking && !wait_on_stable_peeks(0.1))
    fprintf(stderr, "* WARNING : The following peek is on an unstable value!\n");
  data_t value = read(((unsigned int*) OUTPUT_ADDRS)[id]);
  if (log)
    fprintf(stderr, "* PEEK %s.%s -> 0x%x *\n", TARGET_NAME, (const char*) OUTPUT_NAMES[id], value);
  return value;
}

data_t simif_peek_poke_t::sample_value(size_t id) {
  return peek(id, false);
}

bool simif_peek_poke_t::expect(size_t id, data_t expected) {
  data_t value = peek(id);
  bool pass = value == expected;
  if (log) {
    fprintf(stderr, "* EXPECT %s.%s -> 0x%x ?= 0x%x : %s\n",
            TARGET_NAME, (const char*)OUTPUT_NAMES[id], value, expected, pass ? "PASS" : "FAIL");
  }
  return expect(pass, NULL);
}

bool simif_peek_poke_t::expect(bool pass, const char *s) {
  if (log && s) fprintf(stderr, "* %s : %s *\n", s, pass ? "PASS" : "FAIL");
  if (this->pass && !pass) fail_t = t;
  this->pass &= pass;
  return pass;
}

void simif_peek_poke_t::poke(size_t id, mpz_t& value) {
  if (log) {
    char* v_str = mpz_get_str(NULL, 16, value);
    fprintf(stderr, "* POKE %s.%s <- 0x%s *\n", TARGET_NAME, INPUT_NAMES[id], v_str);
    free(v_str);
  }
  size_t size;
  data_t* data = (data_t*)mpz_export(NULL, &size, -1, sizeof(data_t), 0, 0, value);
  for (size_t i = 0 ; i < INPUT_CHUNKS[id] ; i++) {
    write(INPUT_ADDRS[id]+ (i * sizeof(data_t)), i < size ? data[i] : 0);
  }
}

void simif_peek_poke_t::peek(size_t id, mpz_t& value) {
  const size_t size = (const size_t)OUTPUT_CHUNKS[id];
  data_t data[size];
  for (size_t i = 0 ; i < size ; i++) {
    data[i] = read((size_t)OUTPUT_ADDRS[id] + (i * sizeof(data_t)) );
  }
  mpz_import(value, size, -1, sizeof(data_t), 0, 0, data);
  if (log) {
    char* v_str = mpz_get_str(NULL, 16, value);
    fprintf(stderr, "* PEEK %s.%s -> 0x%s *\n", TARGET_NAME, (const char*)OUTPUT_NAMES[id], v_str);
    free(v_str);
  }
}

bool simif_peek_poke_t::expect(size_t id, mpz_t& expected) {
  mpz_t value;
  mpz_init(value);
  peek(id, value);
  bool pass = mpz_cmp(value, expected) == 0;
  if (log) {
    char* v_str = mpz_get_str(NULL, 16, value);
    char* e_str = mpz_get_str(NULL, 16, expected);
    fprintf(stderr, "* EXPECT %s.%s -> 0x%s ?= 0x%s : %s\n",
      TARGET_NAME, (const char*)OUTPUT_NAMES[id], v_str, e_str, pass ? "PASS" : "FAIL");
    free(v_str);
    free(e_str);
  }
  mpz_clear(value);
  return expect(pass, NULL);
}

int simif_peek_poke_t::teardown() {
  record_end_times();
  fprintf(stderr, "[%s] %s Test", pass ? "PASS" : "FAIL", TARGET_NAME);
  if (!pass) { fprintf(stdout, " at cycle %llu", fail_t); }
  fprintf(stderr, "SEED: %ld\n", get_seed());
  this->print_simulation_performance_summary();

  this->host_finish();

  return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
