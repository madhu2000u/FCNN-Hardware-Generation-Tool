// ESE 507 Project 3 Handout Code
// You may not redistribute this code

// Getting started:
// The main() function contains the code to read the parameters. 
// For Parts 1 and 2, your code should be in the genFCLayer() function. Please
// also look at this function to see an example for how to create the ROMs.
//
// For Part 3, your code should be in the genNetwork() function.



#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <math.h>
using namespace std;

// Function prototypes
void printUsage();
void genFCLayer(int M, int N, int T, int R, int P, vector<int>& constvector, string modName, string out_file, ofstream &os);
void genNetwork(int N, int M1, int M2, int M3, int T, int R, int B, vector<int>& constVector, string modName, ofstream &os);
void readConstants(ifstream &constStream, vector<int>& constvector);
void genROM(int M, int N, int P, vector<int>& constVector, int bits, string modName, ofstream &os);

int global_mode = 0;

int main(int argc, char* argv[]) {

   // If the user runs the program without enough parameters, print a helpful message
   // and quit.
   if (argc < 7) {
      printUsage();
      return 1;
   }

   int mode = atoi(argv[1]);
   global_mode = mode;

   ifstream const_file;
   ofstream os;
   vector<int> constVector;

   //----------------------------------------------------------------------
   // Look here for Part 1 and 2
   if (((mode == 1) && (argc == 7)) || ((mode == 2) && (argc == 8))) {

      // Mode 1/2: Generate one layer with given dimensions and one testbench

      // --------------- read parameters, etc. ---------------
      int M = atoi(argv[2]);
      int N = atoi(argv[3]);
      int T = atoi(argv[4]);
      int R = atoi(argv[5]);

      int P;

      // If mode == 1, then set P to 1. If mode==2, set P to the value
      // given at the command line.
      if (mode == 1) {
         P=1;
         std::string arg = argv[6];
         std::string name = arg;
         const_file.open(name);         
      }
      else {
         P = atoi(argv[6]);
         const_file.open(argv[7]);
      }

      if (const_file.is_open() != true) {
         cout << "ERROR reading constant file " << argv[6] << endl;
         return 1;
      }

      // Read the constants out of the provided file and place them in the constVector vector
      readConstants(const_file, constVector);

      string rom_base_name = "fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P);
      string rom_file_name_extension = "_W_rom";
      
      string rom_out_file = rom_base_name + rom_file_name_extension + ".sv";

      os.open(rom_out_file);
      if (os.is_open() != true) {
         cout << "ERROR opening " << rom_out_file << " for write." << endl;
         return 1;
      }
      // -------------------------------------------------------------

      // call the genFCLayer function you will write to generate this layer
      string romModName = rom_base_name + rom_file_name_extension;
      genFCLayer(M, N, T, R, P, constVector, romModName, rom_out_file, os); 

   }
   //--------------------------------------------------------------------


   // ----------------------------------------------------------------
   // Look here for Part 3
   else if ((mode == 3) && (argc == 10)) {
      // Mode 3: Generate three layers interconnected

      // --------------- read parameters, etc. ---------------
      int N  = atoi(argv[2]);
      int M1 = atoi(argv[3]);
      int M2 = atoi(argv[4]);
      int M3 = atoi(argv[5]);
      int T  = atoi(argv[6]);
      int R  = atoi(argv[7]);
      int B  = atoi(argv[8]);

      const_file.open(argv[9]);
      if (const_file.is_open() != true) {
         cout << "ERROR reading constant file " << argv[8] << endl;
         return 1;
      }
      readConstants(const_file, constVector);

      string out_file = "net_" + to_string(N) + "_" + to_string(M1) + "_" + to_string(M2) + "_" + to_string(M3) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(B)+ ".sv";


      os.open(out_file);
      if (os.is_open() != true) {
         cout << "ERROR opening " << out_file << " for write." << endl;
         return 1;
      }
      // -------------------------------------------------------------

      string mod_name = "net_" + to_string(N) + "_" + to_string(M1) + "_" + to_string(M2) + "_" + to_string(M3) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(B);

      // generate the design
      genNetwork(N, M1, M2, M3, T, R, B, constVector, mod_name, os);

   }
   //-------------------------------------------------------

   else {
      printUsage();
      return 1;
   }

   // close the output stream
   os.close();

}

