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


# Transfer data from dataset directory to io_directory.
rm -r ../io_directory 2>/dev/null
mkdir ../io_directory 2>/dev/null 
mkdir -p ../results/"$datasetName"/"$updateName" 2>/dev/null

cp ../datasets/"$datasetName"/"$updateName"/D0.csv ../io_directory/D0.csv 2>/dev/null

for (( numUpdate=1; numUpdate<=numUpdates; numUpdate++ )); do
    cp ../datasets/"$datasetName"/"$updateName"/deltaD"$numUpdate".csv ../io_directory/ 2>/dev/null
done


#echo 'lazy_deltaD lazy_AS_D0 inc_AS_deltaD inc_AS_D0 aggressive_AS_deltaD aggressive_AS_D0' #>> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/rrcf_TIME_AAR.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/rrcf_TIME_AAR.csv
echo 'Running_Time AAR_deltaD AAR_D0' >> ../results/"$datasetName"/"$updateName"/rrcf_TIME_AAR.csv


for (( run=1; run<=numRuns; run++ )); do
    python3 our_rrcf.py >> ../results/"$datasetName"/"$updateName"/rrcf_TIME_AAR.csv

done
echo -e '\n\n' >> ../results/"$datasetName"/"$updateName"/rrcf_TIME_AAR.csv


