// Peter Milder
// Testbench Generator for Project 3
// This code may not be redistributed

#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <bitset>

using namespace std;

void printUsage();
void genRandomVector(vector<long>& v, int size, int bits);
void computeOutputs(vector<long>& W, vector<long>& x, vector<long>& y, int M, int N, int T, int R);
long saturate(long v, int bits, int flag);
string hexString(long v, int bits);

// Just used for debugging
int multSaturations = 0;
int totalMults = 0;
int addSaturations = 0;
int totalAdds = 0;

int main(int argc, char* argv[]) {

   int approxInputs = 10000;

   srand(time(NULL)); // initialize random number generator

   if (argc < 6) {
      printUsage();
      return 1;
   }

   int mode = atoi(argv[1]);
   int numInputs;
   string constFileName;
   ofstream os;
   string tb_name;
   int T;
   vector<long> outputVals;
   string inFileName, outFileName, dut_name, out_file;

   // Modes 1/2: for parts 1/2: testbench for a single layer
   if (((mode == 1) && (argc == 6)) || ((mode==2) && (argc == 7))) {
      int M = atoi(argv[2]);
      int N = atoi(argv[3]);
      T = atoi(argv[4]);
      int R = atoi(argv[5]);
      int P;

      if (mode == 1) 
         P = 1;
      else
         P = atoi(argv[6]);

      numInputs = approxInputs - (approxInputs%N);
      if (numInputs < N) {
         cout << "ERROR: N > numInputs" << endl;
         return 1;
      }

      // step 1: generate M*N random inputs (W matrix)
      //         store them in vector here, and also output them to a file
      // For the first M-1 rows of the matrix, we will generate random values
      // with max value T/2 (to make saturation rare)
      // In the last row, we will generate larger random values (to make sure)
      // the testbench checks saturation in your multiplier and adder
      
      vector<long> wMatrix_a;
      genRandomVector(wMatrix_a, (M-1)*N, T/2);
      vector<long> wMatrix_b;
      genRandomVector(wMatrix_b, N, T);
      vector<long> wMatrix = wMatrix_a;
      wMatrix.insert(wMatrix.end(), wMatrix_b.begin(), wMatrix_b.end());



      // Store the constants to file
      constFileName = "const_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R)  + "_" + to_string(P)+ ".txt";
      os.open(constFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << constFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<M*N; i++) {
         os << wMatrix[i] << endl;
      }
      os.close();

      // step 2: generate input values
      vector<long> inputVals;
      genRandomVector(inputVals, numInputs, T/2);

      inFileName = "tb_fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P) + ".in";
      os.open(inFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << inFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<numInputs; i++) {
         string s = bitset<32>(inputVals[i]).to_string();         
         os << s.substr(32-T, 32) << endl;
      }
      os.close();


      // step 3: compute expected outputs. Output them to a file
      computeOutputs(wMatrix, inputVals, outputVals, M, N, T, R);

      outFileName = "tb_fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P) + ".exp";
      os.open(outFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << outFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<outputVals.size(); i++) {
         string s = bitset<32>(outputVals[i]).to_string();         
         os << s.substr(32-T, 32) << endl;
      }
      os.close();

      dut_name = "fc_" + to_string(M) + "_" + to_string(N) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(P);
      tb_name = "tb_" + dut_name;
      out_file = tb_name + ".sv";

   }

   // Mode 3: Testbench for three-layer network
   else if ((mode == 3) && (argc == 9)) {
      int N  = atoi(argv[2]);
      int M1 = atoi(argv[3]);
      int M2 = atoi(argv[4]);
      int M3 = atoi(argv[5]);
      T  = atoi(argv[6]);
      int R  = atoi(argv[7]);
      int B  = atoi(argv[8]);

      numInputs = approxInputs - (approxInputs%N);
      if (numInputs < N) {
         cout << "ERROR: N > numInputs" << endl;
         return 1;
      }

      // step 1: generate three random matrices for the three layers
      // store them in vectors and output them to the constant file

      // Now, since data will go through 3 layers, I am making the 
      // W matrix values even smaller to keep saturation relatively rare
      vector<long> wMatrix1, wMatrix2, wMatrix3;
      genRandomVector(wMatrix1, N*M1, T/4);
      genRandomVector(wMatrix2, M1*M2, T/4);
      genRandomVector(wMatrix3, M2*M3, T/4);

      constFileName = "const_" + to_string(N) + "_" + to_string(M1) + "_" + to_string(M2) + "_" + to_string(M3) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(B) + ".txt";

      os.open(constFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << constFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<M1*N; i++) 
         os << wMatrix1[i] << endl;
      for (int i=0; i<M2*M1; i++) 
         os << wMatrix2[i] << endl;
      for (int i=0; i<M3*M2; i++) 
         os << wMatrix3[i] << endl;
      os.close();

      // step 2: generate input values
      vector<long> inputVals;
      genRandomVector(inputVals, numInputs, T/2);

      dut_name = "net_" + to_string(N) + "_" + to_string(M1) + "_" + to_string(M2) + "_" + to_string(M3) + "_" + to_string(T) + "_" + to_string(R) + "_" + to_string(B);
      tb_name = "tb_" + dut_name;
      inFileName = tb_name + ".in";
      out_file = tb_name + ".sv";

      os.open(inFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << inFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<numInputs; i++) {
         string s = bitset<32>(inputVals[i]).to_string();         
         os << s.substr(32-T, 32) << endl;
      }
      os.close();

      // step 3: compute expected outputs
      vector<long> intermediateVals1, intermediateVals2;
      computeOutputs(wMatrix1, inputVals,         intermediateVals1, M1, N,  T, R);
      computeOutputs(wMatrix2, intermediateVals1, intermediateVals2, M2, M1, T, R);
      computeOutputs(wMatrix3, intermediateVals2, outputVals,        M3, M2, T, R);


      outFileName = tb_name + ".exp";
      os.open(outFileName);
      if (!os.is_open()) {
         cout << "ERROR opening " << outFileName << " for writing." << endl;
         return 1;
      }
      for (int i=0; i<outputVals.size(); i++) {
         string s = bitset<32>(outputVals[i]).to_string();         
         os << s.substr(32-T, 32) << endl;
      }
      os.close();

   }
   else {
      printUsage();
      return 1;
   }

   // step 4: generate the testbench .sv file
   // To do this, I wrote a template called tbtemplate.txt.
   // This is the .sv file we want but a number of things need to be filled in based on
   // this design. So, we will use the command line tool "sed" to do the substituions.
   // This code could run outside of our program in a shell script, but it's very convenient
   // to include it here.
   // This will only work in an environment like Linux, Unix, or MacOS where you have 
   // a standard shell and command line tools. 
   string myCmd = "cat tbtemplate.txt";
   myCmd += "| sed 's/<TBMODULENAME>/" + tb_name + "/g; ";
   myCmd += " s/<NUMBITS>/" + to_string(T)  + "/g;";
   myCmd += " s/<NUMINPUTVALS>/" + to_string(numInputs)  + "/g;";
   myCmd += " s/<NUMOUTPUTVALS>/" + to_string(outputVals.size())  + "/g;";
   myCmd += " s/<INFILENAME>/" +  inFileName + "/g;";
   myCmd += " s/<EXPFILENAME>/" +  outFileName + "/g;";
   myCmd += " s/<DUTNAME>/" +  dut_name + "/g;";
   myCmd += "' > " + out_file;
   system(myCmd.c_str());

   // Just used for debugging -- keep track of total saturations
   // We want to make sure we are testing saturation of multiplication and addition
   // without having *too many* operations saturate
   //cout << "Multiplications: " << multSaturations << "/" << totalMults << " = " << (float)multSaturations/totalMults << " saturated" << endl;
   //cout << "Additions: " << addSaturations << "/" << totalAdds << " = " << (float)addSaturations/totalAdds << " saturated" << endl;


}

