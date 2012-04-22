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

module aes_key_mem(input  [7:0] rd_addr,
                   output [7:0] rd_data);

  const logic [7:0] rom[0:175] = '{
    'h00, 'h01, 'h02, 'h03, 'h04, 'h05, 'h06, 'h07, 'h08, 'h09, 'h0A, 'h0B, 'h0C, 'h0D, 'h0E, 'h0F, 
    'hD6, 'hAA, 'h74, 'hFD, 'hD2, 'hAF, 'h72, 'hFA, 'hDA, 'hA6, 'h78, 'hF1, 'hD6, 'hAB, 'h76, 'hFE, 
    'hB6, 'h92, 'hCF, 'h0B, 'h64, 'h3D, 'hBD, 'hF1, 'hBE, 'h9B, 'hC5, 'h00, 'h68, 'h30, 'hB3, 'hFE, 
    'hB6, 'hFF, 'h74, 'h4E, 'hD2, 'hC2, 'hC9, 'hBF, 'h6C, 'h59, 'h0C, 'hBF, 'h04, 'h69, 'hBF, 'h41, 
    'h47, 'hF7, 'hF7, 'hBC, 'h95, 'h35, 'h3E, 'h03, 'hF9, 'h6C, 'h32, 'hBC, 'hFD, 'h05, 'h8D, 'hFD, 
    'h3C, 'hAA, 'hA3, 'hE8, 'hA9, 'h9F, 'h9D, 'hEB, 'h50, 'hF3, 'hAF, 'h57, 'hAD, 'hF6, 'h22, 'hAA, 
    'h5E, 'h39, 'h0F, 'h7D, 'hF7, 'hA6, 'h92, 'h96, 'hA7, 'h55, 'h3D, 'hC1, 'h0A, 'hA3, 'h1F, 'h6B, 
    'h14, 'hF9, 'h70, 'h1A, 'hE3, 'h5F, 'hE2, 'h8C, 'h44, 'h0A, 'hDF, 'h4D, 'h4E, 'hA9, 'hC0, 'h26, 
    'h47, 'h43, 'h87, 'h35, 'hA4, 'h1C, 'h65, 'hB9, 'hE0, 'h16, 'hBA, 'hF4, 'hAE, 'hBF, 'h7A, 'hD2, 
    'h54, 'h99, 'h32, 'hD1, 'hF0, 'h85, 'h57, 'h68, 'h10, 'h93, 'hED, 'h9C, 'hBE, 'h2C, 'h97, 'h4E, 
    'h13, 'h11, 'h1D, 'h7F, 'hE3, 'h94, 'h4A, 'h17, 'hF3, 'h07, 'hA7, 'h8B, 'h4D, 'h2B, 'h30, 'hC5
  };

  assign rd_data = rom[rd_addr];

endmodule

