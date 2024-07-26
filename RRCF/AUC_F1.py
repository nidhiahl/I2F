import pandas as pd
import numpy as np
from sklearn import metrics
import sys

#dataset='synthetic'
update_type=str(sys.argv[1])
testset_name=str(sys.argv[2])
method = str(sys.argv[3])

pred_scores = pd.read_csv('../io_directory/'+method+'_AS_'+testset_name+'.csv',sep=' ',header=None)
actual_labels = pd.read_csv('../io_directory/'+testset_name+'_'+update_type+'_labels.csv',sep=' ',header=None)
num_anomalies=actual_labels[0].value_counts()[1]

scores_labels = pd.DataFrame(columns=['score','actual_labels'])
scores_labels['score'] = pred_scores[0]
scores_labels['actual_labels'] = actual_labels
scores_labels.sort_values(by='score',ascending=False,inplace=True)

pred_labels = np.zeros(len(actual_labels))
pred_labels[:num_anomalies]=1
scores_labels['pred_labels']=pred_labels

f_score=metrics.f1_score(scores_labels['actual_labels'],scores_labels['pred_labels'])

fpr, tpr, thresholds = metrics.roc_curve(scores_labels['actual_labels'], scores_labels['score'], pos_label=1)
auc_score = metrics.auc(fpr, tpr)

print(f_score,auc_score,end=' ')
