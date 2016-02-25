#include <stdio.h>
#include "mpi.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

struct time_struct {

	double min;
	double sec;

};

struct record {
    time_struct time;
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
            //cout << *it;
            buffer += *it;
            it++;
        }
        
        if (it == temp.end()) return rec_vec;
        
        string::iterator it_buf = buffer.begin();
        record rec_temp;
        string temp_s;
        try {
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
        }//for try
        catch (...) {//catch every thing
            cout << endl << "invalid data format!" << endl;
        }
    }
    return rec_vec;
}

void swap(vector<record>& src, int p1, int p2) {
    record temp;
    temp = src[p1];
    src[p1] = src[p2];
    src[p2] = temp;
}

bool islater(record r1, record r2) {
    //return true if r1 is later record than r2
    //which means the time value of r1 is greater than r2
    if (r1.time.min != r2.time.min) {
        return (r1.time.min > r2.time.min) ? true : false; }
    else return (r1.time.sec > r2.time.sec) ? true : false;
}

void initial_window(vector<record>& src, int start, int end) {
    //this is a small sort so the advantage of quick sort over bubble sort is not obvious
    //so here I just implement a simple bubble sort
    
    int n = end - start;
    for (int i = 0; i < n; i++) {
        for (int j = start + 1; j <= end - i; j++) {
            if (islater(src[j - 1], src[j])) swap(src, j - 1, j);
        }
    }
    
}

bool mean_variance_valid(double x, double mean, double st, double tol) {
    return abs(x - mean) < tol*st;
}

pair<vector<record>, vector<record> > filter(vector<record> & src,int window_size,double tol=3) {
    
    pair<vector<record>, vector<record> > res;
    
    //window_size = 20;
    //if (20 > src.size() / 10) window_size = src.size() / 10;
    initial_window(src, 0, window_size - 1);
    vector<record>::iterator it_src = src.begin();
    
    vector<record> signal;
    vector<record>::iterator it_sig = signal.begin();
    vector<record> noise;
    vector<record>::iterator it_nos = noise.begin();
    double sum = 0;
    double squared_sum = 0;
    double running_mean = 0;
    for (int count = 0; count < window_size; count++) {
        sum += (*it_src).price;
        it_src++;
    }
    double number = window_size;
    running_mean = sum / number;
    
    for (int count = 0; count < window_size; count++) {
        squared_sum += pow(src[count].price-running_mean,2);
    }
    
    for (; it_src != src.end(); it_src++) {
        if (islater(*(it_src - window_size), *it_src)) {
            it_src = src.erase(it_src);
            it_src--;
        }
        else if (islater(*(it_src - 1), *it_src)) {
            sum += (*it_src).price;
            number += 1;
            running_mean = sum / number;//update the running mean
            squared_sum += pow((*it_src).price - running_mean, 2);
            vector<record>::iterator it_temp = it_src;
            while (islater(*(it_temp - 1), *it_temp)) {
                record temp_record = *it_temp;
                *it_temp = *(it_temp - 1);
                *(it_temp - 1) = temp_record;
                it_temp--;
            }
        }
        else {
            sum += (*it_src).price;
            number += 1;
            running_mean = sum / number;//update the running mean
            squared_sum += pow((*it_src).price - running_mean, 2);
        }
    }
    
    double mean = sum / number;
    double sd = sqrt(squared_sum / (number - 1.0));
    
    for (it_src = src.begin(); it_src != src.end(); it_src++) {
        if (mean_variance_valid((*it_src).price, mean, sd, tol)) signal.push_back(*it_src);
        else noise.push_back(*it_src);
    }
    
    res = make_pair(signal, noise);
    return res;
}

int main(int argc, char **argv){

    MPI_Offset FILESIZE;//=stoi(argv[1]);
    
	int rank, size, bufsize, nints;
	MPI_File fh;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_File_open(MPI_COMM_WORLD,"/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/data10k.txt",MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
    
    MPI_File_get_size(fh, &FILESIZE);
    
	bufsize = FILESIZE/size;
	nints = bufsize/sizeof(char);
	char buf[nints];
	
    
    MPI_File_read_at(fh, rank*bufsize, buf, nints, MPI_BYTE, &status);
    
    vector<record> vec_rec=string2record(buf);
    cout<<"cluster: "<<rank<<" the size of vector is"<<vec_rec.size()<<endl;
    
    //printf("\nrank: %d, buf[%d]: %d \n", rank, rank*bufsize, buf[0]);
	
    
    MPI_File_close(&fh);
	MPI_Finalize();

	return 0;
}
