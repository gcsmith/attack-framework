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

module grostl_add_constant(input  [0:7][7:0] din,
                           input             pq,
                           input       [3:0] rnd,
                           input       [2:0] col,
                           output [0:7][7:0] dout);

  wire [63:0] p, q;
  wire [2:0] cpp;

  assign cpp = col + 6;
  assign p = { din[0] ^ { 1'b0, col, 4'h0 } ^ { 4'h0, rnd }, din[1:7] };
  assign q = { din[0:6], din[7] ^ { 1'b0, cpp, 4'h0 } ^ { 4'h0, rnd } } ^ '1;

  assign dout = pq ? q : p;

endmodule

