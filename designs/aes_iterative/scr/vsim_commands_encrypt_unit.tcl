# create and map work library

vlib work
vmap work work

# compile the design and testbench

vlog rtl/aes_add_round_key.sv
vlog rtl/aes_sbox_lut.sv
vlog rtl/aes_dual_row_mem.sv
vlog rtl/aes_encrypt_control.sv
vlog rtl/aes_key_mem.sv
vlog rtl/aes_mix_columns.sv
vlog rtl/aes_read_mux.sv
vlog rtl/aes_write_mux.sv
vlog rtl/aes_encrypt_unit.sv
vlog tb/test_encrypt_unit.sv

# invoke the simulator

vsim -voptargs="+acc" -t 1ns -lib work work.testbench

# add relevent waveforms to the wave window

add wave -noupdate -format Logic -label "clk" /testbench/clk
add wave -noupdate -format Logic -label "rst" /testbench/rst
add wave -noupdate -divider
 
add wave -noupdate -format Literal -radix Hexadecimal -label "din" /testbench/din
add wave -noupdate -format Logic -label "valid_in" /testbench/valid_in
add wave -noupdate -divider

add wave -noupdate -format Literal -radix Hexadecimal -label "dout" /testbench/dout
add wave -noupdate -format Logic -label "valid_out" /testbench/valid_out
add wave -noupdate -divider

view wave
view structure
view signals

run 30ms

