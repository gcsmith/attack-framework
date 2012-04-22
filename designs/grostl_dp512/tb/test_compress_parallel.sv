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
  logic clk = 0, wr = 0;
  logic   [3:0] round = 0;
  logic [511:0] h_in = 0, m_in = 0, dout;
  int fp_sim;

  // instantiate the DUT
  grostl_compress_parallel dut(clk, wr, round, m_in, h_in, dout);

  // invert the clock signal every 20 ns (25 MHz)
  always #20 clk = ~clk;

  class iteration_state;
    rand bit [511:0] chain;
    rand bit [511:0] msg;
  endclass

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
      h_in = msg.chain;
      m_in = msg.msg;
      round = 0;
      @(posedge clk) wr = 1;
      @(posedge clk) wr = 0;

      for (int r = 0; r < 10; r++) begin
        round = round + 1;
        @(posedge clk);
      end

      // retrieve the output state and write it to the trace file
      $fwrite(fp_sim, "%0128X\n", dout);
    end

    // close the file descriptor and terminate the simulation
    $fclose(fp_sim);
    $finish;
  end
endmodule

