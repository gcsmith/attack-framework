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
                              input          sel_h, sel_d, sel_pq,
                              input    [3:0] rnd,
                              input    [2:0] col,
                              input  [511:0] m_in, h_in,
                              output [511:0] dout);

  logic [0:7][0:7][7:0] m_reg, h_reg, p_reg;
  logic [0:7][0:7][7:0] m_val, h_val, mhxor, s_shf, d_val;
  logic [0:7][7:0] s_arc, s_sub, s_mix;

  grostl_shift_bytes  shf_bytes(m_reg, sel_pq, s_shf);
  grostl_add_constant add_const(d_val[0], sel_pq, rnd, col, s_arc); // stage 1
  grostl_sub_bytes    sub_bytes(s_arc, s_sub);
  grostl_mix_bytes    mix_bytes(p_reg[0], s_mix);

  assign mhxor = m_reg ^ h_reg;
  assign dout  = m_reg;

  always_comb case (sel_m)
    2'b00:   m_val <= m_in;                  // 0: message input
    2'b01:   m_val <= { p_reg[1:7], s_mix }; // 1: column-rotated round output
    default: m_val <= mhxor;                 // 2: m xor h
  endcase

  always_comb case (sel_h)
    1'b0:    h_val <= h_in;  // 0: hash input (IV)
    default: h_val <= mhxor; // 1: m xor h
  endcase

  always_comb case (sel_d)
    1'b0:    d_val <= m_reg;  // 0: unshifted state (subround)
    default: d_val <= s_shf;  // 1: shifted state (round start)
  endcase

  always_ff @(posedge clk) if (wr_m) m_reg <= m_val;       // message register
  always_ff @(posedge clk) if (wr_h) h_reg <= h_val;       // chaining register
  always_ff @(posedge clk) p_reg <= { s_sub, d_val[1:7] }; // pipeline register

endmodule

