module D_FF_PipelineReg_28b(regProdIn, regProdOut, clk, reset, en_pipeline_reg); //Register used for pipelining
    input [13:0] regProdIn;
    input clk, reset, en_pipeline_reg;
    output logic [13:0] regProdOut;
    always_ff @(posedge clk) begin
        if (reset)
            regProdOut <= 0;
        else if(en_pipeline_reg)
            regProdOut <= regProdIn;

    end
endmodule 

module D_FF_28b(sum, f, clk, en_acc, clear_acc, reset);     //Accumulator
    input clk, en_acc, clear_acc,  reset;
    input signed [13:0] sum;
    output logic signed [13:0] f;

    always_ff @(posedge clk) begin
        if(reset || clear_acc) begin
            f <= 0;
        end
        else if(en_acc == 1) begin
            f <= sum;
        end
    end
endmodule


module mac_part1(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, a, b, f);
    input clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc;
    input signed [13:0] a, b;

    output logic signed [13:0] f;
    logic signed [13:0] sum;
    logic signed [27:0] prod;
    logic signed [13:0] pipelinedRegOut; 
    logic signed [27:0] pipelinedMultOut;

    parameter multPipelinedStages = 2;
    parameter integer WIDTH = 14; 

    logic [13:0] MIN_VALUE, MAX_VALUE;
    assign MAX_VALUE = 14'd8191;
    assign MIN_VALUE = -14'd8192;  

    always_comb begin
        prod = pipelinedMultOut;
        if(prod > 8191) begin
            prod = MAX_VALUE;
        end
        else if(prod < -8192) begin
            prod = MIN_VALUE;
        end
        
    end

    always_comb begin
        
        sum = pipelinedRegOut + f; 
        if(pipelinedRegOut[13] && f[13] && ~sum[13]) begin
            sum = MIN_VALUE;  
        end
        else if(~pipelinedRegOut[13] && ~f[13] && sum[13]) begin
            sum = MAX_VALUE;
        end
            
        
    end

    DW_mult_pipe #(WIDTH, WIDTH, multPipelinedStages, 1, 2) pipelinedMultiplier(clk, ~reset, enable_mult, 1'b1, b, a, pipelinedMultOut); 
    D_FF_PipelineReg_28b pipelineReg(prod[13:0], pipelinedRegOut, clk, reset, en_pipeline_reg);
    D_FF_28b D_FF_28b(sum, f, clk, en_acc, clear_acc, reset);

endmodule