// Read values from the constant file into the vector
void readConstants(ifstream &constStream, vector<int>& constvector) {
   string constLineString;
   while(getline(constStream, constLineString)) {
      int val = atoi(constLineString.c_str());
      constvector.push_back(val);
   }
}

// Generate a ROM based on values constVector.
// Values should each be "bits" number of bits.
void genROM(int M, int N, int P, vector<int>& constVector, int bits, string modName, ofstream &os) {

      int numWords = constVector.size();
      int addrBits = ceil(log2(numWords));

      if(global_mode == 1){
         os << "module " << modName << "(clk, addr, z);" << endl;
         os << "   input clk;" << endl;
         os << "   input [" << addrBits-1 << ":0] addr;" << endl;
         os << "   output logic signed [" << bits-1 << ":0] z;" << endl;
         os << "   always_ff @(posedge clk) begin" << endl;
         os << "      case(addr)" << endl;
         int i=0;
         for (vector<int>::iterator it = constVector.begin(); it < constVector.end(); it++, i++) {
            if (*it < 0)
               os << "        " << i << ": z <= -" << bits << "'d" << abs(*it) << ";" << endl;
            else
               os << "        " << i << ": z <= "  << bits << "'d" << *it      << ";" << endl;
         }
         os << "      endcase" << endl << "   end" << endl << "endmodule" << endl << endl;
      }
      else if(global_mode == 2){
         addrBits = ceil(log2(numWords/P));
         int init_addr, offset_addr, effective_addr = 0;
         for(int p = 0; p < P; p++){      //Loop for each rom module
            //int addr_scale = 0;
            int case_count = 0;           //case statment value for each rom module
            os << "module " << modName + to_string(p) << "(clk, addr, z);" << endl;
            os << "   parameter T = 4;" << endl;
            os << "   input clk;" << endl;
            os << "   input [T - 1:0] addr;" << endl;
            os << "   output logic signed [" << bits-1 << ":0] z;" << endl;
            os << "   always_ff @(posedge clk) begin" << endl;
            os << "      case(addr)" << endl;

            for(int rom_sets = 0; rom_sets < M/P; rom_sets++){    //each rom module has N * (M/P) values
               init_addr = N * p;
               offset_addr = (P * N * rom_sets);                  //calculate the offset addresses of the starting value from the constVector that must go into rom module p
               effective_addr = init_addr + offset_addr;
               vector<int>::iterator it = constVector.begin() + effective_addr;
               for(int n = 0; n < N; n++){      //iterate N times from the effective address calculated and put them inside the rom module p
                  if (*it < 0)
                     os << "        " << case_count  << ": z <= -" << bits << "'d" << abs(*it) << ";" << endl;
                  else
                     os << "        " << case_count << ": z <= "  << bits << "'d" << *it      << ";" << endl;
                  
                  it++;
                  case_count++;
               }
               //addr_scale++;
            }
            os << "      endcase" << endl << "   end" << endl << "endmodule" << endl << endl;
            
         }
      }
}

