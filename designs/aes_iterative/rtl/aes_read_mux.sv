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

module aes_read_mux(input  [1:0] sel,
                    input  [7:0] d0, d1, d2, d3,
                    output [7:0] y);

  logic [7:0] out;

  // implement a 4-to-1 mux on 8-bit buses
  always_comb case (sel)
    2'b00:   out <= d0;
    2'b01:   out <= d1;
    2'b10:   out <= d2;
    default: out <= d3;
  endcase

  assign y = out;

endmodule

