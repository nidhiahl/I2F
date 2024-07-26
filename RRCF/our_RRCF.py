#!/usr/bin/env python
# coding: utf-8
import os
import sys
import time

import pandas as pd
import numpy as np
import rrcf
import time
from datetime import datetime

def load_data(dataFile: str):
  df = pd.read_csv(dataFile,sep=' ')
  df = df.astype(float)
  class_labels = df.iloc[:, -1:]
  class_labels = class_labels.to_numpy()
  df = df.iloc[: , :-1]
  df = df.to_numpy()
  return df, class_labels


def load_data_without_labels(dataFile: str):
  df = pd.read_csv(dataFile,sep=' ')
  df = df.astype(float)
  df = df.to_numpy()
  return df


def AAR(df: pd.DataFrame):
    
    #df.drop(0,axis=1,inplace=True)
    df.sort_values(by=0,ascending=False,inplace=True)
    df=df.reset_index(drop=True)
    AAR = np.mean(df.index[df[1] == 1].tolist())
    return AAR


#print ("Running_Time", "AAR_deltaD", "AAR_Di")

datasets= str(sys.argv[1])
update_type = str(sys.argv[2])

input_path = '../io_directory/'
Di,Di_class_labels = load_data(input_path+'Di.csv')
Di_class_labels = pd.read_csv(input_path+'Di_'+update_type+'_labels.csv',sep=' ',header=None)
nDi = len(Di)

# Set forest parameters
num_trees = 100
tree_size = 256
sample_size_range = (nDi // tree_size, tree_size)

# Construct RRCF over Di
forest = []
while len(forest) < num_trees:
    # Select random subsets of points uniformly
    ixs = np.random.choice(nDi, size=sample_size_range,
                           replace=False)
    # Add sampled trees to forest
    trees = [rrcf.RCTree(Di[ix], index_labels=ix)
             for ix in ixs]
    forest.extend(trees)
    #print(len(forest),'trees created')
    
    
    
'''
#Compute anomaly score for Di using lazy-RRCF

avg_codisp_Di = pd.Series(0.0, index=np.arange(nDi))
index = np.zeros(nDi)
# Compute average CoDisp
for tree in forest:
    codisp = pd.Series({leaf : tree.codisp(leaf)
                       for leaf in tree.leaves})
    
    avg_codisp_Di[codisp.index] += codisp
    
    np.add.at(index, codisp.index.values, 1)
avg_codisp_Di /= index

AAR_Di_lazy = AAR(pd.concat([avg_codisp_Di,pd.DataFrame(Di_class_labels)],axis=1,ignore_index=True))

print ( AAR_Di_lazy)
'''



#Read deltaD for updates in RRCF
deltaD,deltaD_class_labels = load_data(input_path+'deltaD_'+update_type+'.csv')
deltaD_class_labels=pd.read_csv(input_path+'deltaD_'+update_type+'_labels.csv',sep=' ',header=None)
#print(deltaD_class_labels)
ndeltaD=len(deltaD)
#print(pd.DataFrame(deltaD_class_labels,index=range(ndeltaD)))

start_update_time = time.time()
#Update RRCF and compute anomaly score for deltaD.
avg_codisp_deltaD = {}
avg_codisp_deltaD=pd.Series(0.0, index=np.arange(ndeltaD))
avg_codisp_deltaD
for index, point in enumerate(deltaD):
    index += nDi
    for tree in forest:
        
        tree.insert_point(point, index=index)
        
        new_codisp = tree.codisp(index)
        
        if not index-nDi in avg_codisp_deltaD:
            avg_codisp_deltaD[index] = 0
            
        avg_codisp_deltaD[index-nDi] += new_codisp / num_trees

running_time_for_updates = time.time()-start_update_time
print(running_time_for_updates,end=' ')


avg_codisp_deltaD = ((avg_codisp_deltaD - avg_codisp_deltaD.min())/ (avg_codisp_deltaD.max() - avg_codisp_deltaD.min()))

avg_codisp_deltaD.to_csv('../io_directory/RRCF_AS_deltaD.csv',sep=' ',index=False, header=None)
ddff=pd.concat([avg_codisp_deltaD,pd.DataFrame(deltaD_class_labels)],axis=1,ignore_index=True)
ddff.to_csv('../io_directory/RRCF_AAR_deltaD.csv',sep=' ')

#AAR_deltaD = AAR(pd.concat([avg_codisp_deltaD,pd.DataFrame(deltaD_class_labels)],axis=1,ignore_index=True))
#print ( AAR_deltaD,end=' ')
#ndeltaD
avg_codisp_deltaD = pd.concat([avg_codisp_deltaD,pd.DataFrame(deltaD_class_labels)],axis=1,ignore_index=True)



#Compute anomlay score for Di after updating RRCF using deltaD
#For experiments to measure catastrophic forgetting

avg_codisp_Di = {}
avg_codisp_Di = pd.Series(0.0, index=np.arange(nDi+ndeltaD))
index = np.zeros(nDi+ndeltaD)
# Compute average CoDisp
for tree in forest:
    codisp = pd.Series({leaf : tree.codisp(leaf)
                       for leaf in tree.leaves})
    
    avg_codisp_Di[codisp.index] += codisp
    
    np.add.at(index, codisp.index.values, 1)
avg_codisp_Di /= index




'''
avg_codisp_Di = {}
avg_codisp_Di=pd.Series(0.0, index=np.arange(nDi))
for index, point in enumerate(Di):
    # For each tree in the forest...
    for tree in forest:
        new_codisp = tree.codisp(tree.query(point))
        # And take the average over all trees
        avg_codisp_Di[index] += new_codisp / num_trees
'''

avg_codisp_Di = avg_codisp_Di[:-ndeltaD]
avg_codisp_Di = ((avg_codisp_Di - avg_codisp_Di.min())/ (avg_codisp_Di.max() - avg_codisp_Di.min()))

avg_codisp_Di.to_csv('../io_directory/RRCF_AS_Di.csv',sep=' ',index=False, header=None)


#avg_codisp_Di = pd.concat([avg_codisp_Di,pd.DataFrame(Di_class_labels)],axis=1,ignore_index=True)

#dd=pd.concat([avg_codisp_Di,pd.DataFrame(Di_class_labels)],axis=1,ignore_index=True)
#dd.to_csv('../io_directory/RRCF_AAR_Di.csv',sep=' ')

#AAR_Di = AAR(pd.concat([avg_codisp_Di,pd.DataFrame(Di_class_labels)],axis=1,ignore_index=True))


#print ( AAR_Di,end=' ')











