/*
Authors:
Madhu Sudhanan - 115294248
Suvarna Tirur Ananthanarayanan - 115012264
Date: November 16, 2022
*/


`include "<ROM.sv>"
`include "mac_part2.sv"
`include "memory.sv"

module Counter(clk, reset, clear, enable, countOut);
    parameter WIDTH = 4;
    input clk, reset, clear, enable;
    logic [WIDTH-1:0] countIn;
    output logic [WIDTH-1:0]countOut;


    always_ff @( posedge clk ) begin
        if(reset || clear)
            countOut <= 0;
        else if(enable) begin
            countOut <= countOut + 1;
        end        
    end
    
    
endmodule

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

module Controller(clk, reset, input_valid, output_ready, addr_x, wr_en_x, addr_w, clear_acc, en_acc, en_pipeline_reg, enable_mult, input_ready, output_valid, <MAC_OUTPUT_PARAM_TEMPLATE>, output_data);
    
    parameter M = 5;
    parameter N = 4;
    parameter T = 14;
    parameter P = 2;
    parameter ReLU = 0;
    parameter ADDR_X_SIZE = 2;
    parameter ADDR_W_SIZE = 4;
    parameter WIDTH_MEM_READ_X = 2;         
    parameter WIDTH_MEM_READ_W = 4;         
    parameter WIDTH_MAC = 4; 
    parameter WIDTH_EXEC = 4;
    parameter WIDTH_MAC_SELECT = 4;             

    parameter delay_pipeline_n = 4;
    parameter pipelineStages = 0;
    parameter enable_pipeline_reg_after_initial_delay = pipelineStages + 1;             
    parameter enable_acc_after_initial_delay = enable_pipeline_reg_after_initial_delay + 1;  

    input clk, reset, input_valid, output_ready;
    output logic input_ready, output_valid;

    output logic signed [T - 1:0] output_data;
    input logic signed [T - 1:0] <MAC_OUTPUT_TEMPLATE>;     //mac_output0, mac_output1, mac_output2;

    output logic [ADDR_X_SIZE-1:0] addr_x;  
    output logic [ADDR_W_SIZE-1:0] addr_w;
    output logic wr_en_x, clear_acc, en_acc, en_pipeline_reg, enable_mult; 


    logic operationState;                                       //0 -> writing state, 1 -> reading/execution state
    logic initialExecState;                                     //1 -> initial state in execution (meaing we just switched from writing to execution), 0 -> not initial state
    logic clearState;                                           //1 -> clear state (to clear necessary signals and registers after finishing writing or executing), 0 -> not clear state

    logic clear_cntrMac, enable_cntrMac;             
    logic clear_cntrMemX, enable_cntrReadMem_X;      
    logic clear_cntrMemW, enable_cntrReadMem_W;
    logic clear_cntrExec, enable_cntrExec;
    logic clear_cntrMacSelect, enable_cntrMacSelect;


    logic [WIDTH_MAC-1:0] countMacOut;
    logic [WIDTH_MEM_READ_X-1:0] countMem_X_Out;
    logic [WIDTH_MEM_READ_W-1:0] countMem_W_Out;
    logic [WIDTH_EXEC-1:0] countExecOut;
    logic [WIDTH_MAC_SELECT-1:0] countMacSelect;

    logic [T - 1:0] muxOutput;


    always_ff @( posedge clk ) begin
        if(reset) begin
            output_valid <= 0;
            enable_mult <= 0;
            en_acc <= 0;
            en_pipeline_reg <= 0;
            operationState <= 0;
            clearState <= 0;
            
        end


        if(~operationState) begin               //Disable execution when we switch to writing state
            enable_mult <= 0;
            en_acc <= 0;
            en_pipeline_reg <= 0;
        end


        if(clearState) begin
            if(~operationState) begin
                operationState <= 1;
                enable_mult <= 1;
            end
            else if(operationState) begin
                operationState <= 0;
            end
            en_acc <= 0;
            clearState <= 0;
        end

        else if(~operationState) begin           //when in write mode, we are ready to take in new input
            input_ready <= 1;

            if(input_valid) begin
                if(countMem_X_Out == N-1) begin     //we are writing to vector, once written switch state to execution state (operationState = 1)
                    input_ready <= 0;
                    clearState <= 1;
                    initialExecState <= 1;
                    clear_cntrMac <= 0;
                    clear_cntrExec <= 0;
                    clear_acc <= 0;
                end
            end
        end
        else if(operationState) begin       //read/execution state
            if(initialExecState) begin 
                clear_acc <= 0; 
            end
            

            if((countMacOut == (delay_pipeline_n)) && ~clear_acc) begin     //When output is valid, stall execution by disabling all multipliers, registers and accumulators until output_ready is asserted and value is sampled (check reference #1)
                output_valid <= 1;
                enable_mult <= 0;
                en_acc <= 0;
                en_pipeline_reg <= 0;
            end
        
            if(countMem_W_Out == enable_pipeline_reg_after_initial_delay && initialExecState) begin        //enable the pipeline register after this delay to pass through a valid value
                en_pipeline_reg <= 1;
            end

            if(countMem_W_Out == enable_acc_after_initial_delay && initialExecState) begin                 //enable the accumulator after this delay to pass through a valid value
                en_acc <= 1;
                initialExecState <= 0;
            end       
            

            if(clear_acc) begin      //When cleac_acc is asserted(by another part of the control logic (check reference #1)), it implies output data was sampled and we can un-stall the execution
                enable_mult <= 1;
                en_acc <= 1;
                en_pipeline_reg <= 1;

            end

            if(output_valid && output_ready) begin     //(reference #1). On posedge when output_valid and output_ready is asserted, the valid data is sampled and we clear the accumulator and de-assert output_valid
                if(countMacSelect == P-1) begin
                    clear_acc <= 1;
                    output_valid <= 0;
                    enable_cntrExec <= 1;
                    clear_cntrMac <= 1;
                end
            end
            else begin
                clear_acc <= 0;
                enable_cntrExec <= 0;
                clear_cntrMac <= 0;
            end

            if(countExecOut == M/P) begin       //we have the final value of matrix at this delay and waiting for clear_acc to be asserted once value is sampled and then we switch to clear state
                clear_cntrMac <= 1;
                clear_acc <= 1;
                clear_cntrExec <= 1;
                clearState <= 1;
            end

        end
        
    end

   


    always_comb begin
        addr_w = countMem_W_Out;
        addr_x = countMem_X_Out;

        wr_en_x = 0;
        enable_cntrReadMem_X = 0;
        enable_cntrReadMem_W = 0;
        enable_cntrMac = 0;
        clear_cntrMemX = clearState ? 1 : 0;
        clear_cntrMemW = clearState ? 1 : 0;

        enable_cntrMacSelect = 0;
        clear_cntrMacSelect = (countMacSelect == P) ? 1 : 0;
        if(output_valid && output_ready) begin
            enable_cntrMacSelect = 1;
            if(countMacOut == P-1) begin
                enable_cntrMacSelect = 0;
            end
        end

        
        case (countMacSelect)
            <MUX_TEMPLATE>
            default: muxOutput = 0;
        endcase


        // case (countMacSelect)
        //     0:muxOutput = mac_output0;
        //     1:muxOutput = mac_output1;
        //     2:muxOutput = mac_output2; 
        //     default: muxOutput = 0;
        // endcase

        if(operationState && ~clearState) begin    //read mode
            wr_en_x = input_ready;
            enable_cntrReadMem_X = countExecOut < M ? ~output_valid : 0;
            enable_cntrReadMem_W = ~output_valid;
            enable_cntrMac = en_acc ? 1 : 0;
            

            


            if((countMacOut == (delay_pipeline_n)) && ~clear_acc) begin     //Stall address and delay counters when output_valid = 1 and accumulator is not yet cleared (since value is not sampled yet)
                enable_cntrReadMem_X = 0;
                enable_cntrReadMem_W = 0;

            end

            clear_cntrMemX = (countMem_X_Out == N-1) && (enable_cntrReadMem_X) ? 1 : 0;       //clear vector counter after count N-1 as we don't have any value at address >= N

            
        end
        else if(~operationState && ~clearState) begin                          //write mode
            
            enable_cntrMac = 0;
            
            clear_cntrMemX = (countMem_X_Out == N) || (countExecOut == M)? 1 : 0;         


            if(~input_valid) begin       
                enable_cntrReadMem_X = input_valid;

                wr_en_x = input_valid;
            end
            else if(input_valid) begin
                enable_cntrReadMem_X = input_ready;
                
                wr_en_x = input_ready;
                
            end
        end
            
    end

    Counter #(WIDTH_MAC) cntrMac (clk, reset, clear_cntrMac, enable_cntrMac, countMacOut);
    Counter #(WIDTH_EXEC) cntrExec (clk, reset, clear_cntrExec, enable_cntrExec, countExecOut);
    Counter #(WIDTH_MEM_READ_X) cntrMemX (clk, reset, clear_cntrMemX, enable_cntrReadMem_X, countMem_X_Out);
    Counter #(WIDTH_MEM_READ_W) cntrMemW (clk, reset, clear_cntrMemW, enable_cntrReadMem_W, countMem_W_Out);
    Counter #(WIDTH_MAC_SELECT) cntrMacSelect(clk, reset, clear_cntrMacSelect, enable_cntrMacSelect, countMacSelect);

    generate
        if(ReLU) begin
            ReLU #(T) ReLU(muxOutput, output_data);
        end
        else
            assign output_data = muxOutput;
    endgenerate

endmodule



module <MODULENAME>(clk, reset, input_valid, input_ready, input_data, output_valid, output_ready, output_data);
    parameter M = <M>;
    parameter N = <N>;
    parameter T = <T>;
    parameter P = <P>;
    parameter ReLU = <ReLU>;
    parameter signed max_value = <T>'sd<max_value>;
    parameter signed min_value = -<T>'sd<min_value>;

    parameter SIZE_X = N;
    parameter SIZE_W = M * N;
    parameter pipelineStages = 5;
    parameter d0 = N - 1;                                           //Base delay(including the pipeline register) when multiplier pipeline stages is 0. equal to N(no. of columns) - 1

    localparam delay_pipeline_n = d0;
    localparam enable_pipeline_reg_after_initial_delay = pipelineStages - 1;                        //The delay after with the pipeline register must be enabled in order to not propagate 'x'/junk values into it
    localparam enable_acc_after_initial_delay = enable_pipeline_reg_after_initial_delay + 1;        //The delay after which the accumulator must be enabled in order to not propage 'x'/junk values into it

    localparam ADDR_X_SIZE = $clog2(SIZE_X);
    localparam ADDR_W_SIZE = $clog2(SIZE_W/P);

    parameter WIDTH_MEM_READ_X = ADDR_X_SIZE + 1;                             //Width of counter that writes to memory and reads from memory X (cntrMemX).
    parameter WIDTH_MEM_READ_W = ADDR_W_SIZE + 1;                             //Width of counter that writes to memory and reads from memory W (cntrMemW).
    parameter WIDTH_MAC = $clog2(N) + 1;                                      //Width of Mac counter that track columns executed in matrix (cntrMac).
    parameter WIDTH_EXEC = $clog2(M) + 1;                                     //Width of Execution counter that tracks the rows executed in matrix (cntrExec).
    parameter WIDTH_MAC_SELECT = $clog2(P) + 1;

    input clk, reset, input_valid, output_ready;
    input signed [T - 1:0] input_data;

    logic signed [T - 1:0] vectorMem_data_out;
    logic signed [T - 1:0]  <MATRIX_DATA_OUT_TEMPLATE>      //matrixMem_data_out0,  matrixMem_data_out1,  matrixMem_data_out2;
    logic [ADDR_X_SIZE-1:0] addr_x;
    logic [ADDR_W_SIZE-1:0] addr_w;
    logic wr_en_x;
    logic clear_acc, en_acc, en_pipeline_reg, enable_mult;

    logic signed [T - 1:0] <MAC_OUTPUT_TEMPLATE>
    // logic signed [T - 1:0] mac_output0;
    // logic signed [T - 1:0] mac_output1;
    // logic signed [T - 1:0] mac_output2;
    
    output logic signed [T - 1:0] output_data;
    output logic output_valid, input_ready;

    Controller #(M, N, T, P, ReLU, ADDR_X_SIZE, ADDR_W_SIZE, WIDTH_MEM_READ_X, WIDTH_MEM_READ_W, WIDTH_MAC, WIDTH_EXEC, WIDTH_MAC_SELECT, delay_pipeline_n, pipelineStages, enable_pipeline_reg_after_initial_delay, enable_acc_after_initial_delay) controller(clk, reset, input_valid, output_ready, addr_x, wr_en_x, addr_w, clear_acc, en_acc, en_pipeline_reg, enable_mult, input_ready, output_valid, <MAC_OUTPUT_PARAM_TEMPLATE>, output_data);

    memory #(T, SIZE_X) vectorMem(clk, input_data, vectorMem_data_out, addr_x, wr_en_x);

    <ROM_TEMPLATE>
    // fc_6_4_14_1_3_W_rom0 rom0(clk, addr_w, matrixMem_data_out0);
    // fc_6_4_14_1_3_W_rom1 rom1(clk, addr_w, matrixMem_data_out1);
    // fc_6_4_14_1_3_W_rom2 rom2(clk, addr_w, matrixMem_data_out2);

    <MAC_UNIT_TEMPLATE>
    // mac_part1 #(pipelineStages, T, max_value, min_value) macUnit0(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, vectorMem_data_out, matrixMem_data_out0, mac_output0);
    // mac_part1 #(pipelineStages, T, max_value, min_value) macUnit1(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, vectorMem_data_out, matrixMem_data_out1, mac_output1); 
    // mac_part1 #(pipelineStages, T, max_value, min_value) macUnit2(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, vectorMem_data_out, matrixMem_data_out2, mac_output2); 

    
endmodule