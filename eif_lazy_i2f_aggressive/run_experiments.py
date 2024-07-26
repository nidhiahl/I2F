#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys, os

datasetName = str(sys.argv[1])  #name of dataset.
updateName = str(sys.argv[2])   #U1 or U2.
numUpdates = int(sys.argv[3])   #number of batches of updates.
numRuns = int(sys.argv[4])      #number of times experiments will be run.

# transfer data from dataset directory to io_directory.
os.system("rm -r io_directory")
os.system("mkdir io_directory")
os.system("cp datasets/"+datasetName+"/"+updateName+"/D0.csv io_directory/D0.csv")
for numUpdate in range(1,numUpdates+1):
	os.system("cp datasets/"+datasetName+"/"+updateName+"/deltaD"+str(numUpdate)+".csv io_directory/")

os.system('g++ -o inc_I2forest inc_forest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization')
os.system('g++ -o aggressive_forest aggressive_forest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization')
	

print('lazy_deltaD lazy_AS_D0 inc_AS_deltaD inc_AS_D0 aggressive_AS_deltaD aggressive_AS_D0')
for run in range(1,numRuns+1):
	os.system('./inc_I2forest '+ datasetName+' 100 0.01 256 '+str(numUpdates))
	os.system('python3 AAR.py lazy_AS_deltaD')
	os.system('python3 AAR.py lazy_AS_D0')
	os.system('python3 AAR.py inc_AS_deltaD')
	os.system('python3 AAR.py inc_AS_D0')
	
	os.system('./aggressive_forest '+ datasetName+' 100 0.01 256 '+str(numUpdates))
	os.system('python3 AAR.py aggressive_AS_deltaD')
	os.system('python3 AAR.py aggressive_AS_D0')
	
	print()


#os.system("rm -f "+outputPath+"Time_Mem.csv")
#run incremental


'''
Di = "inputdatafiles/"+datasetName+"/"+updateName+"/Di.csv"
deltaD = "inputdatafiles/"+datasetName+"/"+updateName+"/deltaD.csv"
Di1 = "inputdatafiles/"+datasetName+"/"+updateName+"/Di1.csv"
testD = "inputdatafiles/"+datasetName+"/"+updateName+"/deltaD.csv"
outputPath = "outputfiles/"+datasetName+"/"+updateName+"/"
numTrees = 100
samplingFactor = 0.01
sampleSize = 256
hyperplane_dimension = 5

os.system("g++ -o fullExperiments main.cpp itree.cpp iforest.cpp data.cpp treenode.cpp -lboost_serialization")

os.system("rm -f "+outputPath+"Time_Mem.csv")
os.system("rm -f "+outputPath+"Fi_AnomalyScore.csv")
os.system("rm -f "+outputPath+"IncFi+1_AnomalyScore.csv")
os.system("rm -f "+outputPath+"Fi+1_AnomalyScore.csv")
for runs in range(1,numRuns+1):
    for type_run in ["Fi", "IncFi+1", "Fi+1"]:
        #print(runs,type_run) 
        os.system("./fullExperiments "+ type_run +" "+ Di +" "+ deltaD +" "+ Di1 +" "+ testD +" "+ str(numTrees) +" "+ str(samplingFactor) +" "+ str(sampleSize) +" "+ outputPath +" "+ str(hyperplane_dimension) +" >> "+outputPath +"Time_Mem.csv")      
        

'''

