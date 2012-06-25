dump -file dump.vpd 
dump -add /testbench/dut -depth 0
dump -autoflush on
dump -deltaCycle on
dump -forceEvent on
run 280000000
exit