// Compute the expected output values for a layer.
void computeOutputs(vector<long>& W, vector<long>& x, vector<long>& y, int M, int N, int T, int R) {
   int iterations = x.size()/N;
   for (int i=0; i<iterations; i++) {
      for (int m=0; m<M; m++) {
         y.push_back(0);
         for (int n=0; n<N; n++) {
            long prod = saturate(W[m*N+n]*x[n+i*N], T, 0);
            y[m+i*M] = saturate(y[m+i*M] + prod, T, 1); 
         }
         if (R==1)
            y[m+i*M] = (y[m+i*M] < 0) ? 0 : y[m+i*M];
      }
   }
}

// Check to see if v will saturate, and if so, return appropriate saturation value
long saturate(long v, int bits, int flag) {
   long maxVal = ((long)1<<(bits-1))-1;
   long minVal = -1*((long)1<<(bits-1));

   // just for debugging
   if (flag==0)
      totalMults++;
   else
      totalAdds++;

   if (v > maxVal) {
      
      // for debugging
      if (flag==0)
         multSaturations++;
      else
         addSaturations++;

      return maxVal;
   }  
   if (v < minVal) {
      
      // for debugging
      if (flag==0)
         multSaturations++;
      else
         addSaturations++;

      return minVal;
   }

   return v;
}
 
// Generate a random vector of the given size with the given number of bits
void genRandomVector(vector<long>& v, int size, int bits) {
   if (bits==32)
      bits--;

   for (int i=0; i<size; i++)
     v.push_back(rand()%(1<<bits)-(1<<(bits-1)));     
}

void printUsage() {
   cout << "usage: ./testgen mode ARGS" << endl;

   cout << "   Mode 1 (Part 1): Produce testbench for one neural network layer (unparallelized)" << endl;
   cout << "      ./testgen 1 M N T R" << endl << endl;

   cout << "   Mode 2 (Part 2): Produce testbench one neural network layer (parallelized)" << endl;
   cout << "      ./testgen 2 M N T R P" << endl << endl;

   cout << "   Mode 3 (Part 3): Produce testbench for a system with three interconnected layers" << endl;
   cout << "      ./testgen 3 N M1 M2 M3 T R B" << endl;

}
