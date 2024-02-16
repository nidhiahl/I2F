#!/bin/bash


cd lazy_i2f_aggressive/
bash run_lazy_i2f_aggressive_experiments.sh shuttle U1 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh shuttle U1 1 10'
bash run_lazy_i2f_aggressive_experiments.sh shuttle U2 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh shuttle U2 1 10'

cd ../iFADS/
bash run_iFADS_experiments.sh shuttle U1 1 10
echo 'run_iFADS_experiments.sh shuttle U1 1 10'
bash run_iFADS_experiments.sh shuttle U2 1 10
echo 'run_iFADS_experiments.sh shuttle U2 1 10'

cd ../our_pcbif/
bash run_PCBIF_experiments.sh shuttle U1 1 10
echo 'run_PCBIF_experiments.sh shuttle U1 1 10'
bash run_PCBIF_experiments.sh shuttle U2 1 10
echo 'run_PCBIF_experiments.sh shuttle U2 1 10'

cd ../lazy_i2f_aggressive/
bash run_lazy_i2f_aggressive_experiments.sh fcovtype U1 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh fcovtype U1 1 10'
bash run_lazy_i2f_aggressive_experiments.sh fcovtype U2 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh fcovtype U2 1 10'

cd ../iFADS/
bash run_iFADS_experiments.sh fcovtype U1 1 10
echo 'run_iFADS_experiments.sh fcovtype U1 1 10'
bash run_iFADS_experiments.sh fcovtype U2 1 10
echo 'run_iFADS_experiments.sh fcovtype U2 1 10'

cd ../our_pcbif/
bash run_PCBIF_experiments.sh fcovtype U1 1 10
echo 'run_PCBIF_experiments.sh fcovtype U1 1 10'
bash run_PCBIF_experiments.sh fcovtype U2 1 10
echo 'run_PCBIF_experiments.sh fcovtype U2 1 10'

cd ../lazy_i2f_aggressive/
bash run_lazy_i2f_aggressive_experiments.sh cicids U1 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh cicids U1 1 10'
bash run_lazy_i2f_aggressive_experiments.sh cicids U2 1 10
echo 'run_lazy_i2f_aggressive_experiments.sh cicids U2 1 10'

cd ../iFADS/
bash run_iFADS_experiments.sh cicids U1 1 10
echo 'run_iFADS_experiments.sh cicids U1 1 10'
bash run_iFADS_experiments.sh cicids U2 1 10
echo 'run_iFADS_experiments.sh cicids U2 1 10'

cd ../our_pcbif/
bash run_PCBIF_experiments.sh cicids U1 1 10
echo 'run_PCBIF_experiments.sh cicids U1 1 10'
bash run_PCBIF_experiments.sh cicids U2 1 10
echo 'run_PCBIF_experiments.sh cicids U2 1 10'


