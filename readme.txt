The scrub program uses glog (google log) library to generate the log file.
glog need to be installed with glog_install included in this repository

If you have installed the glog library in the same folder with scrub.cpp:
you can:

compile the proram with:

mpic++ -o scrub -I glog/include -L glog/lib/ -lglog scrub.cpp

To run the executable, use the following code:

GLOG_logtostderr=1  mpirun -np 16 scrub "data10k.txt" 2>log

In case of glog is not installed, I included a no logging version where all 
logging functions are commentte out

To compile the no log version, run:

mpic++ -o scrub_nl scrub_no_log.cpp

To run the executable, use the following code:

mpirun -np 16 scrub_nl "data10k.txt" 2>log


NOTE:
You can change data10k.txt to any other data file.
For file size less than 50k, set np less than or equal to 16


