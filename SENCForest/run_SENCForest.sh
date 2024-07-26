#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <dataset> <updateName> <numRuns>"
    exit 1
fi

datasetName=$1  # name of dataset.
updateName=$2   # CDI or CDII or CDIII.
numRuns=$3      # number of times experiments will be run.
minSize_nodes=10

numTrees=100
samplingfraction=0.01
minsamplesize=256
# Transfer data from dataset directory to io_directory.
rm -r ../io_directory 2>/dev/null
mkdir ../io_directory 2>/dev/null
mkdir -p ../results/"$datasetName"/"$updateName" 2>/dev/null

cp ../datasets/"$datasetName"/Di.csv ../io_directory/Di.csv 2>/dev/null

cp ../datasets/"$datasetName"/Di_"$updateName"_labels.csv ../io_directory/ 2>/dev/null

cp ../datasets/"$datasetName"/deltaD_"$updateName".csv ../io_directory/ 2>/dev/null

cp ../datasets/"$datasetName"/deltaD_"$updateName"_labels.csv ../io_directory/ 2>/dev/null


#touch ../io_directory/sum.csv

g++ -o SENC_Forest SENC_Forest.cpp 


echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv
echo 'SENCForest_Running_Time ' >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv
echo 'recall_Di@anomalies recall_deltaD@anomalies recall_full@anomalies recall_Di@10% recall_deltaD@10% recall_full@10% recall_Di@20% recall_deltaD@20% recall_full@20% recall_Di@30% recall_deltaD@30% recall_full@30% AUC_full' >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv


for (( run=1; run<=numRuns; run++ )); do
#-----------SENCForest execution------------
    ./SENC_Forest "$datasetName" 100 0.01 256 "$minSize_nodes" "$updateName" >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv
 
 
 
#----------SENCForest evaluation-------------------------   
    python3 sum_ascores.py SENCForest_AS_deltaD SENCForest_sumScores_deltaD
    python3 sum_ascores.py SENCForest_AS_Di SENCForest_sumScores_Di
    
    python3 recall_AUC.py "$updateName" SENCForest >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv

#---------insert blank line in quality file-------------------------   
    echo  >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv
	echo  >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv
	
done



python3 avg_ascores.py SENCForest_sumScores_deltaD ../results/"$datasetName"/"$updateName"/SENCForest_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py SENCForest_sumScores_Di ../results/"$datasetName"/"$updateName"/SENCForest_avg_ascore_Di.csv "$numRuns"



echo -e '\n' >> ../results/"$datasetName"/"$updateName"/SENCForest_QUALITY.csv
echo -e '\n' >> ../results/"$datasetName"/"$updateName"/SENCForest_TIME.csv

	
	
	
	
	
	
	
	
	
	
