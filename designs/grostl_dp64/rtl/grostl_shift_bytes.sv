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

module grostl_shift_bytes(input  [0:7][0:7][7:0] din,
                          input                  pq,
                          output [0:7][0:7][7:0] dout);

  wire [0:7][0:7][7:0] p, q;

  assign p[0] = { din[0][0], din[1][1], din[2][2], din[3][3], din[4][4], din[5][5], din[6][6], din[7][7] };
  assign p[1] = { din[1][0], din[2][1], din[3][2], din[4][3], din[5][4], din[6][5], din[7][6], din[0][7] };
  assign p[2] = { din[2][0], din[3][1], din[4][2], din[5][3], din[6][4], din[7][5], din[0][6], din[1][7] };
  assign p[3] = { din[3][0], din[4][1], din[5][2], din[6][3], din[7][4], din[0][5], din[1][6], din[2][7] };
  assign p[4] = { din[4][0], din[5][1], din[6][2], din[7][3], din[0][4], din[1][5], din[2][6], din[3][7] };
  assign p[5] = { din[5][0], din[6][1], din[7][2], din[0][3], din[1][4], din[2][5], din[3][6], din[4][7] };
  assign p[6] = { din[6][0], din[7][1], din[0][2], din[1][3], din[2][4], din[3][5], din[4][6], din[5][7] };
  assign p[7] = { din[7][0], din[0][1], din[1][2], din[2][3], din[3][4], din[4][5], din[5][6], din[6][7] };

  assign q[0] = { din[1][0], din[3][1], din[5][2], din[7][3], din[0][4], din[2][5], din[4][6], din[6][7] };
  assign q[1] = { din[2][0], din[4][1], din[6][2], din[0][3], din[1][4], din[3][5], din[5][6], din[7][7] };
  assign q[2] = { din[3][0], din[5][1], din[7][2], din[1][3], din[2][4], din[4][5], din[6][6], din[0][7] };
  assign q[3] = { din[4][0], din[6][1], din[0][2], din[2][3], din[3][4], din[5][5], din[7][6], din[1][7] };
  assign q[4] = { din[5][0], din[7][1], din[1][2], din[3][3], din[4][4], din[6][5], din[0][6], din[2][7] };
  assign q[5] = { din[6][0], din[0][1], din[2][2], din[4][3], din[5][4], din[7][5], din[1][6], din[3][7] };
  assign q[6] = { din[7][0], din[1][1], din[3][2], din[5][3], din[6][4], din[0][5], din[2][6], din[4][7] };
  assign q[7] = { din[0][0], din[2][1], din[4][2], din[6][3], din[7][4], din[1][5], din[3][6], din[5][7] };

  assign dout = pq ? q : p;

endmodule