// Parts 1 and 2
// Here is where you add your code to produce a neural network layer.
void genFCLayer(int M, int N, int T, int R, int P, vector<int>& constVector, string romModName, string rom_out_file, ofstream &os) {

   // os << "module " << modName << "();" << endl;
   // os << "   // your stuff here!" << endl;
   // os << "endmodule" << endl << endl;
   long maxVal = ((long)1<<(T-1))-1;
   long minVal = ((long)1<<(T-1));

   string max_value = to_string(T) + "'sd" + to_string(maxVal);
   string min_value = "-" + to_string(T) + "'sd" + to_string(minVal);

   string mainModName = "fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P);
   string out_file = "fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P) + ".sv";

   string mac_output = "mac_output";
   string output_data = "output_data";
   if(global_mode == 1){
      string myCmd = "cat matvec_part1_template.sv";
      myCmd += "| sed 's/<ROM.sv>/" + rom_out_file + "/g; ";
      myCmd += " s/<MODULENAME>/" + mainModName  + "/g;";
      myCmd += " s/<M>/" + to_string(M)  + "/g;";
      myCmd += " s/<N>/" + to_string(N)  + "/g;";
      myCmd += " s/<T>/" + to_string(T)  + "/g;";
      myCmd += " s/<ReLU>/" +  to_string(R) + "/g;";
      myCmd += " s/<max_value>/" + to_string(maxVal) + "/g;";
      myCmd += " s/<min_value>/" + to_string(minVal) + "/g;";
      if(R){
         myCmd += " s/<ReLU_output>/" + mac_output  + "/g;";
      }
      else{
         myCmd += " s/<ReLU_output>/" +  output_data + "/g;";
      }
      myCmd += "' > " + out_file;
      system(myCmd.c_str());
   }
   else if(global_mode == 2){

      

      string myCmd = "cat matvec_part2_template.sv";
      myCmd += "| sed 's/<ROM.sv>/" + rom_out_file + "/g; ";
      myCmd += " s/<MODULENAME>/" + mainModName  + "/g;";
      myCmd += " s/<M>/" + to_string(M)  + "/g;";
      myCmd += " s/<N>/" + to_string(N)  + "/g;";
      myCmd += " s/<T>/" + to_string(T)  + "/g;";
      myCmd += " s/<P>/" + to_string(P)  + "/g;";
      myCmd += " s/<ReLU>/" +  to_string(R) + "/g;";
      myCmd += " s/<max_value>/" + to_string(maxVal) + "/g;";
      myCmd += " s/<min_value>/" + to_string(minVal) + "/g;";
      

      string x = "";
      for(int i = 0; i < P; i++){
         x += "mac_output" + to_string(i) + ((i != P-1) ? "," : "");
      }
      myCmd += " s/<MAC_OUTPUT_TEMPLATE>/" + x + ";" + "/g;";
      myCmd += " s/<MAC_OUTPUT_PARAM_TEMPLATE>/" + x + "/g;";

      x = "";
      for(int i = 0; i < P; i++){
         x +="matrixMem_data_out" + to_string(i) + ((i != P-1) ? "," : ";");
      }
      myCmd += " s/<MATRIX_DATA_OUT_TEMPLATE>/" + x + "/g;";

      x = "";
      for(int i = 0; i < P; i++){
         x += to_string(i) + " : muxOutput = mac_output" + to_string(i) + ";";
      }
      myCmd += " s/<MUX_TEMPLATE>/" + x + "/g;";

      x = "";
      for(int i = 0; i < P; i++){
         x += romModName + to_string(i) + "#(ADDR_W_SIZE) rom" + to_string(i) + "(clk, addr_w, matrixMem_data_out" + to_string(i) + ");";
      }
      myCmd += " s/<ROM_TEMPLATE>/" + x + "/g;";

      x = "";
      for(int i = 0; i < P; i++){
         x += "mac_part2 #(pipelineStages, T, max_value, min_value) macUnit" + to_string(i) + "(clk, reset, en_acc, en_pipeline_reg, enable_mult, clear_acc, vectorMem_data_out, matrixMem_data_out" + to_string(i) + ", mac_output" + to_string(i) + ");";
      }
      myCmd += " s/<MAC_UNIT_TEMPLATE>/" + x + "/g;";
      myCmd += "' > " + out_file;
      system(myCmd.c_str());
   }

   // You will need to generate ROM(s) with values from the pre-stored constant values.
   // Here is code that demonstrates how to do this for the simple case where you want to put all of
   // the matrix values W in one ROM. This is probably what you will need for P=1, but you will want 
   // to change this for P>1. Please also see some examples of splitting these vectors in the Part 3
   // code.


   // Check there are enough values in the constant file.
   if (M*N != constVector.size()) {
      cout << "ERROR: constVector does not contain correct amount of data for the requested design" << endl;
      cout << "The design parameters requested require " << M*N+M << " numbers, but the provided data only have " << constVector.size() << " constants" << endl;
      assert(false);
   }

   // Generate a ROM (for W) with constants 0 through M*N-1, with T bits
   //string romModName = modName + "_W_rom";
   genROM(M, N, P, constVector, T, romModName, os);

}

