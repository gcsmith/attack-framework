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

module aes_dual_row_mem(input        clk, wr_en,
                        input  [2:0] wr_addr, rd_addr,
                        input  [7:0] wr_data,
                        output [7:0] rd_data);

  logic [7:0] ram[7:0];

  // asynchronous read
  assign rd_data = ram[rd_addr];

  // synchronous write
  always @(posedge clk)
    if (wr_en) ram[wr_addr] <= wr_data;

endmodule

