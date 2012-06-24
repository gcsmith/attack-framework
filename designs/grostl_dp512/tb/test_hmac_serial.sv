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

`timescale 1ns/10ps

module testbench;
  logic         clk = 0, wr_m = 0, wr_h = 0, sel_h = 0, sel_pq = 0;
  logic   [1:0] sel_m = '0;
  logic   [3:0] round = '0;
  logic [511:0] m_in = '0, h_in = '0, dout;
  int fp_sim;

  // instantiate the DUT
  grostl_compress_serial dut(clk, wr_m, wr_h, sel_m, sel_h, sel_pq,
                             round, m_in, h_in, dout);

  // invert the clock signal every 20 ns (25 MHz)
  always #20 clk = ~clk;

  class iteration_state;
    rand bit [511:0] msg;
  endclass

  task drive(input logic [3:0] rnd, logic wm, wh, logic [1:0] sm, logic sh, pq);
    @(posedge clk);
    round = rnd;
    wr_m = wm;
    wr_h = wh;
    sel_m = sm;
    sel_h = sh;
    sel_pq = pq;
  endtask

  initial begin
    // set the initial system state
    fp_sim = $fopen("simulation.txt", "w");

    if (!$value$plusargs("iterations=%d", iterations))
        iterations = 1000;

    // initialize the fixed chaining value
    for (int i = 0; i < 64; i++) begin
        h_in = { h_in[503:0], 8'b0 };
        h_in[7:0] = i;
    end

    for (int i = 0; i < iterations; i++) begin
      // create a random 512-bit message block and chaining value
      automatic iteration_state msg = new;
      void'(msg.randomize());

      // display and record the timestamp at the start of compression
      $display("%10d - iter %0d", $time, i);
      $fwrite(fp_sim, "%10d %0128X %0128X ", $time, msg.msg, h_in);

      // initialize the permute function with a 512-bit message block
      m_in = msg.msg;

      drive(0, 1, 1, 2'b00, 0, 0); // 0: latch H & M
      drive(0, 1, 0, 2'b10, 0, 1); // 1: Q-S1, M^=H

      m_in = 'x;

      drive(0, 1, 0, 2'b01, 0, 0); // 2: Q-S2, P-S1

//    for (int r = 1; r < 10; r++) begin
//      drive(r, 1, 0, 2'b01, 0, 1); // Q-S1, P-S2
//      drive(r, 1, 0, 2'b01, 0, 0); // Q-S2, P-S1
//    end

//    drive(0, 1, 1, 2'b01, 1, 1); // 21: P-S2, H^=M(Q_out)
//    drive(0, 1, 1, 2'b10, 1, 0); // 0*: P-S2, H^=M(P_out) (next block)
      @(posedge clk);

      // retrieve the output state and write it to the trace file
      @(negedge clk) $fwrite(fp_sim, "%0128X\n", dout);
    end

    // close the file descriptor and terminate the simulation
    $fclose(fp_sim);
    $finish;
  end
endmodule

