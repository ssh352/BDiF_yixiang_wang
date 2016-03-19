# -*- coding: utf-8 -*-
"""
Created on Sat Mar 19 17:04:27 2016

@author: wyx
"""
import re
import numpy as np
import csv

f = open('/Users/wyx/Documents/Baruch MFE/BDiF_yixiang_wang/assignment_C/LoughranMcDonald_MasterDictionary_2014.csv', 'rb')
reader = csv.reader(f)
headers = reader.next()

wordlist = {}
for h in headers:
    wordlist[h] = []
    
for row in reader:
    for h, v in zip(headers, row):
        wordlist[h].append(v)

f.close()

print headers

word_list={}

for i in range(len(wordlist['Positive'])):
    if wordlist['Positive'][i]=='2009':
        temp=1
    elif wordlist['Negative'][i]=='2009':
        temp=-1
    else:
        temp=0
    word_list[wordlist['Word'][i]]=temp
    
word_list['GOOD']
    



string_test='test:.[]da..gS***NOT////not..StuPid@@@@'

stck_list={"JPM":1, "MS":1,"GS":1}




def parse_tweet(src):
    src_list=re.findall(r"[\w']+", src)     
    res=[False,"NA",1]    
    for i in src_list:
        if i.upper() in stck_list.keys():
            res[0]=True            
            res[1]=i.upper()
        if i.upper() in word_list.keys() and res[0]:
            res[2]=res[2]*word_list[i.upper()]
    return res
    
print parse_tweet(string_test)

