# create and map work library

vlib work
vmap work work

# compile the design and testbench

vlog rtl/grostl_add_constant_p.sv
vlog rtl/grostl_add_constant_q.sv
vlog rtl/grostl_compress_parallel.sv
vlog rtl/grostl_mix_bytes.sv
vlog rtl/grostl_sbox_lut.sv
vlog rtl/grostl_shift_bytes_p.sv
vlog rtl/grostl_shift_bytes_q.sv
vlog rtl/grostl_sub_bytes.sv
vlog tb/test_compress_parallel.sv

# invoke the simulator

vsim -voptargs="+acc" -t 1ns -lib work work.testbench

# add relevent waveforms to the wave window

add wave -noupdate -format Logic -label "clk" /testbench/clk
add wave -noupdate -format Logic -label "wr" /testbench/wr
add wave -noupdate -divider
 
add wave -noupdate -format Literal -radix Hexadecimal -label "round" /testbench/round
add wave -noupdate -format Literal -radix Hexadecimal -label "h_in" /testbench/h_in
add wave -noupdate -format Literal -radix Hexadecimal -label "m_in" /testbench/m_in
add wave -noupdate -format Literal -radix Hexadecimal -label "dout" /testbench/dout
add wave -noupdate -divider

add wave -noupdate -format Literal -radix Hexadecimal -label "p_in" /testbench/dut/p_in
add wave -noupdate -format Literal -radix Hexadecimal -label "p_rc" /testbench/dut/p_rc
add wave -noupdate -format Literal -radix Hexadecimal -label "p_sb" /testbench/dut/p_sb
add wave -noupdate -format Literal -radix Hexadecimal -label "p_sh" /testbench/dut/p_sh
add wave -noupdate -format Literal -radix Hexadecimal -label "p_mb" /testbench/dut/p_mb
add wave -noupdate -divider

add wave -noupdate -format Literal -radix Hexadecimal -label "q_in" /testbench/dut/q_in
add wave -noupdate -format Literal -radix Hexadecimal -label "q_rc" /testbench/dut/q_rc
add wave -noupdate -format Literal -radix Hexadecimal -label "q_sb" /testbench/dut/q_sb
add wave -noupdate -format Literal -radix Hexadecimal -label "q_sh" /testbench/dut/q_sh
add wave -noupdate -format Literal -radix Hexadecimal -label "q_mb" /testbench/dut/q_mb
add wave -noupdate -divider

view wave
view structure
view signals

run 30ms

