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

module grostl_add_constant_q(input  [0:7][0:7][7:0] din,
                             input            [3:0] rnd,
                             output [0:7][0:7][7:0] dout);

  assign dout[0] = { din[0][0:6] ^ '1, din[0][7] ^ 8'hFF ^ { 4'h0, rnd } };
  assign dout[1] = { din[1][0:6] ^ '1, din[1][7] ^ 8'hEF ^ { 4'h0, rnd } };
  assign dout[2] = { din[2][0:6] ^ '1, din[2][7] ^ 8'hDF ^ { 4'h0, rnd } };
  assign dout[3] = { din[3][0:6] ^ '1, din[3][7] ^ 8'hCF ^ { 4'h0, rnd } };
  assign dout[4] = { din[4][0:6] ^ '1, din[4][7] ^ 8'hBF ^ { 4'h0, rnd } };
  assign dout[5] = { din[5][0:6] ^ '1, din[5][7] ^ 8'hAF ^ { 4'h0, rnd } };
  assign dout[6] = { din[6][0:6] ^ '1, din[6][7] ^ 8'h9F ^ { 4'h0, rnd } };
  assign dout[7] = { din[7][0:6] ^ '1, din[7][7] ^ 8'h8F ^ { 4'h0, rnd } };

endmodule

