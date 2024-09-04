##############################################
# Setup: fill out the following parameters: name of clock signal, clock period (ns),
# reset signal name (if used), name of top-level module, name of source file
set CLK_NAME "clk";
set CLK_PERIOD 1.1;
set RST_NAME "reset";
set TOP_MOD_NAME "fc_6_4_14_1_3";
set SRC_FILE "fc_6_4_14_1_3.sv";
# If you have multiple source files, change the line above to list them all like this:
# set SRC_FILE [list "file1.sv" "file2.sv"];
###############################################

source setupdc.tcl
file mkdir work_synth

# optimize FSMs
set fsm_auto_inferring "true"; 
set fsm_enable_state_minimization "true";

define_design_lib WORK -path work_synth
analyze $SRC_FILE -format sverilog
elaborate -work WORK $TOP_MOD_NAME

###### CLOCKS AND PORTS #######
set CLK_PORT [get_ports $CLK_NAME]
set TMP1 [remove_from_collection [all_inputs] $CLK_PORT]
set INPUTS [remove_from_collection $TMP1 $RST_NAME]
create_clock -period $CLK_PERIOD [get_ports clk]
# set delay for inputs to be 0.2ns
set_input_delay 0.2 -max -clock clk $INPUTS
set_output_delay 0.2 -max -clock clk [all_outputs]

###### OPTIMIZATION #######
set_max_area 0 

###### RUN #####
compile_ultra
report_area
report_power
report_timing
write -f verilog $TOP_MOD_NAME -output gates.v -hierarchy

quit

