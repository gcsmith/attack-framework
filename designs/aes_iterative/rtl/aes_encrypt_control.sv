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

module aes_encrypt_control(input        clk, reset, valid_input,
                           output       valid_output,
                           output [2:0] op_sel, rd_col, wr_col,
                           output [1:0] rd_row,
                           output [3:0] wr_row,
                           output [7:0] key_addr,
                           output [3:0] rnd);

  enum logic [2:0] {
    IDLE, LOADT, BYTESUB, SHIFTROWS, MIXCOLS, ADDKEY, STORET
  } cs, ns;
  logic [2:0] shift_addr, select;
  logic [3:0] count, round, dest_row;
  logic [7:0] key_address;

  // state register control
  always @(posedge clk, posedge reset)
      cs <= reset ? IDLE : ns;

  // next state control logic
  always_comb begin
    ns <= cs;
    case (cs)
    IDLE:      if (valid_input) ns <= LOADT;
    LOADT:     if (count == 4'b1111) ns <= ADDKEY;
    BYTESUB:   if (count == 4'b1111) ns <= SHIFTROWS;
    SHIFTROWS: if (count == 4'b1111) ns <= (round == 10) ? ADDKEY : MIXCOLS;
    MIXCOLS:   if (count == 4'b1111) ns <= ADDKEY;
    ADDKEY:    if (count == 4'b1111) ns <= (round == 10) ? STORET : BYTESUB;
    STORET:    if (count == 4'b1111) ns <= IDLE;
    endcase
  end

  // state machine registered outputs
  always @(posedge clk) begin
    count <= count + 1;
    case (cs)
    IDLE: begin
      count <= 4'd0;
      round <= 4'd0;
      key_address <= 8'd0;
    end
    ADDKEY: begin
      if (count == 4'b1111 && round != 10)
        round <= round + 1;
      if (key_address != 8'b10101111)
        key_address <= key_address + 1;
    end
    default: ;
    endcase
  end

  // state machine combinational outputs
  always_comb case (cs)
    default:   select <= 3'b000;
    LOADT:     select <= 3'b000;
    BYTESUB:   select <= 3'b001;
    SHIFTROWS: select <= 3'b010;
    MIXCOLS:   select <= 3'b011;
    ADDKEY:    select <= 3'b100;
    STORET:    select <= 3'b000;
  endcase

  always_comb case (cs)
    IDLE:    dest_row <= 4'b0000;
    STORET:  dest_row <= 4'b0000;
    default: dest_row <= {  count[1] & count[0],  count[1] & ~count[0],
                           ~count[1] & count[0], ~count[1] & ~count[0] };
  endcase

  assign op_sel = select;
  assign rd_row = count[1:0];
  assign rd_col = { (cs == MIXCOLS) || (cs == ADDKEY && round == 10), count[3:2] };
  assign wr_row = dest_row;

  always_comb case (count)
    4'b0000: shift_addr <= 3'b100;
    4'b0001: shift_addr <= 3'b111;
    4'b0010: shift_addr <= 3'b110;
    4'b0011: shift_addr <= 3'b101;
    4'b0100: shift_addr <= 3'b101;
    4'b0101: shift_addr <= 3'b100;
    4'b0110: shift_addr <= 3'b111;
    4'b0111: shift_addr <= 3'b110;
    4'b1000: shift_addr <= 3'b110;
    4'b1001: shift_addr <= 3'b101;
    4'b1010: shift_addr <= 3'b100;
    4'b1011: shift_addr <= 3'b111;
    4'b1100: shift_addr <= 3'b111;
    4'b1101: shift_addr <= 3'b110;
    4'b1110: shift_addr <= 3'b101;
    4'b1111: shift_addr <= 3'b100;
  endcase

  assign wr_col = (cs == SHIFTROWS) ? shift_addr : { 1'b0, count[3:2] };
  assign valid_output = (cs == STORET);
  assign key_addr = key_address;
  assign rnd = round;

endmodule

