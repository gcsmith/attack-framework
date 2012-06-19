QUIET ?= 0
AMD64 ?= 0
SEED ?= random
UNIT ?= simple_func
PLUS ?=
CLOG = compile.log
SLOG = simulate.log

ifeq ($(QUIET), 1)
    CLOG += &>/dev/null
    SLOG += &>/dev/null
endif

ifeq ($(AMD64), 1)
    export SYN_ARCH = -64
else
    export SYN_ARCH =
endif

ifeq ($(SEED), random)
    export SYN_PLUS = $(PLUS) +ntb_random_seed_automatic
else
    export SYN_PLUS = $(PLUS) +ntb_random_seed=$(SEED)
endif

################################################################################
# rule to create the gate-level netlist
################################################################################

rijndael_$(UNIT).vg: scr/dc_commands_$(UNIT).tcl lib/core_typ.db
	dc_shell $(SYN_ARCH) -f $< | tee $(CLOG)

DESIGN_HDL = lib/core.v rijndael_$(UNIT).vg tb/test_$(UNIT).sv

################################################################################
# separate rules for compilation, simulation, and power analysis
################################################################################

simv: $(DESIGN_HDL)
	vlogan -sverilog $^ | tee -a $(CLOG)
	vcs -debug testbench +vcs+dumpvars+top.vcd | tee -a $(CLOG)

dump.vpd: scr/simv_commands_$(UNIT).tcl simv
	./simv -ucli -do $< $(SYN_PLUS) | tee $(SLOG)

dump.vcd: dump.vpd
	vpd2vcd +includemda $< $@ | tee -a $(SLOG)

power_waveform.fsdb: scr/pt_commands_$(UNIT).tcl dump.vcd
	PT_PIPE=0 pt_shell $(SYN_ARCH) -f $< | tee -a $(SLOG)

power_waveform_orig.out: power_waveform.fsdb
	fsdb2ns -fmt out -o $@ $<

.PHONY: power_orig
power_orig: power_waveform_orig.out

################################################################################
# combined rule for piping VCS simulation data to PrimeTime
################################################################################

power_waveform.out: scr/pt_commands_$(UNIT).tcl $(DESIGN_HDL)
	vlogan -sverilog $(DESIGN_HDL) | tee -a $(CLOG)
	PT_PIPE=1 pt_shell $(SYN_ARCH) -f $< | tee -a $(SLOG)

simulation_results.tar.bz2: power_waveform.out
	tar -cvjf $@ $^ simulation.txt

.PHONY: power
power: power_waveform.out

.PHONY: power_bz2
power_bz2: simulation_results.tar.bz2

################################################################################
# targets for modelsim (vsim) simulation
################################################################################

.PHONY: vsim
vsim: 
	vsim -c -do scr/vsim_commands_$(UNIT).tcl

.PHONY: vgui
vgui: 
	vsim -do scr/vsim_commands_$(UNIT).tcl

.PHONY: clean
clean:
	rm -rf AN.DB 64 csrc work simv simv.daidir transcript* *.txt *.out *.vg \
		*.ini *.key *.wlf *.log *.vcd *.vpd *.svf *.fsdb fsdb2nsLog

