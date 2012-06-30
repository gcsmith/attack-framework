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

regsub -all {\+define\+} $env(SYN_VDEF) { } def
read_file -format sverilog -define [ list $def ] {                          \
          aes_encrypt_unit.sv                                               \
          aes_add_round_key.sv                                              \
          aes_sbox_logic.sv                                                 \
          aes_sbox_lut.sv                                                   \
          aes_dual_row_mem.sv                                               \
          aes_encrypt_control.sv                                            \
          aes_key_mem.sv                                                    \
          aes_mix_columns.sv                                                \
          aes_read_mux.sv                                                   \
          aes_write_mux.sv                                                  \
      }

current_design aes_encrypt_unit
link

list_designs
uniquify -force

#set_clock_gating_style -sequential_cell none -num_stages 3

check_design
compile -map_effort medium

report_cell
report_area -hierarchy

################################################################################
# Generate the gate-level netlist
################################################################################

write -format verilog -hierarchy -output aes_encrypt_unit.vg

exit
