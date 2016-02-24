#include<stdio.h>
#include "mpi.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>
using namespace std;

struct time_struct {

	double min;
	double sec;

};

struct record {
	 time_struct time;//use a double to represent the time
	float price;
	int size;
};


time_struct timeConvert(string source) {

	string::iterator it;
	string temp;
	for (it = source.begin(); (*it) != '.'; it++) {
		if ((*it) != ':') temp += *it;
	}
	double d1 = stod(temp);
	temp.clear();
	for (; it != source.end(); it++) temp += *it;
	double d2 = stod(temp);

	time_struct temp_t;
	temp_t.min = d1;
	temp_t.sec = d2;
	return temp_t;
}

vector<record> string2record(char a[]) {
	string temp = a;
	string buffer;
	vector<record> rec_vec;
	string::iterator it = temp.begin();
	//skip the first first record since it is likely to be broken
	while (it != temp.end() && (*it) != '\n') it++;
	it++;

	for (; it != temp.end(); it++) {

		while (it != temp.end() && (*it) != '\n')
		{
			cout << *it;
			buffer += *it;
			it++;
		}

		if (it == temp.end()) return rec_vec;

		string::iterator it_buf = buffer.begin();
		record rec_temp;
		string temp_s;
		while (*it_buf != ',')
		{
			temp_s += *it_buf;
			it_buf++;
		}
		rec_temp.time = timeConvert(temp_s);
		temp_s.clear();

		//get the price
		it_buf++;
		while (*it_buf != ',')
		{
			temp_s += *it_buf;
			it_buf++;
		}
		rec_temp.price = stod(temp_s);
		temp_s.clear();

		//get the size
		it_buf++;
		while (it_buf != buffer.end())
		{
			temp_s += *it_buf;
			it_buf++;
		}
		rec_temp.size = stoi(temp_s);

		rec_vec.push_back(rec_temp);
		buffer.clear();
	}
	return rec_vec;
}


int main(int argc, char **argv){

    int FILESIZE=stoi(argv[1]);


	int rank, size, bufsize, nints;
	MPI_File fh;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	bufsize = FILESIZE/size;
	nints = bufsize/sizeof(char);
	char buf[nints];
    
    cout<<"nints is "<<nints;
	
    MPI_File_open(MPI_COMM_WORLD,"/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/data10k.txt",MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
    MPI_File_read_at(fh, rank*bufsize, buf, nints, MPI_BYTE, &status);
	
    cout<<endl<<buf<<endl;
    
    vector<record> vec_rec=string2record(buf);
    
    
    
    //printf("\nrank: %d, buf[%d]: %d \n", rank, rank*bufsize, buf[0]);
	
    
    MPI_File_close(&fh);
	MPI_Finalize();

	return 0;
}
