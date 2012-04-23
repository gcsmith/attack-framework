################################################################################
# Link design
################################################################################

set search_path {rtl tb lib .}
set target_library {core_typ.db}
set link_library {* core_typ.db}

################################################################################
# Compile HDL sources
################################################################################

set compile_clock_gating_through_hierarchy true
read_file -format sverilog "rtl/grostl_add_constant.sv"
read_file -format sverilog "rtl/grostl_add_constant_p.sv"
read_file -format sverilog "rtl/grostl_add_constant_q.sv"
read_file -format sverilog "rtl/grostl_compress_serial_m.sv"
read_file -format sverilog "rtl/grostl_mix_bytes.sv"
read_file -format sverilog "rtl/grostl_sbox_masked.sv"
read_file -format sverilog "rtl/grostl_shift_bytes.sv"
read_file -format sverilog "rtl/grostl_shift_bytes_p.sv"
read_file -format sverilog "rtl/grostl_shift_bytes_q.sv"
read_file -format sverilog "rtl/grostl_sub_bytes_m.sv"

current_design grostl_compress_serial_m
link

uniquify -force

#set_clock_gating_style -sequential_cell none -num_stages 3

check_design
compile -map_effort medium

################################################################################
# Generate the gate-level netlist
################################################################################

write -format verilog -hierarchy -output grostl_compress_serial_m.vg

exit
