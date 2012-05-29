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
`define ITERATIONS 5000

module testbench;
  logic       clk = 0, rst = 0, valid = 0;
  logic [7:0] din = '0, key = '0, dout;
  int fp_sim;

  const logic [7:0] key_rom[0:15] = '{
      'h00, 'h11, 'h22, 'h33, 'h44, 'h55, 'h66, 'h77,
      'h88, 'h99, 'hAA, 'hBB, 'hCC, 'hDD, 'hEE, 'hFF
  };

  // instantiate the DUT
  rijndael_simple_func dut(clk, rst, valid, din, key, dout);

  // invert the clock signal every 20 ns (25 MHz)
  always #20 clk = ~clk;

  class iteration_state;
    rand bit [0:15][7:0] text;
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
      $fwrite(fp_sim, "%10d %032X ", $time, msg.text);

      // assert and de-assert the reset signal, then wait a few cycles
      @(posedge clk) rst = 1;
      @(posedge clk) rst = 0;
      for (int i = 0; i < 2; i++) @(posedge clk);

      // initialize the cipher with the input plaintext
      valid = 1;
      for (int i = 0; i < 16; i++) begin
        din = msg.text[i];
        key = key_rom[i];
        @(posedge clk);
        @(posedge clk);
        @(negedge clk) $fwrite(fp_sim, "%02X", dout);
      end

      valid = 0;
      @(posedge clk);
      $fwrite(fp_sim, "\n");
    end

    // wait for a few clock cycles, then terminate the simulation
    for (int i = 0; i < 4; i++) @(posedge clk);

    // close the file descriptor and terminate the simulation
    $fclose(fp_sim);
    $finish;
  end
endmodule

