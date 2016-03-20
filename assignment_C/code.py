# -*- coding: utf-8 -*-
"""
Created on Sat Mar 19 17:04:27 2016

@author: wyx
"""
import re
import numpy as np
import csv
from yahoo_finance import Share

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

string_test='test:.[]da..gS***NOT/goLdman//bAd/not..StuPid@@@@'

stck_list={"google":"GOOG", 
"facebook":"FB",
"3M":"MMM" ,
"americanexpress":"AXP", 
"apple":"AAPL", 
"boeing":"BA" ,
"caterpillar":"CAT", 
"chevron":"CVX", 
"cisco":"CSCO", 
"coca":"KO", 
"disney":"DIS", 
"ge":"GE",
"goldman":"GS",
"homedepot":"HD",
"ibm":"IBM",
"intel":"INTC",
"johnsonjohnson":"JNJ", 
"jpmorgan":"JPM",
"mcdonald":"MCD", 
"merck":"MRK", 
"microsoft":"MSFT", 	
"nike":"NKE", 	
"pfizer":"PFE",	
"p&g":"PG", 
"verizon":"VZ", 
"visa":"Verizon",		
"walmart":"WMT"} 


def parse_tweet(src):
    """
    analyze one tweet
    output a list contains three information:
    if the tweet contains stocks information
    what is the stock symbol
    positive or negative
    """
    src_list=re.findall(r"[\w']+", src)     
    res=[False,"NA",0]    
    for i in src_list:
        if i.lower() in stck_list.keys():
            res[0]=True            
            res[1]=stck_list[i.lower()]
        if i.upper() in word_list.keys():
            res[2]=res[2]+word_list[i.upper()]
    return res
    
print parse_tweet(string_test)
    
f = open("/Users/wyx/Documents/Baruch MFE/BDiF2016/tweets_sample.txt"  , 'rb')
reader = csv.reader(f)
tweet_atrr = ['f1','f2','f3','f4','f5','f6','f7','f8','f9']
tweets={}
for h in tweet_atrr:
    tweets[h] = []
    
for row in reader:
    for h, v in zip(tweet_atrr, row):
        tweets[h].append(v)
f.close()


def tweet_price_predictor(stock_list,tweets_rec):
        """
        input arguments:
        stock_list:the dictionary of stocks name to symbol
        tweets_rec:the tweets records during the time horizon we want to predict            
        return:
        a dictionary between the stock symbols and the corresponding price prediction
        
        """
        res={}
        for i in stock_list.values():
            res[i]=[0,0,0]#total;positive;negative
        for twt in tweets_rec:
            temp=parse_tweet(twt)
            if temp[0]:
                res[temp[1]][0]+=1
                if temp[2]>0:
                    res[temp[1]][1]+=1
                elif temp[2]<0:
                    res[temp[1]][2]+=1

        inv_map = {v: k for k, v in stock_list.items()}
        for i in res.keys():
            if res[i][1]>res[i][2]:
                print inv_map[i].upper()+' price will go up: '+ str(Share(i).get_price())
            elif res[i][1]<res[i][2]:
                print inv_map[i].upper()+' price will go down: '+ str(Share(i).get_price())
            else:
                print inv_map[i].upper()+' price will stay: '+ str(Share(i).get_price())
            
                
#test

tweet_price_predictor(stck_list,tweets['f8'][1:100])





