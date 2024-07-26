import os
import sys
import time
import pandas as pd
import numpy as np
from sklearn import metrics
import json
from statistics import mean
from PCB_iForest_modules.PCB_EIF import *


INF_value = 1519889160.0* 100000

pcb_param_list = {
# PCB-iForest
'extension_level' : 1.0, # in percent of dimension - 1.0 denotes to fully extended version
'anomaly_threshold_eif' : 0.6,
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


def AAR_F1_AUC(df: pd.DataFrame):
    #df.drop(0,axis=1,inplace=True)
    df.sort_values(by=0,ascending=False,inplace=True)
    df=df.reset_index(drop=True)
    AAR = np.mean(df.index[df[1] == 1].tolist())
    
    num_anomalies=df[1].value_counts()[1]
    
    pred_labels = np.zeros(len(df))
    pred_labels[:num_anomalies]=1
    df['pred_labels']=pred_labels
    
    
    f_score=metrics.f1_score(df[1],df['pred_labels'])
    
    fpr, tpr, thresholds = metrics.roc_curve(df[1], df[0], pos_label=1)
    
    auc_score = metrics.auc(fpr, tpr)
    
    print(AAR,f_score,auc_score,end=' ')


def recall_AUC(scores_labels:pd.DataFrame):
    #sort the anomlayscores in descending order
    scores_labels.sort_values(by=0,ascending=False,inplace=True)
    total_anomalies=scores_labels[1].value_counts()[1]
    #add predicted labels column in dataframe------
    pred_labels = np.zeros(len(scores_labels))
    pred_labels[:total_anomalies]=1
    scores_labels['pred_labels']=pred_labels
    for i in [total_anomalies, int(len(scores_labels)*0.1), int(len(scores_labels)*0.2), int(len(scores_labels)*0.3),]:
        head_rows=scores_labels.head(i)
        #head_rows=scores_labels.head(int(len(scores_labels)/2))
        tp_Di=len(head_rows[(head_rows[1] == 1) & (head_rows.index < Di_size)])
        tp_deltaD=len(head_rows[(head_rows[1] == 1) & (head_rows.index >= Di_size)])
        tp_full=len(head_rows[(head_rows[1] == 1)])
        recall_full=tp_full/total_anomalies
        recall_Di = tp_Di/Di_anomalies
        recall_deltaD='NAN'
        if update_type=='CDI':
            recall_deltaD = tp_deltaD/deltaD_anomalies
        print(recall_Di,recall_deltaD,recall_full,end=' ')
    fpr, tpr, thresholds = metrics.roc_curve(scores_labels[1], scores_labels[0], pos_label=1)
    auc_score = metrics.auc(fpr, tpr)
    print(auc_score)

dataset_name = str(sys.argv[1])
update_type = str(sys.argv[2])   # CDI or CDII or CDIII
numiTrees = int(sys.argv[3])
num_iteration = int(sys.argv[4])

Di_path = "../io_directory/Di.csv"
deltaD_path = "../io_directory/deltaD_"+update_type+".csv"


Di = load_data(Di_path)
deltaD = load_data(deltaD_path)
Di_size = len(Di)
deltaD_size = len(deltaD)


data = np.concatenate((Di,deltaD))
sample_size = min(deltaD_size,256)


dict_key = "W" + str(sample_size) + "_T" + str(numiTrees)
#print("extension_level="+str(int(pcb_param_list['extension_level'] * data.shape[1] - 2)))

deltaD_labels=pd.read_csv('../io_directory/deltaD_'+update_type+'_labels.csv',sep=' ',header=None)
Di_labels=pd.read_csv('../io_directory/Di_'+update_type+'_labels.csv',sep=' ',header=None)


Di_anomalies=len(Di_labels[(Di_labels[0] == 1)])
deltaD_anomalies=len(deltaD_labels[(deltaD_labels[0] == 1)])
total_anomalies=Di_anomalies+deltaD_anomalies


avg_Di_score = pd.DataFrame(np.zeros(Di_size),columns=[0])
avg_deltaD_score = pd.DataFrame(np.zeros(deltaD_size),columns=[0])  


for i in range(num_iteration):
    random_state = np.random.RandomState(i)
    initial_time = time.time()
    samples_used = 0
       
        
    ix = np.random.choice(Di_size, size=sample_size, replace=False)
    Di_sample = Di[ix,:-1]
        
        
    F = EiForest(Di_sample, ntrees=numiTrees, sample_size=sample_size,
                                         threshold=pcb_param_list['anomaly_threshold_eif'], ExtensionLevel=int(
                                         pcb_param_list['extension_level'] * data.shape[1] - 2)) 
    #print('model initiated')
    
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
        print("modelupdates",end=' ')
        F.update_model(deltaD_sample.tolist())
    else:
        print('',end=' ')   
    PCBIF_time = time.time()- PCBIF_time

    Di_ascore = pd.DataFrame(columns=[0,1])
    for data_instance in Di:
        x_instance, y_instance = preprocessing_data(data_instance)
        overall_anomaly_score, individual_anomaly_score = F.compute_paths(x_instance) # self.window[-1]
        Di_ascore.loc[len(Di_ascore.index)] = [overall_anomaly_score,Di_labels[0].iloc[len(Di_ascore.index)]]  
        #Di_ascore.to_csv('../io_directory/PCBIF_AS_Di.csv',sep=' ',index=False, header=None)
    
    avg_Di_score[0]+=Di_ascore[0]	



    deltaD_ascore = pd.DataFrame(columns=[0,1])
    for data_instance in deltaD:
        x_instance, y_instance = preprocessing_data(data_instance)
        overall_anomaly_score, individual_anomaly_score = F.compute_paths(x_instance) # self.window[-1]
        deltaD_ascore.loc[len(deltaD_ascore.index)] = [overall_anomaly_score,deltaD_labels[0].iloc[len(deltaD_ascore.index)]]
        #deltaD_ascore.to_csv('../io_directory/PCBIF_AS_deltaD.csv',sep=' ',index=False, header=None)
    
    avg_deltaD_score[0]+=deltaD_ascore[0]	
         
    
    print(PCBIF_time,end=' ')
    
    recall_AUC(pd.concat([Di_ascore,deltaD_ascore]))

