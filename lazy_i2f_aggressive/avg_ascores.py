import pandas as pd
import numpy as np
import sys

input_file = str(sys.argv[1])
output_file  =str(sys.argv[2])
num_runs = int(sys.argv[3])

df = pd.read_csv("../io_directory/"+input_file+".csv",sep=" ",header=None)

#sumdf = pd.DataFrame()


df[0]/=num_runs

df.to_csv(output_file,sep=' ', header=None,index=False)


