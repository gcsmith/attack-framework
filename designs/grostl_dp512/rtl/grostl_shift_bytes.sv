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

  wire [511:0] out_p, out_q;

  grostl_shift_bytes_p sbp(din, out_p);
  grostl_shift_bytes_q sbq(din, out_q);

  assign dout = pq ? out_q : out_p;

endmodule

