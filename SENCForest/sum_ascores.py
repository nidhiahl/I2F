import pandas as pd
import numpy as np
import sys

input_file = str(sys.argv[1])
output_file = str(sys.argv[2])
#input_file = "lazy_AS_deltaD1.csv"

df = pd.read_csv("../io_directory/"+input_file+".csv",sep=" ",header=None)
df.drop(0,axis=1,inplace=True)

#sumdf = pd.DataFrame()

try:
    sumdf = pd.read_csv("../io_directory/"+output_file+".csv",sep=" ",header=None)
except Exception:
    df[1].to_csv("../io_directory/"+output_file+".csv",sep=' ', header=None,index=False)
    exit(0)

sumdf[0]+=df[1]

sumdf.to_csv("../io_directory/"+output_file+".csv",sep=' ', header=None,index=False)


