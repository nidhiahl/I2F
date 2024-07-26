#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <dataset> <updateName> <numRuns>"
    exit 1
fi

datasetName=$1  # name of dataset.
updateName=$2   # CDI or CDII or CDIII
numRuns=$3      # number of times experiments will be run.

numTrees=100


# Transfer data from dataset directory to io_directory.
rm -r ../io_directory 2>/dev/null
mkdir ../io_directory 2>/dev/null 
mkdir -p ../results/"$datasetName"/"$updateName" 2>/dev/null

cp ../datasets/"$datasetName"/Di.csv ../io_directory/Di.csv 2>/dev/null

cp ../datasets/"$datasetName"/Di_"$updateName"_labels.csv ../io_directory/ 2>/dev/null

cp ../datasets/"$datasetName"/deltaD_"$updateName".csv ../io_directory/ 2>/dev/null

cp ../datasets/"$datasetName"/deltaD_"$updateName"_labels.csv ../io_directory/ 2>/dev/null


#echo 'lazy_deltaD lazy_AS_D0 inc_AS_deltaD inc_AS_D0 aggressive_AS_deltaD aggressive_AS_D0' #>> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv
echo 'running_time recall_Di@anomalies recall_deltaD@anomalies recall_full@anomalies recall_Di@10% recall_deltaD@10% recall_full@10% recall_Di@20% recall_deltaD@20% recall_full@20% recall_Di@30% recall_deltaD@30% recall_full@30% AUC_full' >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv

  
for (( run=1; run<=numRuns; run++ )); do

#----------RRCF execution-----------------
    python3 our_RRCF.py "$datasetName" "$updateName" >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv

#-----------RRCF evalaution---------------	
	python3 sum_ascores.py RRCF_AS_deltaD RRCF_sumScores_deltaD
    python3 sum_ascores.py RRCF_AS_Di RRCF_sumScores_Di
     
    python3 recall_AUC.py "$updateName" RRCF >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv


#----------insert blank ne wline in results file-----------	
	echo '' >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv

done

python3 avg_ascores.py RRCF_sumScores_deltaD ../results/"$datasetName"/"$updateName"/RRCF_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py RRCF_sumScores_Di ../results/"$datasetName"/"$updateName"/RRCF_avg_ascore_Di.csv "$numRuns"



echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/RRCF_TIME_QUALITY.csv


