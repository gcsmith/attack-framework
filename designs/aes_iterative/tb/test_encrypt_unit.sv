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
  logic       clk = 0, rst = 0, valid_in = 0, valid_out;
  logic [7:0] din = '0, dout;
  logic [3:0] rnd;
  int         fp_sim, iterations, max_rounds;

  aes_encrypt_unit dut(clk, rst, valid_in, din, valid_out, dout, rnd);

  // invert the clock signal every 20 ns
  always #20 clk = ~clk;

  class iteration_state;
    rand bit [0:15][7:0] text;
  endclass

  initial begin
    // set the initial system state
    fp_sim = $fopen("simulation.txt", "w");
    @(posedge clk);

    if (!$value$plusargs("iterations=%d", iterations))
        iterations = 1000;
    if (!$value$plusargs("max_rounds=%d", max_rounds))
        max_rounds = 2;

    for (int i = 0; i < iterations; i++) begin
      // create a random 128-bit message block
      automatic iteration_state msg = new;
      void'(msg.randomize());

      // display and record the timestamp at the start of encryption
      $display("%10d - encryption %0d", $time, i);
      $fwrite(fp_sim, "%10d %032X ", $time, msg.text);

      // assert and de-assert the reset signal, then wait a few cycles
      @(posedge clk) rst = 1;
      @(posedge clk) rst = 0;
      for (int i = 0; i < 2; i++) @(posedge clk);

      // initialize the cipher with the input plaintext
      valid_in = 1;
      for (int i = 0; i < 16; i++) begin
        din = msg.text[i];
        @(posedge clk);
      end
      valid_in = 0;

      // retrieve the output ciphertext
      wait(rnd == max_rounds || valid_out == 1);
      for (int i = 0; i < 16; i++) @(posedge clk) msg.text[i] = dout;
      @(posedge clk);

      $fwrite(fp_sim, "%032X\n", msg.text);
    end

    // wait for a few clock cycles, then terminate the simulation
    for (int i = 0; i < 4; i++) @(posedge clk);

    // close the file descriptor and terminate the simulation
    $fclose(fp_sim);
    $finish;
  end
endmodule

