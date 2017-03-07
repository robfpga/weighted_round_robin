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

#pragma once

#include <libtb.h>
#include "Vweighted_round_robin.h"
#include "weighted_round_robin_ports.h"
#include <map>

// Opts:
constexpr uint32_t OPT_N = 8;
constexpr uint32_t OPT_PRIORITY_W = 4;

using ReqT = vluint32_t;
using IdT = vluint32_t;
using PriorityT = vluint32_t;

struct weighted_round_robin : public libtb::TopLevel {
  SC_HAS_PROCESS(weighted_round_robin);
  weighted_round_robin();
  virtual ~weighted_round_robin();

 protected:
  void b_prio_idle();
  void b_prio_set(IdT id, PriorityT p);
  void report();
  void reset_perf();

 private:
  void bind_rtl();
  void m_checker();
  Vweighted_round_robin uut_;

  std::map<IdT, int> freq_;
  std::map<IdT, PriorityT> prio_map_;

 protected:
#define __define_signals(__port, __type) sc_core::sc_signal<__type> __port##_;
  WEIGHTED_ROUND_ROBIN_PORTS(__define_signals)
#undef __define_signals
};
