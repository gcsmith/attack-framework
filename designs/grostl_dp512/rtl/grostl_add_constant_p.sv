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

module grostl_add_constant_p(input  [0:7][0:7][7:0] din,
                             input            [3:0] rnd,
                             output [0:7][0:7][7:0] dout);

  assign dout[0] = { din[0][0] ^ 8'h00 ^ { 4'h0, rnd }, din[0][1:7] };
  assign dout[1] = { din[1][0] ^ 8'h10 ^ { 4'h0, rnd }, din[1][1:7] };
  assign dout[2] = { din[2][0] ^ 8'h20 ^ { 4'h0, rnd }, din[2][1:7] };
  assign dout[3] = { din[3][0] ^ 8'h30 ^ { 4'h0, rnd }, din[3][1:7] };
  assign dout[4] = { din[4][0] ^ 8'h40 ^ { 4'h0, rnd }, din[4][1:7] };
  assign dout[5] = { din[5][0] ^ 8'h50 ^ { 4'h0, rnd }, din[5][1:7] };
  assign dout[6] = { din[6][0] ^ 8'h60 ^ { 4'h0, rnd }, din[6][1:7] };
  assign dout[7] = { din[7][0] ^ 8'h70 ^ { 4'h0, rnd }, din[7][1:7] };

endmodule

