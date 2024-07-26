import pandas as pd
import numpy as np
import sys

ascore_file = str(sys.argv[1])
label_file = str(sys.argv[2])

#input_file = "lazy_AS_deltaD1.csv"

score_df = pd.read_csv("../io_directory/"+ascore_file+".csv",sep=" ",header=None)
label_df = pd.read_csv("../io_directory/"+label_file+".csv",sep=" ",header=None)

df = pd.concat([score_df[1],label_df[0]], axis=1)

df.sort_values(by=1,ascending=False,inplace=True)

df=df.reset_index(drop=True)
AAR = np.mean(df.index[df[0] == 1].tolist())
print(AAR,end=' ')
