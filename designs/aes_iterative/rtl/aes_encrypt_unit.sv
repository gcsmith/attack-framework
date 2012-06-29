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

module aes_encrypt_unit(input        clk, reset, valid_input,
                        input  [7:0] data_in,
                        output       valid_output,
                        output [7:0] data_out,
                        output [3:0] rnd);

  wire [1:0] rd_row;
  wire [2:0] sel, wr_col, rd_col;
  wire [3:0] wr_row;
  wire [7:0] state, key, ak_out, sb_out, mc_out, wr_data, key_addr, r0, r1, r2, r3;

  aes_add_round_key add_round_key(state, key, ak_out);
`ifdef LOGIC_SBOX
  bSbox sub_bytes(state, '1, sb_out);
`else
  aes_sbox_lut sub_bytes(state, sb_out);
`endif

  aes_dual_row_mem mem0(clk, wr_row[0], wr_col, rd_col, wr_data, r0);
  aes_dual_row_mem mem1(clk, wr_row[1], wr_col, rd_col, wr_data, r1);
  aes_dual_row_mem mem2(clk, wr_row[2], wr_col, rd_col, wr_data, r2);
  aes_dual_row_mem mem3(clk, wr_row[3], wr_col, rd_col, wr_data, r3);

  aes_encrypt_control control(clk, reset, valid_input, valid_output, sel,
                              rd_col, wr_col, rd_row, wr_row, key_addr, rnd);

  aes_key_mem key_mem(key_addr, key);
  aes_mix_columns mix_columns(wr_row, r0, r1, r2, r3, mc_out);
  aes_read_mux r_mux(rd_row, r0, r1, r2, r3, state);
  aes_write_mux w_mux(clk, sel, data_in, sb_out, state, mc_out, ak_out, wr_data);

  assign data_out = state;

endmodule

