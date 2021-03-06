################################################################################
# Set the power analysis mode
################################################################################

set power_enable_analysis   true
set power_analysis_mode     time_based

################################################################################
# Link design, load constraints and activity file
################################################################################

set search_path     "lib ."
set link_library    "* core_typ.db"
read_verilog        rijndael_simple_func_m.vg
current_design      rijndael_simple_func_m
link

################################################################################
# Run timing analysis, read in constraints and switching activity file
################################################################################

update_timing
read_sdc lib/rijndael_sbox.sdc

if {$env(PT_PIPE) == 0} {
    set waveform_path "power_waveform_old"
    read_vcd -strip_path testbench/dut dump.vcd
} else {
    set waveform_path "power_waveform"
    read_vcd top.vcd -strip_path testbench/dut -pipe_exec \
             "vcs -R testbench +vcs+dumpvars+top.vcd $env(SYN_PLUS)"
}

################################################################################
# Perform power analysis
################################################################################

set_power_analysis_options -waveform_format fsdb            \
                           -waveform_output $waveform_path  \
                           -waveform_interval .01           \
                           -include top

check_power
update_power
report_power
report_power -hierarchy
quit

