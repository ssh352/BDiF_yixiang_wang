# -*- coding: utf-8 -*-
"""
Created on Sat Mar 19 17:04:27 2016

@author: wyx
"""
import re

string_test='test:.[]da..gS***NOT////not..StuPid@@@@'

stck_list={"JPM":1, "MS":1,"GS":1}
word_list={"GOOD":1,"YES":1,"BAD":-1,"STUPID":-1,"NOT":-1}



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

