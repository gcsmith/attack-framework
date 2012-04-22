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

module grostl_mix_bytes(input  [0:7][0:7][7:0] din,
                        output [0:7][0:7][7:0] dout);

  function [7:0] gf_2(input [7:0] x);
    gf_2 = { x[6],
             x[5],
             x[4],
             x[3] ^ x[7],
             x[2] ^ x[7],
             x[1],
             x[0] ^ x[7],
             x[7] };
  endfunction

  function [7:0] gf_4(input [7:0] x);
    gf_4 = { x[5],
             x[4],
             x[3] ^ x[7],
             x[2] ^ x[7] ^ x[6],
             x[1] ^ x[6],
             x[0] ^ x[7],
             x[7] ^ x[6],
             x[6] };
  endfunction
 
  function [7:0] gf_3(input [7:0] x);
    gf_3 = gf_2(x) ^ x;
  endfunction

  function [7:0] gf_5(input [7:0] x);
    gf_5 = gf_4(x) ^ x;
  endfunction

  function [7:0] gf_7(input [7:0] x);
    gf_7 = gf_4(x) ^ gf_2(x) ^ x;
  endfunction

  generate
    for (genvar c = 0; c < 8; c++) begin: gen_mix_cols
      assign dout[c][0] = gf_2(din[c][0]) ^ gf_2(din[c][1]) ^ gf_3(din[c][2]) ^ gf_4(din[c][3]) ^ gf_5(din[c][4]) ^ gf_3(din[c][5]) ^ gf_5(din[c][6]) ^ gf_7(din[c][7]);
      assign dout[c][1] = gf_7(din[c][0]) ^ gf_2(din[c][1]) ^ gf_2(din[c][2]) ^ gf_3(din[c][3]) ^ gf_4(din[c][4]) ^ gf_5(din[c][5]) ^ gf_3(din[c][6]) ^ gf_5(din[c][7]);
      assign dout[c][2] = gf_5(din[c][0]) ^ gf_7(din[c][1]) ^ gf_2(din[c][2]) ^ gf_2(din[c][3]) ^ gf_3(din[c][4]) ^ gf_4(din[c][5]) ^ gf_5(din[c][6]) ^ gf_3(din[c][7]);
      assign dout[c][3] = gf_3(din[c][0]) ^ gf_5(din[c][1]) ^ gf_7(din[c][2]) ^ gf_2(din[c][3]) ^ gf_2(din[c][4]) ^ gf_3(din[c][5]) ^ gf_4(din[c][6]) ^ gf_5(din[c][7]);
      assign dout[c][4] = gf_5(din[c][0]) ^ gf_3(din[c][1]) ^ gf_5(din[c][2]) ^ gf_7(din[c][3]) ^ gf_2(din[c][4]) ^ gf_2(din[c][5]) ^ gf_3(din[c][6]) ^ gf_4(din[c][7]);
      assign dout[c][5] = gf_4(din[c][0]) ^ gf_5(din[c][1]) ^ gf_3(din[c][2]) ^ gf_5(din[c][3]) ^ gf_7(din[c][4]) ^ gf_2(din[c][5]) ^ gf_2(din[c][6]) ^ gf_3(din[c][7]);
      assign dout[c][6] = gf_3(din[c][0]) ^ gf_4(din[c][1]) ^ gf_5(din[c][2]) ^ gf_3(din[c][3]) ^ gf_5(din[c][4]) ^ gf_7(din[c][5]) ^ gf_2(din[c][6]) ^ gf_2(din[c][7]);
      assign dout[c][7] = gf_2(din[c][0]) ^ gf_3(din[c][1]) ^ gf_4(din[c][2]) ^ gf_5(din[c][3]) ^ gf_3(din[c][4]) ^ gf_5(din[c][5]) ^ gf_7(din[c][6]) ^ gf_2(din[c][7]);
    end
  endgenerate

endmodule

