The scrub program uses glog (google log) library to generate the log file.
glog need to be installed with glog_install included in this repository

(note that a glog directory already existed in the repository, however you 
need to delete it and re-installed with glog_install after you clone the 
repository)

If you have installed the glog library in the same folder with scrub.cpp:
you can:

compile the proram with:

mpic++ -o SCRUB -I glog/include -L glog/lib/ -lglog scrub.cpp

To run the executable, use the following code:

GLOG_logtostderr=1  mpirun -np 16 SCRUB "data10k.txt" 2>log

In case of glog is not installed, I included a no logging version where all 
logging functions are commentted out

To compile the no log version, run:

mpic++ -o SCRUB_nl scrub_no_log.cpp

To run the executable, use the following code:

mpirun -np 16 SCRUB_nl "data10k.txt" 2>log

NOTE:
You can change data10k.txt to any other data file.
For file size less than 50k, set np less than or equal to 16

Normality test:

The SCRUB program will do a normality test to the signal data and output the
result to normality_test_result.txt. 

However, a NORMAL program is also available in case that user wants to test 
the normality of input data without scrubbing.

To compile:
mpic++ -o NORMAL norm.cpp

To run the program:
mpirun -np 16 NORMAL "signal.txt"

