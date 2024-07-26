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



dataset_name = "shuttle"
update_type = "U1"   #U1 or U2
D0_path = "./datasets/"+dataset_name+"/"+update_type+"/D0.csv"
deltaD_path = "./datasets/"+dataset_name+"/"+update_type+"/deltaD.csv"

D0 = load_data(D0_path)
deltaD = load_data(deltaD_path)
D0_size = len(D0)
deltaD_size = len(deltaD)


data = np.concatenate((D0,deltaD))

len(data)

print(D0_size, deltaD_size)

print("D0 shape: ", data.shape)
extension = 1.0  # in percent of dimension - 1.0 denotes to fully extended version
print("extension: ", extension)

threshold_EIF = 0.5
num_dimensions = 1
num_samples = 0.1
outlier_content_seen =0


hyper_parameters = [[128,100]]
list_of_mean_F1_scores = []
list_of_mean_exec_times = []
list_of_mean_AUC = []
list_of_feature_scores_PCB_IBFS = list()

for params in hyper_parameters:
    dict_key = "W" + str(params[0]) + "_T" + str(params[1])
    list_of_all_F1_scores = []
    list_of_all_exec_times = []
    list_of_all_AUC = []
    print("classifier PCBIF processing "+ dataset_name + " with hyper_parameters " + dict_key)

    num_iteration = 1
    for i in range(num_iteration):
        random_state = np.random.RandomState(i)
        initial_time = time.time()
        predicted_results = []
        true_outliers = []
        true_positives = 0
        false_positives = 0
        true_negatives  = 0
        false_negatives = 0
        samples_used = 0
        epsilon = 0.00000001  #small +ve value
        PCB_iForest = PCB_EIF_Window(dimension=data.shape[1], window_size=pcb_param_list['window_size'],
                                                     num_trees=pcb_param_list['num_trees'],
                                                     anomaly_threshold=pcb_param_list['anomaly_threshold_eif'],
                                                     extension_level=int(
                                                         pcb_param_list['extension_level'] * data.shape[1] - 2),  # -2 here since label column is still contained in data
                                                     initial_data_size = D0_size, update_data_size = deltaD_size,
                                                     alpha=pcb_param_list['alpha'],
                                                     n_dimensions=pcb_param_list['n_dimensions'],
                                                     n_tested_samples=pcb_param_list['n_tested_samples'],
                                                     stat_size=pcb_param_list['stat_size'])
        
        for data_instance in data:
            #samples_used += 1
            x_instance, y_instance = preprocessing_data(data_instance)
            PCB_iForest.run_train_classifier(x_instance)
            '''
            predicted_value = PCB_iForest.give_prediction()
            predicted_results.append(predicted_value)

            if y_instance   == 1:
                if predicted_value == 1:
                    true_positives +=1
                else:
                    false_negatives +=1
            else:
                if predicted_value == 1:
                    false_positives +=1
                else:
                    true_negatives +=1

        outlier_content_seen = (len(true_outliers)/samples_used) * 100
'''
        
        for data_instance in data:
            samples_used += 1
            x_instance, y_instance = preprocessing_data(data_instance)
            if y_instance == 1:
                true_outliers.append(y_instance)
            #predicted_value = PCB_iForest.predict(x_instance)
            predicted_value = PCB_iForest.give_ascore(sample_used)
            #predicted_results.append(predicted_value)
            print(predicted_value)
            
            
            
            
