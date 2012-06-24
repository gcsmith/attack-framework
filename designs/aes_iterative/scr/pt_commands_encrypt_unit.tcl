################################################################################
# Set the power analysis mode
################################################################################

set power_enable_analysis   true
set power_analysis_mode     time_based

################################################################################
# Set library search path, read in the netlist, and link the design
################################################################################

set search_path     "lib ."
set link_library    "* core_typ.db"
read_verilog        aes_encrypt_unit.vg
current_design      aes_encrypt_unit
link

################################################################################
# Run timing analysis, read in constraints and switching activity file
################################################################################

update_timing
read_sdc lib/aes_encrypt_unit.sdc

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

