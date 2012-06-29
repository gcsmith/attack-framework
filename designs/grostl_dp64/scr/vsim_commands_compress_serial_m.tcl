# create and map work library

vlib work
vmap work work

# compile the design and testbench

vlog rtl/grostl_add_constant.sv                 \
     rtl/grostl_compress_serial_m.sv            \
     rtl/grostl_mix_bytes.sv                    \
     rtl/grostl_sbox_masked.sv                  \
     rtl/grostl_shift_bytes.sv                  \
     rtl/grostl_sub_bytes_m.sv                  \
     tb/test_compress_serial_m.sv               \
     $env(SYN_VDEF)

# invoke the simulator

vsim $env(SYN_PLUS) -voptargs="+acc" -t 1ns -lib work work.testbench

# add relevent waveforms to the wave window

add wave -noupdate -format Logic -label "clk" /testbench/clk
add wave -noupdate -format Logic -label "wr_m" /testbench/wr_m
add wave -noupdate -format Logic -label "wr_h" /testbench/wr_h
add wave -noupdate -format Literal -radix Hexadecimal -label "sel_m" /testbench/sel_m
add wave -noupdate -format Logic -label "sel_h" /testbench/sel_h
add wave -noupdate -format Logic -label "sel_d" /testbench/sel_d
add wave -noupdate -format Logic -label "sel_pq" /testbench/sel_pq
add wave -noupdate -divider
 
add wave -noupdate -format Literal -radix Hexadecimal -label "round" /testbench/round
add wave -noupdate -format Literal -radix Hexadecimal -label "column" /testbench/column
add wave -noupdate -format Literal -radix Hexadecimal -label "m_in" /testbench/h_in
add wave -noupdate -format Literal -radix Hexadecimal -label "h_in" /testbench/m_in
add wave -noupdate -format Literal -radix Hexadecimal -label "dout" /testbench/dout
add wave -noupdate -divider

add wave -noupdate -format Literal -radix Hexadecimal -label "m_reg" /testbench/dut/m_reg
add wave -noupdate -format Literal -radix Hexadecimal -label "s_shf" /testbench/dut/s_shf
add wave -noupdate -format Literal -radix Hexadecimal -label "d_val" /testbench/dut/d_val
add wave -noupdate -format Literal -radix Hexadecimal -label "s_arc" /testbench/dut/s_arc
add wave -noupdate -format Literal -radix Hexadecimal -label "s_sub" /testbench/dut/s_sub
add wave -noupdate -format Literal -radix Hexadecimal -label "p_reg" /testbench/dut/p_reg
add wave -noupdate -format Literal -radix Hexadecimal -label "s_mix" /testbench/dut/s_mix
add wave -noupdate -format Literal -radix Hexadecimal -label "h_reg" /testbench/dut/h_reg
add wave -noupdate -divider

add wave -noupdate -format Literal -radix Hexadecimal -label "m_val" /testbench/dut/m_val
add wave -noupdate -format Literal -radix Hexadecimal -label "h_val" /testbench/dut/h_val
add wave -noupdate -divider

view wave
view structure
view signals

run 30ms

