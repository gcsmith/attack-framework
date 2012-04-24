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

module grostl_sub_bytes_m(input  [0:7][7:0] din, imask, omask,
                          output [0:7][7:0] dout);

  generate
    for (genvar i = 0; i < 8; i++) begin: gen_sbox
      bSbox sub_bytes(din[i], imask[i], omask[i], 1'b1, dout[i]);
    end
  endgenerate

endmodule

