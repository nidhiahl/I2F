#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys, os

datasetName = 'nyc_taxi'  #name of dataset.
updateName = 'U1'   #U1 or U2.
numUpdates = 213   #number of batches of updates.
#numRuns = int(sys.argv[4])      #number of times experiments will be run.
algorithm = 'incremental'    # aggressive or incremental or lazy.

# transfer data from dataset directory to io_directory.
os.system("rm -r io_directory")
os.system("mkdir io_directory")
os.system("cp datasets/"+datasetName+"/"+updateName+"/D0.csv io_directory/D0.csv")
for numUpdate in range(1,numUpdates+1):
	os.system("cp datasets/"+datasetName+"/"+updateName+"/deltaD"+str(numUpdate)+".csv io_directory/")


#os.system("rm -f "+outputPath+"Time_Mem.csv")
if algorithm == "incremental":
	#run incremental
	os.system('g++ -o i2f_nyc_taxi inc_forest.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization')
	os.system('./i2f_nyc_taxi '+ datasetName+' 100 0.01 64 '+str(numUpdates))
	print("algorithm",algorithm)
elif algorithm == "lazy":
	#run lazy main
	print("algorithm",algorithm)
elif algorithm == "aggresive":
	#run aggressive main
	print("algorithm",algorithm)
else:
	print("\"",algorithm,"\" is an incorrect algorithm name")
	print("Please enter a correct algorithm name from the list,  i. lazy ii. aggresive iii. incremental")

# delete io_directory.


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

