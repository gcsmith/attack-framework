UIET ?= 0
AMD64 ?= 0
UNIT ?= compress_serial

ifeq ($(QUIET), 1)
    CLOG = compile_$(UNIT).log &>/dev/null
    SLOG = simulate_$(UNIT).log &>/dev/null
else
    CLOG = compile_$(UNIT).log
    SLOG = simulate_$(UNIT).log
endif

ifeq ($(AMD64), 1)
    SYN_OPTS = -64
else
    SYN_OPTS =
endif

################################################################################
# rule to create the gate-level netlist for aes_encrypt_unit
################################################################################

grostl_$(UNIT).vg: scr/synth_$(UNIT).script
	dc_shell $(SYN_OPTS) -f scr/synth_$(UNIT).script | tee $(CLOG)

################################################################################
# separate rules for compilation, simulation, and power analysis
################################################################################

simv: lib/core.v aes_encrypt_unit.vg tb/testbench.sv
	vlogan -sverilog lib/core.v aes_encrypt_unit.vg tb/testbench.sv | tee -a $(CLOG)
	vcs -debug testbench +vcs+dumpvars+top.vcd | tee -a $(CLOG)

dump.vpd: simv scr/simv.do
	./simv -ucli -do scr/simv.do | tee $(SLOG)

dump.vcd: dump.vpd
	vpd2vcd +includemda dump.vpd dump.vcd | tee -a $(SLOG)

power_waveform.out: dump.vcd scr/power.script
	pt_shell $(SYN_OPTS) -f scr/power.script | tee -a $(SLOG)

.PHONY: power
power: power_waveform.out

################################################################################
# combined rule for piping VCS simulation data to PrimeTime
################################################################################

power_waveform_piped.out: lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv
	vlogan -sverilog lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv | tee -a $(CLOG)
	pt_shell $(SYN_OPTS) -f scr/ptx_$(UNIT).script | tee -a $(SLOG)

.PHONY: power_piped
power_piped: power_waveform_piped.out

################################################################################
# targets for modelsim (vsim) simulation
################################################################################

.PHONY: vsim
vsim: 
	vsim -c -do scr/vsim_$(UNIT).do

.PHONY: vsim
vgui: 
	vsim -do scr/vsim_$(UNIT).do

.PHONY: clean
clean:
	rm -rf AN.DB 64 csrc work simv simv.daidir transcript* *.txt *.out *.vg \
		*.key *.wlf *.log *.vcd *.vpd *.svf

