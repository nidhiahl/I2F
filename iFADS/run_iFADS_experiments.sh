#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <datasetName> <updateName> <numUpdates> <numRuns>"
    exit 1
fi

datasetName=$1  # name of dataset.
updateName=$2   # U1 or U2.
numUpdates=$3   # number of batches of updates.
numRuns=$4      # number of times experiments will be run.


numTrees=100
samplingfraction=0.01
minsamplesize=256
# Transfer data from dataset directory to io_directory.
rm -r ../io_directory
mkdir ../io_directory
mkdir -p ../results/"$datasetName"/"$updateName"
cp ../datasets/"$datasetName"/"$updateName"/D0.csv ../io_directory/D0.csv
for (( numUpdate=1; numUpdate<=numUpdates; numUpdate++ )); do
    cp ../datasets/"$datasetName"/"$updateName"/deltaD"$numUpdate".csv ../io_directory/
done

g++ -o iFADS iFADS.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo 'iFADS_Running_Time' >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
echo 'iFADS_AAR_deltaD iFADS_AAR_D0' >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv


for (( run=1; run<=numRuns; run++ )); do
    ./iFADS "$datasetName" 100 0.01 256 >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
    python3 AAR.py iFADS_AS_deltaD >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
    python3 AAR.py iFADS_AS_D0 >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
    echo  >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
done
echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/iFADS_TIME.csv
echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/iFADS_AAR.csv
    
