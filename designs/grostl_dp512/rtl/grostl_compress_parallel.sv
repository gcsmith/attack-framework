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

module grostl_compress_parallel(input          clk, wr,
                                input    [3:0] round,
                                input  [511:0] m_in, h_in,
                                output [511:0] dout);

  logic [0:7][0:7][7:0] p_in, p_rc, p_sb, p_sh, p_mb, p_reg;
  logic [0:7][0:7][7:0] q_in, q_rc, q_sb, q_sh, q_mb, q_reg;

  assign p_in = wr == 0 ? p_reg : m_in ^ h_in;
  assign q_in = wr == 0 ? q_reg : m_in;
  assign dout = h_in ^ p_reg ^ q_reg;

  grostl_add_constant_p p_add_const(p_in, round, p_rc);
  grostl_sub_bytes      p_sub_bytes(p_rc, p_sb);
  grostl_shift_bytes_p  p_shf_bytes(p_sb, p_sh);
  grostl_mix_bytes      p_mix_bytes(p_sh, p_mb);

  grostl_add_constant_q q_add_const(q_in, round, q_rc);
  grostl_sub_bytes      q_sub_bytes(q_rc, q_sb);
  grostl_shift_bytes_q  q_shf_bytes(q_sb, q_sh);
  grostl_mix_bytes      q_mix_bytes(q_sh, q_mb);

  always_ff @(posedge clk) p_reg <= p_mb;
  always_ff @(posedge clk) q_reg <= q_mb;

endmodule

