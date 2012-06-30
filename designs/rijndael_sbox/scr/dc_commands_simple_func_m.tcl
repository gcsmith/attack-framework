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
          rijndael_sbox_masked.sv                                           \
          rijndael_simple_func_m.sv                                         \
      }

current_design rijndael_simple_func_m
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

write -format verilog -hierarchy -output rijndael_simple_func_m.vg

exit
