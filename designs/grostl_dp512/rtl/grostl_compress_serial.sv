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

module grostl_compress_serial(input          clk, wr_m, wr_h,
                              input    [1:0] sel_m,
                              input          sel_h, sel_pq,
                              input    [3:0] round,
                              input  [511:0] m_in, h_in,
                              output [511:0] dout);

  logic [0:7][0:7][7:0] m_reg, h_reg, p_reg;
  logic [0:7][0:7][7:0] m_val, h_val, mhxor, s_arc, s_sub, s_shf, s_mix;

  grostl_add_constant add_const(m_reg, sel_pq, round, s_arc); // stage 1
  grostl_sub_bytes    sub_bytes(s_arc, s_sub);
  grostl_shift_bytes  shf_bytes(p_reg, ~sel_pq, s_shf);       // stage 2
  grostl_mix_bytes    mix_bytes(s_shf, s_mix);

  assign mhxor = m_reg ^ h_reg;
  assign dout  = m_reg;

  always_comb case (sel_m)
    2'b00:   m_val <= m_in;  // 0: message input
    2'b01:   m_val <= s_mix; // 1: round output
    default: m_val <= mhxor; // 2: m xor h
  endcase

  always_comb case (sel_h)
    1'b0:    h_val <= h_in;  // 0: hash input (IV)
    default: h_val <= mhxor; // 1: m xor h
  endcase

  always_ff @(posedge clk) if (wr_m) m_reg <= m_val; // message register
  always_ff @(posedge clk) if (wr_h) h_reg <= h_val; // chaining register
  always_ff @(posedge clk) p_reg <= s_sub;           // pipeline register

endmodule

