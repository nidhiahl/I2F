import os
import sys
import time

import numpy as np
import pandas as pd

from datetime import datetime
import json
from statistics import mean

from PCB_iForest_modules.PCB_EIF import *

INF_value = 1519889160.0* 100000

pcb_param_list = {
# PCB-iForest
'window_size' : 200,
'num_trees' : 100,
'extension_level' : 1.0, # in percent of dimension - 1.0 denotes to fully extended version
'anomaly_threshold_eif' : 0.5,
# NDKSWIN-Parameters
'alpha' : 0.01,
'n_dimensions' : 1,
'n_tested_samples' : 0.1,
'stat_size' : 30
}

def load_data(dataFile: str):
    '''
    return data
    @param dataFile: path to the file
    @return: data
    '''
    df = pd.read_csv(dataFile,sep=' ')
    class_labels = df.iloc[:, -1:]
    final_df = df.astype(float)
    data_with_labels = final_df.to_numpy()
    return data_with_labels

def preprocessing_data(data: np.ndarray):
    '''
    handling remaining nan, inf values in data instance; separating the class labels from data instance
    @param data: numpy array
    @return: data, class label
    '''
    data = data.tolist()
    y_label = data[-1]
    if isinstance(y_label, str):
        if y_label !=  "Benign": #anomalous point is represented as 1
            y_label = 1

        else:
            y_label = 0

    x_data = np.asarray(data[:-1])
    x_data = np.nan_to_num(x_data, nan=0.0, posinf= INF_value,
                         neginf=-INF_value)

    return x_data, y_label

def AAR(df: pd.DataFrame):
    df.drop(0,axis=1,inplace=True)
    df.sort_values(by=1,ascending=False,inplace=True)
    df=df.reset_index(drop=True)
    AAR = np.mean(df.index[df[2] == 1].tolist())
    return AAR



dataset_name = "shuttle"
update_type = "U1"   #U1 or U2
D0_path = "./datasets/"+dataset_name+"/"+update_type+"/D0.csv"
deltaD_path = "./datasets/"+dataset_name+"/"+update_type+"/deltaD.csv"

D0 = load_data(D0_path)
deltaD = load_data(deltaD_path)
D0_size = len(D0)
deltaD_size = len(deltaD)


data = np.concatenate((D0,deltaD))

hyper_parameters = [[128,100]]

for params in hyper_parameters:
    dict_key = "W" + str(params[0]) + "_T" + str(params[1])
    print("classifier PCBIF processing "+ dataset_name + " with hyper_parameters " + dict_key+" extension="+str(int(
                                                         pcb_param_list['extension_level'] * data.shape[1] - 2)))

    num_iteration = 10
    for i in range(num_iteration):
        random_state = np.random.RandomState(i)
        initial_time = time.time()
        samples_used = 0
        PCB_iForest = PCB_EIF_Window(dimension=data.shape[1], window_size=params[0],
                                                     num_trees=params[1],
                                                     anomaly_threshold=pcb_param_list['anomaly_threshold_eif'],
                                                     extension_level=int(
                                                         pcb_param_list['extension_level'] * data.shape[1] - 2),  # -2 here since label column is still contained in data
                                                     initial_data_size = D0_size, update_data_size = deltaD_size,
                                                     alpha=pcb_param_list['alpha'],
                                                     n_dimensions=pcb_param_list['n_dimensions'],
                                                     n_tested_samples=pcb_param_list['n_tested_samples'],
                                                     stat_size=pcb_param_list['stat_size'])
        
        for data_instance in data:
            x_instance, y_instance = preprocessing_data(data_instance)
            PCB_iForest.run_train_classifier(x_instance)


        D0_ascore = pd.DataFrame(columns=[0,1,2])
        for data_instance in D0:
            samples_used += 1
            x_instance, y_instance = preprocessing_data(data_instance)
            predicted_value = PCB_iForest.compute_ascore(x_instance)
            D0_ascore.loc[len(D0_ascore.index)] = [samples_used-1,predicted_value,y_instance]  
            
            
        deltaD_ascore = pd.DataFrame(columns=[0,1,2])
        for data_instance in deltaD:
            samples_used += 1
            x_instance, y_instance = preprocessing_data(data_instance)
            predicted_value = PCB_iForest.give_ascore(samples_used-1)
            deltaD_ascore.loc[len(deltaD_ascore.index)] = [samples_used-1,predicted_value,y_instance]
        print(AAR(D0_ascore),AAR(deltaD_ascore),PCB_iForest.update_time)
        
            
