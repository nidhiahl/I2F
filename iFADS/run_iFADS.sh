#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <datasetName> <updateName> <numRuns> <anomalyrate>"
    exit 1
fi
# anomaly rate


datasetName=$1  # name of dataset.
updateName=$2   # CDI or CDII or CDIII.
numRuns=$3      # number of times experiments will be run.
anomalyrate=$4  #synthetic(0.001), MNIST(0.002)
numUpdates=1

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

#g++ -o iFADS iFADS.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization
g++ -o iFADS iFADS_modified.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo 'iFADS_Running_Time' >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv
echo 'iFADS_deltaD_AAR iFADS_deltaD_F1 iFADS_deltaD_AUC iFADS_Di_AAR iFADS_Di_F1 iFADS_Di_AUC' >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv


for (( run=1; run<=numRuns; run++ )); do
#----------iFADS execute---------------
    ./iFADS "$datasetName" 100 0.01 256 "$updateName" "$anomalyrate" >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv

#----------iFADS evaluation----------------    
    python3 sum_ascores.py iFADS_AS_deltaD iFADS_sumScores_deltaD
    python3 sum_ascores.py iFADS_AS_Di iFADS_sumScores_Di
    
    python3 recall_AUC.py "$updateName" iFADS >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv
    
#--------insert blank line in quality file----------------        
    echo  >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv
done

python3 avg_ascores.py iFADS_sumScores_deltaD ../results/"$datasetName"/"$updateName"/iFADS_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py iFADS_sumScores_Di ../results/"$datasetName"/"$updateName"/iFADS_avg_ascore_Di.csv "$numRuns"




echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/iFADS_QUALITY.csv
 
 
 
 
 
 
 
 
 
 
 
 
 
 
    