'''
            if y_instance   == 1:
                if predicted_value == 1:
                    true_positives +=1
                else:
                    false_negatives +=1
            else:
                if predicted_value == 1:
                    false_positives +=1
                else:
                    true_negatives +=1

        outlier_content_seen = (len(true_outliers)/samples_used) * 100        
        
        
        
        
        print("\n")
        print("Total no. of samples: ",samples_used)
        print("outliers content percentage in data: ",outlier_content_seen)
        print("true_positives={},false_positives={},false_negatives={},true_negatives={}".format(true_positives,false_positives,false_negatives,true_negatives))

        try:
                            false_positives_rate = false_positives / (false_positives + true_negatives)
        except ZeroDivisionError:
                            false_positives_rate = epsilon

        try:
                            false_negatives_rate = false_negatives / (true_positives + false_negatives)
        except ZeroDivisionError:
                            false_negatives_rate = epsilon

        try:
                            recall = true_positives / (
                                        true_positives + false_negatives)  # Sensitivity (aka recall, True Positive Rate)
        except ZeroDivisionError:
                            recall = epsilon

        try:
                            specificity = true_negatives / (false_positives + true_negatives)  # Specificity (aka True Negative Rate)
        except ZeroDivisionError:
                            specificity = epsilon

        try:
                            precision = true_positives / (true_positives + false_positives)  # Precision
        except ZeroDivisionError:
                            precision = epsilon

        try:
                            f1_score = 2 * (precision * recall) / (precision + recall)
        except ZeroDivisionError:
                            f1_score = epsilon

        AUC = (1.0 + recall - false_positives_rate) / 2.0

        print("Prediction_metrics:\nrecall={},false_positives_rate={},\nfalse_negatives_rate={},specificity={},\nf1_score={}, precision={},\nAUC={}".format(recall,false_positives_rate,false_negatives_rate,specificity,f1_score,precision,AUC))
        final_time = time.time()
        diff = final_time - initial_time
        print("total runtime: ", diff)
        print("Average runtime per instance: ",diff/samples_used)

        list_of_all_F1_scores.append(f1_score)
        list_of_all_exec_times.append(diff)
        list_of_all_AUC.append(AUC)

        

    mean_F1_Score = mean(list_of_all_F1_scores)
    mean_exec_time = mean(list_of_all_exec_times)
    mean_AUC = mean(list_of_all_AUC)
    list_of_mean_F1_scores.append({dict_key:mean_F1_Score})
    list_of_mean_exec_times.append({dict_key:mean_exec_time})
    list_of_mean_AUC.append({dict_key: mean_AUC})

    print("Hyperparameter: " + dict_key + "\n")
    for score in list_of_all_F1_scores:
        print(json.dumps(score))
    print("Average F1_score: ",json.dumps(mean_F1_Score, indent=2)+ "\n\n")

    for element in list_of_all_AUC:
        print(json.dumps(element))
    print("Average AUC: ",json.dumps(mean_AUC, indent=2) + "\n\n")

    for element in list_of_all_exec_times:
        print(json.dumps(element))
    print("Average runtime: ",json.dumps(mean_exec_time,indent=2)+ "\n\n")
    print("========================================================================================" + "\n")
'''
'''
    with open(result_filename,"a") as f:
        f.write("*************************************%s mean evaluation results***********************************\n\n" %model)
        f.write("\nnum of runs: %s" % num_iteration)
        f.write("\noutliers content percentage in data: %s " % outlier_content_seen)
        f.write("\nNDKSWIN parameters: No. of dimensions: %s, No. of samples tested: %s" % (num_dimensions,num_samples))
        f.write("\nModel name: %s" % model)
        if model == "PCB_EIF":
            f.write("\nextension level : %s" % extension)
            f.write("\nanomaly threshold: %s "% threshold_EIF)
        else:
            f.write("\nanomaly threshold: %s " % threshold_IBFS)
        f.write("\nDataset: %s "% dataFile)
        f.write("\ndata shape: %s" % str(data.shape))
        f.write("\n\nHyperparameter : mean_F1_score")
        for score in list_of_mean_F1_scores:
            f.write(json.dumps(score,sort_keys=True,indent=2) + "\n")

        f.write("Hyperparameter : mean_AUC")
        for element in list_of_mean_AUC:
            f.write(json.dumps(element, sort_keys=True, indent=2) + "\n")

        f.write("Hyperparameter : mean_exec_time")
        for element in list_of_mean_exec_times:
            f.write(json.dumps(element, sort_keys=True, indent=2) + "\n")
'''

