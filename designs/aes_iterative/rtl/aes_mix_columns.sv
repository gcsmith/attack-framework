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

module aes_mix_columns(input  [3:0] row_sel,
                       input  [7:0] r0, r1, r2, r3,
                       output [7:0] out);

  logic [1:0] m0, m1, m2, m3; // control signals to indicate coefficient
  logic [7:0] p0, p1, p2, p3; // product after multiplication
  logic c0, c1, c2, c3, ca;   // carry result / sum from multiplication
  logic [7:0] pa;             // sum of products

  // generate control signals for each multiplier
  always_comb case (row_sel)
    4'b0001: begin m0 <= 2'b10; m1 <= 2'b11; m2 <= 2'b01; m3 <= 2'b01; end
    4'b0010: begin m0 <= 2'b01; m1 <= 2'b10; m2 <= 2'b11; m3 <= 2'b01; end
    4'b0100: begin m0 <= 2'b01; m1 <= 2'b01; m2 <= 2'b10; m3 <= 2'b11; end
    default: begin m0 <= 2'b11; m1 <= 2'b01; m2 <= 2'b01; m3 <= 2'b10; end
  endcase

  // compute the products
  always_comb case (m0)
    2'b01:   p0 <= r0;
    2'b10:   p0 <= { r0[6:0], 1'b0 };
    default: p0 <= { r0[6:0], 1'b0 } ^ r0;
  endcase

  always_comb case (m1)
    2'b01:   p1 <= r1;
    2'b10:   p1 <= { r1[6:0], 1'b0 };
    default: p1 <= { r1[6:0], 1'b0 } ^ r1;
  endcase

  always_comb case (m2)
    2'b01:   p2 <= r2;
    2'b10:   p2 <= { r2[6:0], 1'b0 };
    default: p2 <= { r2[6:0], 1'b0 } ^ r2;
  endcase

  always_comb case (m3)
    2'b01:   p3 <= r3;
    2'b10:   p3 <= { r3[6:0], 1'b0 };
    default: p3 <= { r3[6:0], 1'b0 } ^ r3;
  endcase

  // compute the carry signals
  assign c0 = r0[7] & m0[1];
  assign c1 = r1[7] & m1[1];
  assign c2 = r2[7] & m2[1];
  assign c3 = r3[7] & m3[1];

  // compute the sum of products and sum of carry signals
  assign pa = p0 ^ p1 ^ p2 ^ p3;
  assign ca = c0 ^ c1 ^ c2 ^ c3;

  assign out = { pa[7], pa[6], pa[5], pa[4] ^ ca,
                 pa[3] ^ ca, pa[2], pa[1] ^ ca, pa[0] ^ ca };
endmodule