// Part 3: Generate a hardware system with three layers interconnected.
// Layer 1: Input length: N, output length: M1
// Layer 2: Input length: M1, output length: M2
// Layer 3: Input length: M2, output length: M3
// B is the number of multipliers your overall design may use.
// Your goal is to build the fastest design that uses B or fewer multipliers
// constVector holds all the constants for your system (all three layers, in order)
void genNetwork(int N, int M1, int M2, int M3, int T, int R, int B, vector<int>& constVector, string modName, ofstream &os) {

   // Here you will write code to figure out the best values to use for P1, P2, and P3, given
   // B. 
   int P1 = 1; // replace this with your optimized value
   int P2 = 1; // replace this with your optimized value
   int P3 = 1; // replace this with your optimized value

   // output top-level module
   os << "module " << modName << "();" << endl;
   os << "   // this module should instantiate three layers and wire them together" << endl;
   os << "endmodule" << endl;
   
   // -------------------------------------------------------------------------
   // Split up constVector for the three layers
   // layer 1's W matrix is M1 x N
   int start = 0;
   int stop = M1*N;
   vector<int> constVector1(&constVector[start], &constVector[stop]);

   // layer 2's W matrix is M2 x M1
   start = stop;
   stop = start+M2*M1;
   vector<int> constVector2(&constVector[start], &constVector[stop]);

   // layer 3's W matrix is M3 x M2
   start = stop;
   stop = start+M3*M2;
   vector<int> constVector3(&constVector[start], &constVector[stop]);

   if (stop > constVector.size()) {
      os << "ERROR: constVector does not contain enough data for the requested design" << endl;
      os << "The design parameters requested require " << stop << " numbers, but the provided data only have " << constVector.size() << " constants" << endl;
      assert(false);
   }
   // --------------------------------------------------------------------------


   // generate the three layer modules
   string subModName1 = "l1_fc_" + to_string(M1) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P1);
   
   //genFCLayer(M1, N, T, R, P1, constVector1, subModName1, os);

   string subModName2 = "l2_fc_" + to_string(M2) + "_" + to_string(M1) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P2);
   //genFCLayer(M2, M1, T, R, P2, constVector2, subModName2, os);

   string subModName3 = "l3_fc3_" + to_string(M3) + "_" + to_string(M2) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P3);
   //genFCLayer(M3, M2, T, R, P3, constVector3, subModName3, os);

}


void printUsage() {
  cout << "Usage: ./gen MODE ARGS" << endl << endl;

  cout << "   Mode 1 (Part 1): Produce one neural network layer (unparallelized)" << endl;
  cout << "      ./gen 1 M N T R const_file" << endl << endl;

  cout << "   Mode 2 (Part 2): Produce one neural network layer (parallelized)" << endl;
  cout << "      ./gen 2 M N T R P const_file" << endl << endl;

  cout << "   Mode 3 (Part 3): Produce a system with three interconnected layers" << endl;
  cout << "      ./gen 3 N M1 M2 M3 T R B const_file" << endl;
}
