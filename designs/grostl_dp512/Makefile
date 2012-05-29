QUIET ?= 0
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
# rule to create the gate-level netlist
################################################################################

grostl_$(UNIT).vg: scr/synth_$(UNIT).script
	dc_shell $(SYN_OPTS) -f scr/synth_$(UNIT).script | tee $(CLOG)

################################################################################
# separate rules for compilation, simulation, and power analysis
################################################################################

simv: lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv
	vlogan -sverilog lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv | tee -a $(CLOG)
	vcs -debug testbench +vcs+dumpvars+top.vcd | tee -a $(CLOG)

dump.vpd: simv scr/simv_$(UNIT).do
	./simv -ucli -do scr/simv_$(UNIT).do | tee $(SLOG)

dump.vcd: dump.vpd
	vpd2vcd +includemda dump.vpd dump.vcd | tee -a $(SLOG)

power_waveform_orig.out: dump.vcd scr/ptx_orig_$(UNIT).script
	pt_shell $(SYN_OPTS) -f scr/ptx_orig_$(UNIT).script | tee -a $(SLOG)

.PHONY: power_orig
power_orig: power_waveform_orig.out

################################################################################
# combined rule for piping VCS simulation data to PrimeTime
################################################################################

power_waveform.out: lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv scr/ptx_$(UNIT).script
	vlogan -sverilog lib/core.v grostl_$(UNIT).vg tb/test_$(UNIT).sv | tee -a $(CLOG)
	pt_shell $(SYN_OPTS) -f scr/ptx_$(UNIT).script | tee -a $(SLOG)

.PHONY: power
power: power_waveform.out

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

