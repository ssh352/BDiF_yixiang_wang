For OpenMPI on local computer

To compile, run:

mpic++ -o scrub -I glog/include -L glog/lib/ -lglog scrub.cpp

To run the executable, use the following code:

GLOG_logtostderr=1  mpirun -np 16 scrub "data10k.txt" 2>log


For MPI on penzias

(for some reason glog library doesn't work on penzias. 
All logging functions are commentted out in the penzias version)

To compile, run:

mpic++ -o scrub_p scrub_penzias.cpp

To run the executable, use the following code:

mpirun -np 16 scrub_p "data10k.txt" 2>log


NOTE:
You can change data10k.txt to any other data file.
For file size less than 50k, set np less than or equal to 16


