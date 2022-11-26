module ReLU (ReLU_input, output_data);
    parameter T = 14;

    input logic [T - 1:0] ReLU_input;
    output logic [T - 1:0] output_data;

    always_comb begin
        if(ReLU_input[T - 1])
            output_data = 0;
        else if(~ReLU_input[T - 1])
            output_data = ReLU_input;   
    end
    
endmodule
