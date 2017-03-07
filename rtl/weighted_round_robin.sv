//========================================================================== //
// Copyright (c) 2016, Stephen Henry
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

module weighted_round_robin #(
    parameter int N = 32
  , parameter int PRIORITY_W = 4
  , parameter int ID_BITS = $clog2(N)
) (

 //========================================================================== //
 //                                                                           //
 // Misc.                                                                     //
 //                                                                           //
 //========================================================================== //

   input                                clk
 , input                                rst

 //========================================================================== //
 //                                                                           //
 // Round Robin                                                               //
 //                                                                           //
 //========================================================================== //

 , input        [N-1:0]                 req
 , input                                ack

 , output logic [N-1:0]                 gnt_w
 , output logic [ID_BITS-1:0]           gnt_id

 //========================================================================== //
 //                                                                           //
 // Round Robin                                                               //
 //                                                                           //
 //========================================================================== //

 , input       [PRIORITY_W-1:0]         prio
 , input       [ID_BITS-1:0]            prio_id
 , input                                prio_upt
);

  typedef logic [PRIORITY_W-1:0] priority_t;
  typedef logic [PRIORITY_W:0] sterile_t;
  typedef logic [N-1:0]          n_t;
  typedef struct packed {
    priority_t [N-1:0] p;
  } priority_vec_t;
  function priority_t bit_reverse;
    input priority_t in;
    begin
      int W = $bits(priority_t);
      priority_t ret;
      for (int i = 0; i < W; i++)
        ret [i]  = in [W - i - 1];
      return ret;
    end
  endfunction

  // ======================================================================== //
  //                                                                          //
  // Wires                                                                    //
  //                                                                          //
  // ======================================================================== //

  //
  logic [PRIORITY_W-1:0]           priority_r;
  //
  logic                            cycle_label_counter_en;
  logic [PRIORITY_W-1:0]           cycle_label_counter_w;
  logic [PRIORITY_W-1:0]           cycle_label_counter_r;
  sterile_t                        ffs0_post_ss;
  sterile_t                        sterile_string;
  sterile_t                        sterile_string_next_0;
  sterile_t                        sterile_string_first_1;
  logic                            next_0_is_sterile;
  sterile_t                        sterile_mask;
  sterile_t                        sterile_inc_A;
  sterile_t                        sterile_inc_B;
  sterile_t                        sterile_inc_C;
  sterile_t                        sterile_inc;
  //
  logic                            cycle_label_counter_upt;
  //
  logic [PRIORITY_W-1:0]           cycle_idx_1d;
  //
  priority_vec_t                   priority_vec_w;
  priority_vec_t                   priority_vec_r;
  logic                            priority_vec_en;
  //
  n_t                              round_req;
  //
  n_t                              sel_vec;
  n_t                              sel_vec_w;
  n_t                              sel_vec_mask;
  n_t                              sel_vec_mask_w;
  n_t                              sel_vec_mask_r;
  logic                            sel_vec_mask_en;
  logic                            sel_vec_empty;
  n_t                              sel_vec_1d;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //


  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      //
      priority_vec_en         = rst | prio_upt;

      //
      for (int i = 0; i < N; i++)
        priority_vec_w.p [i]  =    (prio_id == i[ID_BITS-1:0])
                                 ? bit_reverse(prio)
                                 : priority_vec_r.p [i]
                               ;

    end // always_comb


  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sterile_string_PROC

      //
      sterile_string [PRIORITY_W] = 'b0;
      for (int p = 0; p < PRIORITY_W; p++) begin
        sterile_string [p] = '0;
        for (int i = 0; i < N; i++)
          sterile_string [p] |= req [i] & priority_vec_r.p [i][p];
      end

    end // block: sterile_string_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      //
      sterile_mask       = sterile_string_first_1 - 'b1;

      //
      ffs0_post_ss       = {1'b0, cycle_label_counter_r} | sterile_mask;

    end // block: sterile_string_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      //
      next_0_is_sterile  = |((~sterile_string) & sterile_string_next_0);

      //
      sterile_inc_A  = {1'b0, sterile_mask [PRIORITY_W-1:0]
                        ^ (cycle_label_counter_r & sterile_mask [PRIORITY_W-1:0])};

      //
      sterile_inc_B  = 'b1;

      //
      sterile_inc_C  = next_0_is_sterile ? sterile_string_first_1 : 'b0;

      //
      sterile_inc    = (sterile_inc_A + sterile_inc_B + sterile_inc_C);

    end // block: sterile_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      // Should never become zero.
      //
      casez ({rst})
        1'b1:    cycle_label_counter_w  = 'b1;
        default: cycle_label_counter_w  = cycle_label_counter_r +
                                          sterile_inc [PRIORITY_W-1:0];
      endcase

      //
      cycle_label_counter_upt  = (ack & sel_vec_empty);

      //
      cycle_label_counter_en   = (rst | cycle_label_counter_upt);

    end // always_comb


  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sec_vec_PROC

      //
      for (int i = 0; i < $bits(n_t); i++)
        round_req [i]  = req [i] & |(cycle_idx_1d & priority_vec_r.p[i]);

      //
      sel_vec          = (~sel_vec_mask_r) & (round_req);

    end // block: sec_vec_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      //
      casez ({rst, cycle_label_counter_upt})
        2'b1_?:  sel_vec_mask_w  = '0;
        2'b0_1:  sel_vec_mask_w  = '0;
        default: sel_vec_mask_w  = sel_vec_mask;
      endcase // casez ({rst})

      //
      sel_vec_mask_en         = (rst | ack);

    end // always_comb

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin

      //
      sel_vec_mask   = (sel_vec_mask_r | sel_vec_1d);

      //
      sel_vec_empty  = ~|((~sel_vec_mask) & round_req);

    end

  // ------------------------------------------------------------------------ //
  //
  always_comb gnt_w  = sel_vec_1d;

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (priority_vec_en)
      priority_vec_r <= priority_vec_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (cycle_label_counter_en)
      cycle_label_counter_r <= cycle_label_counter_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (sel_vec_mask_en)
      sel_vec_mask_r <= sel_vec_mask_w;

  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  ffs #(.W(PRIORITY_W)) u_ffs_cycle_label_counter (
    //
      .x                 (cycle_label_counter_r   )
    //
    , .y                 (cycle_idx_1d            )
    , .n                 (                        )
  );

  // ------------------------------------------------------------------------ //
  //
  ffs #(.W(N)) u_ffs_sel_vec (
    //
      .x                 (sel_vec                 )
    //
    , .y                 (sel_vec_1d              )
    , .n                 (                        )
  );

  // ------------------------------------------------------------------------ //
  //
  ffs #(.W($bits(sterile_t)), .OPT_FIND_FIRST_ZERO(1)) u_ffs_sterile (
    //
      .x                 (ffs0_post_ss            )
    //
    , .y                 (sterile_string_next_0   )
    , .n                 (                        )
  );

  // ------------------------------------------------------------------------ //
  //
  ffs #(.W($bits(sterile_t))) u_ffs_ss_first_1 (
    //
      .x                 (sterile_string          )
    //
    , .y                 (sterile_string_first_1  )
    , .n                 (                        )
  );

endmodule
