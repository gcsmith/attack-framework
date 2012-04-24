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

`timescale 1ns/1ns
`define ITERATIONS 10

module testbench;
  logic         clk = 0, wr_m = 0, wr_h = 0, sel_h = 0, sel_d = 0, sel_pq = 0;
  logic   [1:0] sel_m = '0;
  logic   [3:0] round = '0;
  logic   [2:0] column = '0;
  logic [511:0] m_in = '0, h_in = '0, dout;
  int fp_sim;

  // instantiate the DUT
  grostl_compress_serial dut(clk, wr_m, wr_h, sel_m, sel_h, sel_d, sel_pq,
                             round, column, m_in, h_in, dout);

  // invert the clock signal every 20 ns (25 MHz)
  always #20 clk = ~clk;

  class iteration_state;
    rand bit [511:0] msg;
    rand bit [511:0] chain;
  endclass

  task drive(logic [3:0] rnd, logic [2:0] col, wm, wh, logic [1:0] sm, sh, sd, pq);
    @(posedge clk);
    round = rnd;
    column = col;
    wr_m = wm;
    wr_h = wh;
    sel_m = sm;
    sel_h = sh;
    sel_d = sd;
    sel_pq = pq;
  endtask

  initial begin
    // set the initial system state
    fp_sim = $fopen("simulation.txt", "w");

    for (int i = 0; i < `ITERATIONS; i++) begin
      // create a random 512-bit message block and chaining value
      automatic iteration_state msg = new;
      void'(msg.randomize());

      // display and record the timestamp at the start of compression
      $display("%10d - iter %0d", $time, i);
      $fwrite(fp_sim, "%10d %0128X %0128X ", $time, msg.msg, msg.chain);

      // initialize the permute function with a 512-bit message block
      m_in = msg.msg;
      h_in = msg.chain;

      // round, column, wr_m, wr_h, sel_m, sel_h, sel_d, sel_pq

      drive(0, 0, 1, 1, 2'b00, 0, 1, 0); // 0: latch H & M
      drive(0, 0, 1, 0, 2'b10, 0, 1, 1); // 1: Q-S1, M^=H
      drive(0, 0, 1, 0, 2'b01, 0, 1, 0); // 2: Q-S2, P-S1

      for (int c = 1; c < 8; c++) begin
        drive(0, c, 1, 0, 2'b01, 0, 0, 1); // Q-S1, P-S2
        drive(0, c, 1, 0, 2'b01, 0, 0, 0); // Q-S2, P-S1
      end

      for (int r = 1; r < 10; r++) begin
        drive(r, 0, 1, 0, 2'b01, 0, 1, 1); // Q-S1, P-S2
        drive(r, 0, 1, 0, 2'b01, 0, 1, 0); // Q-S2, P-S1

        for (int c = 1; c < 8; c++) begin
          drive(r, c, 1, 0, 2'b01, 0, 0, 1); // Q-S1, P-S2
          drive(r, c, 1, 0, 2'b01, 0, 0, 0); // Q-S2, P-S1
        end
      end

      drive(0, 7, 1, 1, 2'b01, 1, 0, 1); // 21: P-S2, H^=M(Q_out)
      drive(0, 0, 1, 1, 2'b10, 1, 0, 0); // 0*: P-S2, H^=M(P_out) (next block)
      @(posedge clk);

      // retrieve the output state and write it to the trace file
      @(negedge clk) $fwrite(fp_sim, "%0128X\n", dout);
    end

    // close the file descriptor and terminate the simulation
    $fclose(fp_sim);
    $finish;
  end
endmodule

