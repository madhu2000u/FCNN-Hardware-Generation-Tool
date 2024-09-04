/*
Authors:
Madhu Sudhanan - 115294248
Suvarna Tirur Ananthanarayanan - 115012264
Date: November 16, 2022
*/


module D_FF_PipelineReg_28b(regProdIn, regProdOut, clk, reset, en_pipeline_reg); //Register used for pipelining
    parameter T =14;

    input [T - 1:0] regProdIn;
    input clk, reset, en_pipeline_reg;
    output logic [T - 1:0] regProdOut;
    always_ff @(posedge clk) begin
        if (reset)
            regProdOut <= 0;
        else if(en_pipeline_reg)
            regProdOut <= regProdIn;

    end
endmodule 

module D_FF_28b(sum, f, clk, en_acc, clear_acc, reset);     //Accumulator
    parameter T =14;

    input clk, en_acc, clear_acc,  reset;
    input signed [T - 1:0] sum;
    output logic signed [T - 1:0] f;

    always_ff @(posedge clk) begin
        if(reset || clear_acc) begin
            f <= 0;
        end
        else if(en_acc == 1) begin
            f <= sum;
        end
    end
endmodule


module mac_part2(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, a, b, f);

    //These parameters are passed from the main module fc_M_N_T_R_P.sv
    parameter multPipelinedStages = 2;
    parameter integer T = 14;
    parameter signed max_value = 64'sh1fff;    
    parameter signed min_value = 64'shffffffffffffe000;

    input clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc;
    input signed [T - 1:0] a, b;

    output logic signed [T - 1:0] f;
    logic signed [T - 1:0] sum;
    logic signed [(2 * T) - 1:0] prod;
    logic signed [T - 1:0] pipelinedRegOut; 
    logic signed [(2 * T) - 1:0] pipelinedMultOut;


    always_comb begin
        prod = pipelinedMultOut;
        if(prod > max_value) begin
            prod = max_value;
        end
        else if(prod < min_value) begin
            prod = min_value;
        end
        
    end

    always_comb begin
        
        sum = pipelinedRegOut + f; 
        if(pipelinedRegOut[T - 1] && f[T - 1] && ~sum[T - 1]) begin
            sum = min_value[T-1:0];  
        end
        else if(~pipelinedRegOut[T - 1] && ~f[T - 1] && sum[T - 1]) begin
            sum = max_value[T-1:0];
        end
            
        
    end

    DW_mult_pipe #(T, T, multPipelinedStages, 1, 2) pipelinedMultiplier(clk, ~reset, enable_mult, 1'b1, b, a, pipelinedMultOut); 
    D_FF_PipelineReg_28b #(T) pipelineReg(prod[T - 1:0], pipelinedRegOut, clk, reset, en_pipeline_reg);
    D_FF_28b #(T) D_FF_28b(sum, f, clk, en_acc, clear_acc, reset);

endmodule
