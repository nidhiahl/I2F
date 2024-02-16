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

g++ -o inc_I2forest inc_forest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization
g++ -o aggressive_forest aggressive_forest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization


echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
echo 'i2f_Running_Time aggressive_Running_Time' >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv

echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
echo 'lazy_deltaD lazy_AS_D0 inc_AS_deltaD inc_AS_D0 aggressive_AS_deltaD aggressive_AS_D0' >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv



#echo '-----latest run------' $(date)
#echo '**parameters**' 'itrees='"$numTrees", 'samplesize=min('"$minsamplesize" 'or' "$samplingfraction"*100 '%)'
#echo 'lazy i2f aggresive' >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
for (( run=1; run<=numRuns; run++ )); do
    ./inc_I2forest "$datasetName" 100 0.01 256 "$numUpdates" >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
    python3 AAR.py lazy_AS_deltaD >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
    python3 AAR.py lazy_AS_D0 >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
    python3 AAR.py inc_AS_deltaD >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
    python3 AAR.py inc_AS_D0 >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv

    ./aggressive_forest "$datasetName" 100 0.01 256 "$numUpdates" >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
    python3 AAR.py aggressive_AS_deltaD >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
    python3 AAR.py aggressive_AS_D0 >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
	
	echo  >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
	echo  >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
	
done
echo -e '\n' >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_AAR.csv
echo -e '\n' >> ../results/"$datasetName"/"$updateName"/lazy_i2f_aggressive_TIME.csv
	
