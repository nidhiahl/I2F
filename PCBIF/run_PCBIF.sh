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



echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/pcbif_TIME_QUALITY.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/pcbif_TIME_QUALITY.csv
echo ' Running_Time recall_Di@anomalies recall_deltaD@anomlaies recall_full@anomlaies recall_Di@10% recall_deltaD@10% recall_full@10% recall_Di@20% recall_deltaD@20% recall_full@20% recall_Di@30% recall_deltaD@30% recall_full@30%' >> ../results/"$datasetName"/"$updateName"/pcbif_TIME_QUALITY.csv

python3 pcbif.py "$datasetName" "$updateName" "$numTrees" "$numRuns" >> ../results/"$datasetName"/"$updateName"/pcbif_TIME_QUALITY.csv

echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/pcbif_TIME_QUALITY.csv


