import pandas as pd
import numpy as np
from sklearn import metrics
import sys

#dataset='synthetic'
update_type=str(sys.argv[1])
method = str(sys.argv[2])

#read ground truth label files-------------
actual_labels = pd.read_csv('../io_directory/Di_'+update_type+'_labels.csv',sep=' ',header=None)
actual_labels = pd.concat([actual_labels,pd.read_csv('../io_directory/deltaD_'+update_type+'_labels.csv',sep=' ',header=None)],ignore_index=True)


#read predicted score files-------------
pred_scores = pd.read_csv('../io_directory/'+method+'_AS_Di.csv',sep=' ',header=None)
Di_size = len(pred_scores)
pred_scores = pd.concat([pred_scores,pd.read_csv('../io_directory/'+method+'_AS_deltaD.csv',sep=' ',header=None)],ignore_index=True)
deltaD_size = len(pred_scores) -Di_size


# count of anomlaies in Di, deltaD, and total-----------
total_anomalies=actual_labels[0].value_counts()[1]
Di_anomalies=len(actual_labels[(actual_labels[0] == 1) & (actual_labels.index < Di_size)])
deltaD_anomalies=len(actual_labels[(actual_labels[0] == 1) & (actual_labels.index >= Di_size)])
#print(total_anomalies,Di_anomalies,deltaD_anomalies)


#put scores and labels in one dataframe---------------
scores_labels = pd.DataFrame(columns=['score','actual_labels'])
scores_labels['score'] = pred_scores[1]
scores_labels['actual_labels'] = actual_labels

#sort the anomlayscores in descending order
scores_labels.sort_values(by='score',ascending=False,inplace=True)



#add predicted labels column in dataframe------
pred_labels = np.zeros(len(actual_labels))
pred_labels[:total_anomalies]=1
scores_labels['pred_labels']=pred_labels


for i in [total_anomalies, int((Di_size+deltaD_size)*0.1), int((Di_size+deltaD_size)*0.2), int((Di_size+deltaD_size)*0.3)]:
	head_rows=scores_labels.head(i)
	tp_Di=len(head_rows[(head_rows['actual_labels'] == 1) & (head_rows.index < Di_size)])
	tp_deltaD=len(head_rows[(head_rows['actual_labels'] == 1) & (head_rows.index >= Di_size)])
	tp_full=len(head_rows[(head_rows['actual_labels'] == 1)])
	recall_full=tp_full/total_anomalies
	recall_Di = tp_Di/Di_anomalies
	recall_deltaD='NAN'
	if update_type=='CDI':
		recall_deltaD = tp_deltaD/deltaD_anomalies
	print(recall_Di,recall_deltaD,recall_full,end=' ')




'''
#calcultae true positves for Di, deltaD, full
head_rows=scores_labels.head(3*total_anomalies)
tp_Di=len(head_rows[(head_rows['actual_labels'] == 1) & (head_rows.index < Di_size)])
tp_deltaD=len(head_rows[(head_rows['actual_labels'] == 1) & (head_rows.index >= Di_size)])
tp_full=len(head_rows[(head_rows['actual_labels'] == 1)])


#calculate recall Di, deltaD, recall_full
recall_full=tp_full/total_anomalies
recall_Di = tp_Di/Di_anomalies
recall_deltaD='NAN'
if update_type=='CDI':
    recall_deltaD = tp_deltaD/deltaD_anomalies
'''

#calculate aUC for Di_deltaD
fpr, tpr, thresholds = metrics.roc_curve(scores_labels['actual_labels'], scores_labels['score'], pos_label=1)
auc_score = metrics.auc(fpr, tpr)



print(auc_score)












