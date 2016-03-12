# -*- coding: utf-8 -*-
"""
Created on Sat Mar 12 10:40:34 2016

@author: Yixiang Wang
"""

import datetime
from random import randint
from datetime import timedelta
import csv
from yahoo_finance import Share
#in order to use yahoo_finance, you need to install it first
#pip install yahoo-finance

st= datetime.datetime(2016, 3, 7, 9, 45, 0, 00001)#set the default starting time
print st + timedelta(microseconds=randint(0, 100000))

N_rec=100#number of record generated


for i in range(N_rec):
    st+=timedelta(microseconds=randint(0, 100000))
    print st
    
#reading the symbol data from csv file    
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

#print randint(0,29)
#print random.uniform(1.5, 1.9)
prices= range(30)
for i in prices:
    prices[i]=Share(symbol['Ticker'][i]).get_price()

print prices

with open ('data.txt', 'w') as fo:
   for i in range(N_rec):
       st+=timedelta(microseconds=randint(0, 100000))
       fo.write(st.strftime('%m/%d/%Y:%H:%M:%S.%f') +','+'good'+ '\n')
    





        
    