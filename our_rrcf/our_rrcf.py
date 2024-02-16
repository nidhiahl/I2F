#!/usr/bin/env python
# coding: utf-8


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


#print ("Running_Time", "AAR_deltaD", "AAR_D0")

input_path = '../io_directory/'
D0,D0_class_labels = load_data(input_path+'D0.csv')

nD0 = len(D0)

# Set forest parameters
num_trees = 100
tree_size = 256
sample_size_range = (nD0 // tree_size, tree_size)

# Construct RRCF over D0
forest = []
while len(forest) < num_trees:
    # Select random subsets of points uniformly
    ixs = np.random.choice(nD0, size=sample_size_range,
                           replace=False)
    # Add sampled trees to forest
    trees = [rrcf.RCTree(D0[ix], index_labels=ix)
             for ix in ixs]
    forest.extend(trees)

'''
#Compute anomaly score for D0 using lazy-RRCF
avg_codisp_D0 = {}
avg_codisp_D0=pd.Series(0.0, index=np.arange(nD0))
for index, point in enumerate(D0):
    # For each tree in the forest...
    for tree in forest: 
        new_codisp = tree.codisp(tree.query(point))
        # And take the average over all trees
        avg_codisp_D0[index] += new_codisp / num_trees

AAR_D0_lazy = AAR(pd.concat([avg_codisp_D0,pd.DataFrame(D0_class_labels)],axis=1,ignore_index=True))
'''


#Read deltaD for updates in RRCF
deltaD,deltaD_class_labels = load_data(input_path+'deltaD1.csv')
ndeltaD=len(deltaD)

start_update_time = time.time()
#Update RRCF and compute anomaly score for deltaD.
avg_codisp = {}
avg_codisp=pd.Series(0.0, index=np.arange(nD0,nD0+ndeltaD))
avg_codisp
for index, point in enumerate(deltaD):
    index += nD0
    for tree in forest:
        # If tree is above permitted size...
        #if len(tree.leaves) > tree_size:
            # Drop the oldest point (FIFO)
            #tree.forget_point(index+10000 - tree_size)
        # Insert the new point into the tree
        tree.insert_point(point, index=index)
        # Compute codisp on the new point...
        new_codisp = tree.codisp(index)
        # And take the average over all trees
        if not index in avg_codisp:
            avg_codisp[index] = 0
        avg_codisp[index] += new_codisp / num_trees

running_time_for_updates = time.time()-start_update_time

AAR_deltaD = AAR(pd.concat([avg_codisp,pd.DataFrame(deltaD_class_labels,index=range(nD0,nD0+ndeltaD))],axis=1,ignore_index=True))


#Compute anomlay score for D0 after updating RRCF using deltaD
#For experiments to measure catastrophic forgetting

avg_codisp_D0 = {}
avg_codisp_D0=pd.Series(0.0, index=np.arange(nD0))
for index, point in enumerate(D0):
    # For each tree in the forest...
    for tree in forest:
        new_codisp = tree.codisp(tree.query(point))
        # And take the average over all trees
        avg_codisp_D0[index] += new_codisp / num_trees

AAR_D0 = AAR(pd.concat([avg_codisp_D0,pd.DataFrame(D0_class_labels)],axis=1,ignore_index=True))


print (running_time_for_updates, AAR_deltaD, AAR_D0)
