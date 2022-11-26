`include "<l1>"
`include "<l2>"
`include "<l3>"

module <NETMODULE>(clk, reset, input_valid, input_ready, input_data, output_valid, output_ready, output_data);
   parameter T = <T>;
   input clk, reset, input_valid, output_ready;
   input signed [T - 1:0] input_data;

   output logic signed [T - 1:0] output_data;
   output logic output_valid, input_ready;

   logic signed [T - 1:0] l1_output_data, l2_output_data;
   logic l1_output_valid, l1_output_ready;
   
   logic l2_output_valid, l2_output_ready;


   <l1_MODULENAME> layer1(clk, reset, input_valid, input_ready, input_data, l1_output_valid, l1_output_ready, l1_output_data);
   <l2_MODULENAME> layer2(clk, reset, l1_output_valid, l1_output_ready, l1_output_data, l2_output_valid, l2_output_ready, l2_output_data);
   <l3_MODULENAME> layer3(clk, reset, l2_output_valid, l2_output_ready, l2_output_data, output_valid, output_ready, output_data);
endmodule