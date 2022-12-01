SRCCPP = main.cc
EXECNAME = gen
TBSRC = testgen.cc
TBEXEC = testgen
FLAGS = -std=c++11 -O3

all: generator tester
throughput: throughput_tester generator

generator:
	g++ $(SRCCPP) -o $(EXECNAME) $(FLAGS)

tester:
	g++ $(TBSRC) -o $(TBEXEC) $(FLAGS)
 
throughput_tester:
	g++ testgen_throughput.cc -o throughput_testgen $(FLAGS)

clean:
	rm -f $(EXECNAME) $(TBEXEC)

# use with caution: deletes all generated designs. If you have other files in
# your directory that have names related to these, be very careful
cleangencode:
	rm -f const_* tb_* net_*.sv fc_*.sv l1_*.sv l2_*.sv l3_*.sv
