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

#include "weighted_round_robin.h"
#include <sstream>

weighted_round_robin::weighted_round_robin()
    : libtb::TopLevel("t"),
      uut_("uut")
#define __declare_port(__port, __type) , __port##_(#__port)
          WEIGHTED_ROUND_ROBIN_PORTS(__declare_port)
#undef __declare_port
{
  LIBTB_REPORT_INFO("FOO");
  bind_rtl();
  SC_METHOD(m_checker);
  sensitive << e_tb_sample();
  dont_initialize();

  wave_on("foo.vcd", uut_);
}

weighted_round_robin::~weighted_round_robin() {}

void weighted_round_robin::bind_rtl() {
  uut_.clk(clk());
  uut_.rst(rst());
#define __bind_signals(__port, __type) uut_.__port(__port##_);
  WEIGHTED_ROUND_ROBIN_PORTS(__bind_signals)
#undef __bind_signals
}

void weighted_round_robin::m_checker() {
  ack_ = true;
  if (req_ != 0) {
    const ReqT gnt = gnt_w_;
    const unsigned idx = libtb::ffs(gnt);
    if (gnt != 0) {
      const bool is_valid = (prio_map_[idx] > 0 && (req_ & (1 << idx)));
      if (!is_valid) LIBTB_REPORT_ERROR("Invalid GNT asserted");
    }

    const unsigned gnt_count = libtb::pop_count(gnt);
    if (gnt_count != 1 && prio_map_[idx] != 0) {
        std::stringstream ss;
        ss << "Invalid number of grants: " << gnt_count;
        LIBTB_REPORT_ERROR(ss.str());
    }

    if (gnt != 0)
        freq_[libtb::ffs(gnt)]++;
  }
}

void weighted_round_robin::report() {
  std::stringstream ss;

  ss << "Number of Grant lines: " << freq_.size();
  LIBTB_REPORT_INFO(ss.str());

  unsigned total = 0;
  for (auto g : freq_) total += g.second;

  for (int i = 0; i < OPT_N; i++) {
    ss.str("");

    int order = 0;
    if (freq_.count(i)) order = ((double)freq_[i] / (double)total) * 100;
    ss << i << "| " << std::string(order, '*');
    LIBTB_REPORT_INFO(ss.str());
  }
}

void weighted_round_robin::b_prio_idle() {
  prio_upt_ = false;
  prio_ = PriorityT{};
  prio_id_ = IdT{};
}

void weighted_round_robin::reset_perf() { freq_.clear(); }

void weighted_round_robin::b_prio_set(IdT id, PriorityT p) {
  prio_upt_ = true;
  prio_ = p;
  prio_id_ = id;
  {
    std::stringstream ss;
    ss << "Setting priority ID=" << id << " P=" << p;
    LIBTB_REPORT_DEBUG(ss.str());
  }
  t_wait_posedge_clk();
  prio_map_[id] = p;
  b_prio_idle();
}
