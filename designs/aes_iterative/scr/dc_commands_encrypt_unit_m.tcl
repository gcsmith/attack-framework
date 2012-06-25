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
read_file -format sverilog "aes_encrypt_unit_m.sv"
read_file -format sverilog "aes_add_round_key.sv"
read_file -format sverilog "aes_sbox_masked.sv"
read_file -format sverilog "aes_dual_row_mem.sv"
read_file -format sverilog "aes_encrypt_control.sv"
read_file -format sverilog "aes_key_mem.sv"
read_file -format sverilog "aes_mix_columns.sv"
read_file -format sverilog "aes_read_mux.sv"
read_file -format sverilog "aes_write_mux.sv"

current_design aes_encrypt_unit_m
link

uniquify -force

#set_clock_gating_style -sequential_cell none -num_stages 3

check_design
compile -map_effort medium

################################################################################
# Generate the gate-level netlist
################################################################################

write -format verilog -hierarchy -output aes_encrypt_unit_m.vg

exit