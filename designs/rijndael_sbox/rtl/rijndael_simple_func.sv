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

module rijndael_simple_func(input        clk, rst, valid,
                            input  [7:0] din, key,
                            output [7:0] dout);

  enum logic [1:0] { LOAD, BSUB } cs, ns;
  logic [7:0] data_reg, sb_out;

  // state register control
  always @(posedge clk, posedge rst)
    cs <= rst ? LOAD : ns;

  // next state control logic
  always_comb begin
    ns <= cs;
    case (cs)
    LOAD: if (valid) ns <= BSUB;
    BSUB: ns <= LOAD;
    endcase
  end

  // state machine registered outputs
  always @(posedge clk) begin
    case (cs)
    LOAD: data_reg <= din ^ key;
    BSUB: data_reg <= sb_out;
    endcase
  end

  rijndael_sbox_lut sub_bytes(data_reg, sb_out);

  assign dout = data_reg;

endmodule

