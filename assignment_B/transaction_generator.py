# -*- coding: utf-8 -*-
"""
Created on Sat Mar 12 10:40:34 2016

@author: Yixiang Wang
"""

import datetime
from random import randint
from datetime import timedelta
import csv

st= datetime.datetime(2016, 3, 7, 9, 45, 0, 00001)#set the default starting time
print st + timedelta(microseconds=randint(0, 100000))

N_rec=100#number of record generated


for i in range(N_rec):
    st+=timedelta(microseconds=randint(0, 100000))
    print st
    
with open ('data.txt', 'w') as fo:
   for i in range(N_rec):
       st+=timedelta(microseconds=randint(0, 100000))
       fo.write(st.strftime('%m/%d/%Y/%H') +','+'good'+ '\n')
    
    



        
    