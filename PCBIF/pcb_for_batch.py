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



dataset_name = str(sys.argv[1])
update_type = str(sys.argv[2])   #U1 or U2
Di_path = "../io_directory/Di.csv"
deltaD_path = "../io_directory/deltaD_"+update_type+".csv"

Di = load_data(Di_path)
deltaD = load_data(deltaD_path)
Di_size = len(Di)
deltaD_size = len(deltaD)

data = np.concatenate((Di,deltaD))
sample_size = 256
numiTrees = int(sys.argv[3])
num_iteration = int(sys.argv[4])

dict_key = "W" + str(sample_size) + "_T" + str(numiTrees)
print("extension_level="+str(int(pcb_param_list['extension_level'] * data.shape[1] - 2)))

#num_iteration = 10
for i in range(num_iteration):
    random_state = np.random.RandomState(i)
    initial_time = time.time()
    samples_used = 0
       
        
    ix = np.random.choice(Di_size, size=sample_size, replace=False)
    Di_sample = Di[ix,:-1]
        
        
    F = EiForest(Di_sample, ntrees=numiTrees, sample_size=sample_size,
                                         threshold=pcb_param_list['anomaly_threshold_eif'], ExtensionLevel=int(
                                         pcb_param_list['extension_level'] * data.shape[1] - 2))        
    F.ensemble_fit(Di_sample)  # EiForest model trained
    ndkswindow = ndk(alpha=pcb_param_list['alpha'], 
                         data=Di_sample,
                         n_dimensions=pcb_param_list['n_dimensions'],
                         window_size=sample_size,
                         stat_size=pcb_param_list['stat_size'],
                         n_tested_samples=pcb_param_list['n_tested_samples'],
                         fixed_checked_dimension=False, 
                         fixed_checked_sample=False)
                    
        
    PCBIF_time = time.time()
        
    ix = np.random.choice(deltaD_size, size=sample_size, replace=False)
    deltaD_sample = deltaD[ix,:-1]
       
    for data_instance in deltaD[:,:-1]:
        overall_anomaly_score, individual_anomaly_score = F.compute_paths(data_instance) # self.window[-1]
        F.compare_scores(overall_anomaly_score, individual_anomaly_score)
        #prediction = F.predict_function(overall_anomaly_score)
            
    ndkswindow.add_element(deltaD_sample)
    if ndkswindow.detected_change():
        F.update_model(deltaD_sample.tolist())
            
    PCBIF_time = time.time()- PCBIF_time


    Di_ascore = pd.DataFrame(columns=[0,1,2])
    for data_instance in Di:
        x_instance, y_instance = preprocessing_data(data_instance)
        overall_anomaly_score, individual_anomaly_score = F.compute_paths(x_instance) # self.window[-1]
        Di_ascore.loc[len(Di_ascore.index)] = [samples_used-1,overall_anomaly_score,y_instance]  



    deltaD_ascore = pd.DataFrame(columns=[0,1,2])
    for data_instance in deltaD:
        x_instance, y_instance = preprocessing_data(data_instance)
        overall_anomaly_score, individual_anomaly_score = F.compute_paths(x_instance) # self.window[-1]
        deltaD_ascore.loc[len(deltaD_ascore.index)] = [samples_used-1,overall_anomaly_score,y_instance]
             
    
    print(PCBIF_time, AAR(Di_ascore),AAR(deltaD_ascore))
        
