import pandas as pd
import numpy as np
from sklearn import metrics
import sys

#dataset='synthetic'
update_type=str(sys.argv[1])
method = str(sys.argv[2])


actual_labels = pd.read_csv('../io_directory/Di_'+update_type+'_labels.csv',sep=' ',header=None)
actual_labels = pd.concat([actual_labels,pd.read_csv('../io_directory/deltaD_'+update_type+'_labels.csv',sep=' ',header=None)],ignore_index=True)

pred_scores = pd.read_csv('../io_directory/'+method+'_AS_Di.csv',sep=' ',header=None)
Di_size = len(pred_scores)
pred_scores = pd.concat([pred_scores,pd.read_csv('../io_directory/'+method+'_AS_deltaD.csv',sep=' ',header=None)],ignore_index=True)
deltaD_size = len(pred_scores) -Di_size

num_anomalies=actual_labels[0].value_counts()[1]

scores_labels = pd.DataFrame(columns=['score','actual_labels'])
scores_labels['score'] = pred_scores[1]
scores_labels['actual_labels'] = actual_labels

pred_labels = np.zeros(len(actual_labels))
pred_labels[:num_anomalies]=1
scores_labels['pred_labels']=pred_labels



fscore_full=metrics.f1_score(scores_labels['actual_labels'],scores_labels['pred_labels'])

fpr, tpr, thresholds = metrics.roc_curve(scores_labels['actual_labels'], scores_labels['score'], pos_label=1)
auc_score = metrics.auc(fpr, tpr)

print(fscore_full,auc_score)





scores_labels.sort_values(by='score',ascending=False,inplace=True)



#scores_labels.head(30)

scores_labels['rank'] = range(1,Di_size+deltaD_size+1)

AAR_Di=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index < Di_size)]['rank'].mean()

AAR_deltaD=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index >= Di_size)]['rank'].mean()

AAR_full=scores_labels[(scores_labels['actual_labels'] == 1)]['rank'].mean()

LAR_Di=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index < Di_size)]['rank'].min()

LAR_deltaD=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index >= Di_size)]['rank'].min()

LAR_full=scores_labels[(scores_labels['actual_labels'] == 1)]['rank'].min()

HAR_Di=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index < Di_size)]['rank'].max()

HAR_deltaD=scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index >= Di_size)]['rank'].max()

HAR_full=scores_labels[(scores_labels['actual_labels'] == 1)]['rank'].max()


#print(AAR_deltaD, AAR_Di, AAR_full, LAR_deltaD, LAR_Di, LAR_full, HAR_deltaD, HAR_Di, HAR_full,end=' ')


fscore_full=metrics.f1_score(scores_labels['actual_labels'],scores_labels['pred_labels'])

fpr, tpr, thresholds = metrics.roc_curve(scores_labels['actual_labels'], scores_labels['score'], pos_label=1)
auc_score = metrics.auc(fpr, tpr)


recall_full=sum(scores_labels.head(num_anomalies)['actual_labels'] == 1)/num_anomalies

precision_full=sum(scores_labels.head(num_anomalies)['actual_labels'] == 1)/num_anomalies

recall_Di = sum((scores_labels.head(num_anomalies)['actual_labels'] == 1) & (scores_labels.head(num_anomalies).index < Di_size)) /len(scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index < Di_size)])

recall_deltaD=0
if update_type=='CDI':
   recall_deltaD = sum((scores_labels.head(num_anomalies)['actual_labels'] == 1) & (scores_labels.head(num_anomalies).index >= Di_size)) /len(scores_labels[(scores_labels['actual_labels'] == 1) & (scores_labels.index >= Di_size)])

print(AAR_deltaD, AAR_Di, AAR_full, LAR_deltaD, LAR_Di, LAR_full, HAR_deltaD, HAR_Di, HAR_full,recall_deltaD,recall_Di,recall_full,precision_full,fscore_full,auc_score)













