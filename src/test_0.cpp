//========================================================================== //
// Copyright (c) 2017, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

#include <libtb.h>
#include "weighted_round_robin.h"

struct test_0 : public weighted_round_robin {
  SC_HAS_PROCESS(test_0);
  test_0() {}
  bool run_test() {
    LIBTB_REPORT_INFO("Stimulus begins");
    b_prio_idle();

    //    b_prio_set(2, 1);
    for (int i = 0; i < OPT_N; i++) b_prio_set(i, i % 2 == 0);

    reset_perf();
    int ROUNDS = 100;
    while (ROUNDS--) round();

    report();

    LIBTB_REPORT_INFO("Stimulus ends");
    return true;
  }

  void round() {
      const ReqT req_vector = libtb::ones<ReqT>();

      int N = 100;
      req_ = req_vector;
      while (N--)
          t_wait_posedge_clk();
  }
};

int sc_main(int argc, char **argv) {
  using namespace libtb;
  test_0 t;
  LibTbContext::init(argc, argv);
  return LibTbContext::start();
}
