#!/bin/bash


dataset_name=(2d_synthetic)
anomlayrate_iFADS=(0.003)
methods=("eif_lazy_i2f_aggressive" "lazy_i2f_aggressive" "SENCForest" "PCBIF" "iFADS" "RRCF")
drifts=("CDI" "CDII" "CDIII")
num_runs=5


for i in `seq 1 ${#dataset_name[@]}` 
do
	for method in "${methods[@]}" 
	do
    	cd "$method"
    	for drift in "${drifts[@]}" 
    	do
    		echo ****running "$method" "for" "${dataset_name[i-1]}" "dataset for" "$drift"" drift ""$num_runs" "runs****"
    		if [[ "$method" == "iFADS" ]]; then
    			bash run_"$method".sh "${dataset_name[i-1]}" "$drift" "$num_runs" "${anomlayrate_iFADS[i-1]}"
    		else
    			bash run_"$method".sh "${dataset_name[i-1]}" "$drift" "$num_runs"
    		fi;
    		echo ''
    	done;
    	cd ..
	done;
done;


exit;



