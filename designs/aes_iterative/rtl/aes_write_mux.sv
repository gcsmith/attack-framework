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

module aes_write_mux(input        clk,
                     input  [2:0] sel,
                     input  [7:0] d0, d1, d2, d3, d4,
                     output [7:0] y);

  // implement a 5-to-1 mux with first data port registered
  logic [7:0] buffer, out;

  always @(posedge clk)
    buffer <= d0;

  always_comb case (sel)
    3'b000:  out <= buffer;
    3'b001:  out <= d1;
    3'b010:  out <= d2;
    3'b011:  out <= d3;
    default: out <= d4;
  endcase

  assign y = out;

endmodule

