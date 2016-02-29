#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <time.h>

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
    time_struct temp_t;
    temp_t.min=0;
    temp_t.sec=0;
    
    try {
        for (it = source.begin(); (*it) != '.'; it++) {
            if ((*it) != ':') temp += *it;
        }
        double d1 = atof(temp.c_str());
        temp.clear();
        for (; it != source.end(); it++) temp += *it;
        double d2 = atof(temp.c_str());
        
        
        temp_t.min = d1;
        temp_t.sec = d2;
    } catch(...) {
        cout<<"incorrect data format!"<<endl;
    }
    
	return temp_t;
}

vector<record> string2record(char a[]) {
    string temp = a;
    string buffer;
    vector<record> rec_vec;
    string::iterator it = temp.begin();
    while (it != temp.end() && (*it) != '\n') it++;
    it++;
    
    for (; it != temp.end(); it++) {
        
        while (it != temp.end() && (*it) != '\n')
        {
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
            rec_temp.price = atof(temp_s.c_str());
            temp_s.clear();
            
            //get the size
            it_buf++;
            while (it_buf != buffer.end())
            {
                temp_s += *it_buf;
                it_buf++;
            }
            rec_temp.size = atoi(temp_s.c_str());
            
            rec_vec.push_back(rec_temp);
            buffer.clear();
        }//for try
        catch (...) {//catch every thing
            cout << endl << "invalid data format!" << endl;
            //LOG(INFO) << "Invalid data format when converting from string to record.";
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

void moments(vector<record> &price, vector<long double> & res) {
    //stor the elements of JB test in the res vector
    //res[0] number of samples
    //res[1] centralized first moment
    //res[2] centralized second moment
    //res[3] centralized third moment
    //res[4] centralized fourth moment
    
    vector<double> retr(price.size() - 1, 0);
    vector<record>::iterator it_px = price.begin();
    int count = 0;
    long double mean = 0;
    for (; (it_px + 1) != price.end(); it_px++) {
        retr[count] = log((*(it_px + 1)).price / (*it_px).price);
        mean += retr[count];
        count++;
    }
    mean = mean / (retr.size() + 0.0);
    
    //calculate the standard deviation
    long double sd = 0;
    //calculate Skewness and Kurtosis
    long double k = 0;//kurtosis
    long double s = 0;//skewness
    for (vector<double>::iterator it_rt = retr.begin(); it_rt != retr.end(); it_rt++) {
        sd += pow((*it_rt) - mean, 2);
        s += pow((*it_rt) - mean, 3);
        k += pow((*it_rt) - mean, 4);
    }
    
    sd = sqrt(sd / (retr.size() - 1.0));
    s = (s / pow(sd, 3)) / (retr.size() + 0.0);
    k = (k / pow(sd, 3)) / (retr.size() + 0.0);
    
    res[0] = retr.size();
    res[1] = mean;
    res[2] = pow(sd, 2);
    res[3] = s;
    res[4] = k;
}

bool JBtest(vector<long double> moment_vec) {
    //moment_vec is calculated from the moments function
    //here I assume large sample space and use Chi square 2 approximation
    //the rejection region is the JB statistic greater than 5.991465 (calculated with r)
    //return false if null is rejected
    return moment_vec[0] * (pow(moment_vec[3], 2) / 6.0 + pow(moment_vec[4] - 3, 2) / 24.0) < 5.991465;
}

string record_vec2string(vector<record> src);

string record2string(record src) {
    stringstream stream_out;
    long long date = (long long)(src.time.min) / 1000000;
    long long temp = (long long)(src.time.min) % 1000000;
    long long hour = temp / 10000;
    temp = temp % 10000;
    long long min = temp / 100;
    long long sec = temp % 100;
    stream_out << date << ':' << hour << ':' << min << ':' << sec << '.' << int(src.time.sec * 1000000);
    stream_out << ',' << src.price << ',' << src.size;
    return stream_out.str();
}

string record_vec2string(vector<record> src) {
    string res;
    for (vector<record>::iterator it_src = src.begin(); it_src != src.end(); it_src++) {
        res += record2string(*it_src);
        res += '\n';
    }
    return res;
}

int main(int argc, char **argv){

    MPI_Offset FILESIZE;
    
	int rank, size, bufsize, nints;
	MPI_File fh;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_File_open(MPI_COMM_WORLD,argv[1],MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
    MPI_File_get_size(fh, &FILESIZE);
    
	bufsize = FILESIZE/size;
	nints = bufsize/sizeof(char);
	char buf[nints];
	
    MPI_File_read_at(fh, rank*bufsize, buf, nints, MPI_BYTE, &status);
    
    vector<record> vec_rec=string2record(buf);
    MPI_File_close(&fh);
    
    
    vector<long double> moment(5, 0);
    
    moments(vec_rec,moment);
    
    //gather all the information about the normality test
    long double N = moment[0];//number of samples
    long double S = moment[3];//skewness
    long double K = moment[4];//kurtosis
    long double * norm_buf = (long double *)malloc(size*3*sizeof(long double));//define the receive buffer
    
    long double norm_send[3];
    norm_send[0]=N;
    norm_send[1]=S;
    norm_send[2]=K;
    
    MPI_Allgather(norm_send,3,MPI_LONG_DOUBLE,norm_buf,3,MPI_LONG_DOUBLE,MPI_COMM_WORLD);
    
    //only use one node to do the normality test
    
    if (rank==0){
    
        long double num_samp=0;
        long double cumulative_S=0;//the Skewness calculated from the total sample
        long double cumulative_K=0;//the Skewness calculated from the total sample
    
        for (int i=0; i<size; i++) {
            int n_i=norm_buf[3*i];
            cumulative_S+=n_i*norm_buf[3*i+1];
            cumulative_K+=n_i*norm_buf[3*i+2];
            num_samp+=n_i;
        }
        
        cumulative_S=cumulative_S/num_samp;
        cumulative_K=cumulative_K/num_samp;
        
        vector<long double> temp(5,0);
        temp[0]=num_samp;temp[3]=cumulative_S;temp[4]=cumulative_K;
        stringstream norm_res;
        norm_res<<"Normality test: "<<endl;
        norm_res<<"Number of samples is: "<<num_samp<<endl;
        norm_res<<"Skewness is: "<<cumulative_S<<endl;
        norm_res<<"Kurtosis is: "<<cumulative_K<<endl;
        if (JBtest(temp)) {
            norm_res<<"Null hypothesis is accepted: normality test passed"<<endl;
        }
        else {
            norm_res<<"Null hypothesis is rejected: normality test failed"<<endl;
        }
        //cout<<norm_res.str();
        
        ofstream norm_out;
        norm_out.open("exploration_result.txt");
        norm_out<<norm_res.str();
        norm_out.close();
    }
    
    
    MPI_Finalize();
	return 0;
}
