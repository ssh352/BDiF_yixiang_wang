# -*- coding: utf-8 -*-
"""
@author: Yixiang Wang
"""
import sys
import datetime
from random import randint
from datetime import timedelta
import csv
#in order to use yahoo_finance, you need to install it first
#pip install yahoo-finance
from yahoo_finance import Share

st= datetime.datetime(2016, 3, 7, 9, 45, 0, 00001)#set the default starting time

N_rec=int(sys.argv[1])#number of record generated

#reading the symbol data from csv file
#in this assignment, I am using only stocks in the Dow Jones 30
f = open('DJ_data.csv', 'rb')
reader = csv.reader(f)
headers_dj = reader.next()

symbol = {}
for h in headers_dj:
    symbol[h] = []
    
for row in reader:
    for h, v in zip(headers_dj, row):
        symbol[h].append(v)

f.close()

#get the initial price from yahoo finance
prices= range(30)
for i in prices:
    prices[i]=float(Share(symbol['Ticker'][i]).get_price())

#generate the price and size
#for simplicity, the price will stay the same, or go up by 2.5% or go down by 2.5% with same probability
#the size is uniform distributed from 1 to 1000
with open ('data.csv', 'w') as fo:
   for i in range(N_rec):
       tic=randint(0,29)
       move=randint(-1,1)#the price movement direction
       size=randint(1,1000)
       prices[tic]=prices[tic]*(1+move*0.025)
       st+=timedelta(microseconds=randint(0, 100000))
       fo.write(st.strftime('%m/%d/%Y:%H:%M:%S.%f') +','+symbol['Ticker'][tic]+ ','+str(prices[tic])+','+str(size)+'\n')#writing the generated data to a csv file.
    





        
    