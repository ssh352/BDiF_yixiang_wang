#include <stdio.h>
#include "mpi.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>
#include <cmath>
#include "glog/logging.h"
#include <sstream>
#include <fstream>

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
    LOG(INFO) << "convert to a vector of records";
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
            LOG(INFO) << "Invalid data format when converting from string to record.";
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
    //this is a small sort
    //so the advantage of quick sort over bubble sort is not obvious
    //so here I just implement a simple bubble sort
    LOG(INFO) << "sliding window is initialized.";
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

pair<vector<record>, vector<record> > filter(vector<record> & src,int window_size, vector<long double> &mom ,double tol=3) {
    
    if (window_size>src.size())window_size=src.size()-5;//to handle the case that the input data set is too small
    
    LOG(INFO) << "The records number processed in this node is "<<src.size();
    pair<vector<record>, vector<record> > res;
    LOG(INFO) << "initialize filter to scrub data";

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
            noise.push_back(*it_src);
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
        if (mean_variance_valid((*it_src).price, mean, sd, tol)&&(*it_src).size>0) signal.push_back(*it_src);
        else noise.push_back(*it_src);
    }
    
    LOG(INFO) << "finished scrubbing";
    LOG(INFO)<< "noise size of this node is"<<noise.size();
    LOG(INFO)<< "signal size of this node is"<<signal.size();
    
    res = make_pair(signal, noise);
    moments(signal, mom);
    return res;
}

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
    
    google::InitGoogleLogging(argv[0]);

    MPI_Offset FILESIZE;//=stoi(argv[1]);
    
	int rank, size, bufsize, nints;
	MPI_File fh;
	MPI_Status status;
	MPI_Init(&argc, &argv);
    LOG(INFO) << "MPI Initialized.";
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_File_open(MPI_COMM_WORLD,"/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/data10k.txt",MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
    
    MPI_File_get_size(fh, &FILESIZE);
    
	bufsize = FILESIZE/size;
	nints = bufsize/sizeof(char);
	char buf[nints];
	
    
    MPI_File_read_at(fh, rank*bufsize, buf, nints, MPI_BYTE, &status);
    
    vector<record> vec_rec=string2record(buf);
    MPI_File_close(&fh);
    
    vector<long double> moment(5, 0);
    pair<vector<record>, vector<record> > result = filter(vec_rec, 500,moment,2);
    
    vector<record> signal = result.first;
    vector<record> noise = result.second;
    
    //write the signal to output file
    string signal_string=record_vec2string(signal);
    MPI_Offset offset_out=signal_string.size();//the offset of the local node
    MPI_Offset * send_offset = new long long;
    *send_offset=offset_out;
    long long * rbuf = (long long *)malloc(size*sizeof(long long));//define the receive buffer
    MPI_Allgather( send_offset, 1, MPI_LONG, rbuf, 1, MPI_LONG, MPI_COMM_WORLD);
    MPI_Offset cumulative_offset=0;
    for (int i=0;i<=rank;i++){
        cumulative_offset+=rbuf[i];
    }
    MPI_File fh_out;
    MPI_File_open(MPI_COMM_WORLD, "signal.txt", MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh_out);
    MPI_File_write_at(fh_out, cumulative_offset, signal_string.c_str(), offset_out, MPI_BYTE, &status);
	MPI_File_close(&fh_out);
    
    //write the noise to output file
    string noise_string=record_vec2string(noise);
    offset_out=noise_string.size();//the offset of the local node
    delete send_offset;
    send_offset = new long long;
    *send_offset=offset_out;
    free(rbuf);
    rbuf = (long long *)malloc(size*sizeof(long long));//define the receive buffer
    MPI_Allgather( send_offset, 1, MPI_LONG, rbuf, 1, MPI_LONG, MPI_COMM_WORLD);
    cumulative_offset=0;
    for (int i=0;i<=rank;i++){
        cumulative_offset+=rbuf[i];
    }
    //"/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/noise.txt"
    MPI_File_open(MPI_COMM_WORLD, "noise.txt", MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh_out);
    MPI_File_write_at(fh_out, cumulative_offset, signal_string.c_str(), offset_out, MPI_BYTE, &status);
    MPI_File_close(&fh_out);
    
    
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
        norm_res<<"Kurtosis is : "<<cumulative_K<<endl;
        norm_res<<"Skewness is : "<<cumulative_S<<endl;
        if (JBtest(temp)) {
            norm_res<<"Null hypothesis is accepted: normality test passed"<<endl;
        }
        else {
            norm_res<<"Null hypothesis is rejected: normality test failed"<<endl;
        }
        cout<<norm_res.str();
        
        ofstream norm_out;
        norm_out.open("/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/normality_test_result.txt");
        norm_out<<norm_res.str();
        norm_out.close();
    }
    
    MPI_Finalize();
	return 0;
}
