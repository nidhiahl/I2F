#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <datasetName> <updateName> <numRuns>"
    exit 1
fi

datasetName=$1  # name of dataset.
updateName=$2   # CDI or CDII or CDIII.
numRuns=$3      # number of times experiments will be run.
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


#touch ../io_directory/sum.csv

g++ -o inc_eI2forest inc_eiforest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization
g++ -o aggressive_eiforest aggressive_eiforest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization


echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv
echo 'i2f_Running_Time aggressive_Running_Time' >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv



echo '-----recent-run------'$(date) >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
echo '**parameters**:itrees='"$numTrees"  >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
echo 'recall_Di@anomalies recall_deltaD@anomalies recall_full@anomalies recall_Di@10% recall_deltaD@10% recall_full@10% recall_Di@20% recall_deltaD@20% recall_full@20% recall_Di@30% recall_deltaD@30% recall_full@30% AUC_full' >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv



for (( run=1; run<=numRuns; run++ )); do
    ./inc_eI2forest "$datasetName" 100 0.01 256 "$numUpdates" "$updateName" >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv
        
#------------lazy evalaution---------
    python3 sum_ascores.py eif_lazy_AS_deltaD eif_lazy_sumScores_deltaD
    python3 sum_ascores.py eif_lazy_AS_Di eif_lazy_sumScores_Di
    
    echo -ne 'lazy '\\r >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
    python3 recall_AUC.py "$updateName" lazy >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
    
#----------i2f evalaution------------   
    python3 sum_ascores.py eif_i2f_AS_deltaD eif_i2f_sumScores_deltaD
    python3 sum_ascores.py eif_i2f_AS_Di eif_i2f_sumScores_Di
    
    echo -ne 'i2f '\\r >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
    python3 recall_AUC.py "$updateName" i2f >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
    
    
#---------aggressive execution--------    
    ./aggressive_eiforest "$datasetName" 100 0.01 256 "$numUpdates" "$updateName" >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv

#---------aggressive evalaution--------    
    python3 sum_ascores.py eif_aggressive_AS_deltaD eif_aggressive_sumScores_deltaD
    python3 sum_ascores.py eif_aggressive_AS_Di eif_aggressive_sumScores_Di
    
    echo 'aggressive ' >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
    python3 recall_AUC.py "$updateName" aggressive >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv

#------isert empty rows inresult files-----  
    echo  >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
	echo  >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv
	
done

python3 avg_ascores.py eif_lazy_sumScores_deltaD ../results/"$datasetName"/"$updateName"/eif_lazy_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py eif_lazy_sumScores_Di ../results/"$datasetName"/"$updateName"/eif_lazy_avg_ascore_Di.csv "$numRuns"
python3 avg_ascores.py eif_i2f_sumScores_deltaD ../results/"$datasetName"/"$updateName"/eif_i2f_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py eif_i2f_sumScores_Di ../results/"$datasetName"/"$updateName"/eif_i2f_avg_ascore_Di.csv "$numRuns"
python3 avg_ascores.py eif_aggressive_sumScores_deltaD ../results/"$datasetName"/"$updateName"/eif_aggressive_avg_ascore_deltaD.csv "$numRuns"
python3 avg_ascores.py eif_aggressive_sumScores_Di ../results/"$datasetName"/"$updateName"/eif_aggressive_avg_ascore_Di.csv "$numRuns"



echo -e '\n' >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_QUALITY.csv
echo -e '\n' >> ../results/"$datasetName"/"$updateName"/eif_lazy_i2f_aggressive_TIME.csv
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
