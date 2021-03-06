// dpa framework - a collection of tools for differential power analysis
// Copyright (C) 2011  Garrett C. Smith
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

module grostl_compress_serial_m(input          clk, wr_m, wr_h,
                                input    [1:0] sel_m,
                                input          sel_h, sel_pq,
                                input    [3:0] round,
                                input  [511:0] m_in, h_in, imask, omask,
                                output [511:0] dout);

  logic [0:7][0:7][7:0] m_reg, h_reg, p_reg, imask_reg, omask_reg, x2, x3, x4,
                        sb_imask, sb_omask, m_val, h_val, mhxor, s_arc, s_sub,
                        s_shf, s_mix;

  // mask computation
  grostl_shift_bytes m_shf_bytes(omask_reg, '0, x2);
  grostl_mix_bytes   m_mix_bytes(x2, x3);

  assign sb_imask =  sel_pq ? '0 : imask_reg;
  assign sb_omask =  sel_pq ? '0 : omask_reg;
  assign x4       = ~sel_pq ? '0 : imask_reg ^ x3;

  // round computation
  grostl_add_constant pq_add_const(m_reg, sel_pq, round, s_arc);
  grostl_sub_bytes_m  pq_sub_bytes(s_arc, sb_imask, sb_omask, s_sub);
  grostl_shift_bytes  pq_shf_bytes(p_reg, ~sel_pq, s_shf);
  grostl_mix_bytes    pq_mix_bytes(s_shf, s_mix);

  assign mhxor = m_reg ^ h_reg;
  assign dout  = m_reg;

  always_comb case (sel_m)
    2'b00:   m_val <= m_in;         // 0: message input
    2'b01:   m_val <= s_mix ^ x4;   // 1: masked round output
    default: m_val <= mhxor;        // 2: m xor h
  endcase

  always_comb case (sel_h)
    1'b0:    h_val <= h_in ^ imask; // 0: hash input (IV)
    default: h_val <= mhxor;        // 1: m xor h
  endcase

  always_ff @(posedge clk) if (wr_m) m_reg <= m_val; // message register
  always_ff @(posedge clk) if (wr_h) h_reg <= h_val; // chaining register
  always_ff @(posedge clk) p_reg <= s_sub;           // pipeline register

  always_ff @(posedge clk)
      if (sel_m == 2'b00) begin
          imask_reg <= imask;
          omask_reg <= omask;
      end

endmodule

