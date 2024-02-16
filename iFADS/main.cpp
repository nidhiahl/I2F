#include <iostream>
#include <string>
#include "data.cpp"
#include "iforest.cpp"
#include <string>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

using namespace std;

int parseLine(char *line){
    int i = strlen(line);
    const char *p = line;
    while(*p <'0' || *p >'9')
	p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(int par){
    FILE *file = fopen("/proc/self/status","r");
    int result = -1;
    char line[128];

    while(fgets(line,128,file) != NULL){
	if(par == 1){               //Ram used
   	    if(strncmp(line, "VmRSS:", 6) == 0){
		result = parseLine(line);
		break;
	    }
	}
	if(par == 2){               //virtual memory used
   	    if(strncmp(line, "VmSize:", 7) == 0){
		result = parseLine(line);
		break;
	    }
	}
    }
    fclose(file);
    return result;
}


int main(int argc, char* argv[])
{
    //i) dataFile: original data file name having complete dataset
	//ii) numOfTrees: count of itreees in the iforest.
	//iii) smaplingFactor: Fraction of total points in dataset that will act as the sampleSize for iTree creation. 0 < samplingFactor < 1
	//iv) minSampleSize: an interger value, useful when samplingFactor fails to give sufficient samples to create an iTree, when datset is small*/
    srand(time(0));
	const string &datasetName = argv[1];
	
    const int &numOfTrees = atoi(argv[2]);
	
    const double &samplingFactor = atof(argv[3]);
	
    const int &minSampleSize = atoi(argv[4]);
	
    const int &windowSize = atoi(argv[5]);
    /************************************************D0 dataPreparation******************************************************************/
	
    struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);
    
    data *D0 = new data();
	const data &refD0 = *D0; 
	string D0_path = "../io_directory/D0.csv";
	D0->createDataVector(D0_path);
	int D0_ss;
	int D0_numInstances = D0->getnumInstances();
	D0_ss = D0_numInstances * samplingFactor < minSampleSize ? minSampleSize : D0_numInstances * samplingFactor;
    D0_ss = D0_numInstances < D0_ss ? D0_numInstances : D0_ss;
	
	
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;
    cout<<"Time taken in dP: "<< fixed<<dPTime<<"sec"<<endl;

	/************************************************static iForest creation***************************************************/
    
	iforest *F0 = new iforest(refD0, numOfTrees, D0_ss);
	F0->constructiForest();
	
	
	
	/************************************************deltaD dataPreparation**********************************************************/
	int update =1;
	data *deltaD = new data();
	const data &refdeltaD = *deltaD;
	string deltaD_path = "../io_directory/deltaD"+to_string(update)+".csv";
	deltaD->createDataVector(deltaD_path);
			
	int deltaD_ss;
	int deltaD_numInstances = deltaD->getnumInstances();
	deltaD_ss = deltaD_numInstances * samplingFactor < minSampleSize ? minSampleSize : deltaD_numInstances * samplingFactor;
    deltaD_ss = windowSize < deltaD_ss ? windowSize : deltaD_ss;
    	
    
    /************************************************iFADS updates over deltaD***************************************************/
    int iForestRamUsed = getValue(1);
	//iforest *iForestObject = new iforest(trainDataObject, numOfTrees, sampleSize);
	
	
    struct timespec start_iF,end_iF;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iF);
	//Sliding iForest Logic****
    int windowStartIndex=0;
    double anomaly_rate=0.05;

    //F0->constructiFADSForest(windowStartIndex, windowSize, deltaD_ss);
    vector<pair<double, int>> AnomalyScores;
    vector<double> AnomalyScore;

    for(int windowStartIndex=0; windowStartIndex<deltaD_numInstances; windowStartIndex+=windowSize)
    {
    	cout<<"windowStartIndex="<<windowStartIndex<<" deltaD_numInstances="<<deltaD_numInstances<<" windowSize="<<windowSize<<endl;
    
        int actualAnomalies=0;
        int truePositives=0;
        F0->anomalyScore.clear();
        AnomalyScores.clear();

        for(int pointi=windowStartIndex; pointi<windowStartIndex+windowSize && pointi <deltaD_numInstances; pointi++)
        {
            F0->computeAnomalyScore(pointi, refdeltaD);
            AnomalyScores.push_back({F0->anomalyScore[pointi-windowStartIndex], pointi});
        }

        sort(AnomalyScores.begin(), AnomalyScores.end());
        reverse(AnomalyScores.begin(), AnomalyScores.end());

        for(pair<double, int> tmp: AnomalyScores)
        {
            if(refdeltaD.dataVector[tmp.second]->label == 1) actualAnomalies++;
        }

        for(int i=0; i<actualAnomalies; i++)
        {
            if(refdeltaD.dataVector[AnomalyScores[i].second]->label == 1) truePositives++;
        }

        if(truePositives > anomaly_rate*windowSize)
        {
        	delete F0;
            F0 = new iforest(refdeltaD, numOfTrees, deltaD_ss);
			F0->constructiFADSForest(windowStartIndex, windowSize);
			
			for(int pointi=windowStartIndex; pointi<windowStartIndex+windowSize && pointi <deltaD_numInstances; pointi++)
        	{
            	F0->computeAnomalyScore(pointi, refdeltaD);
            	AnomalyScore.push_back(F0->anomalyScore[pointi]);
        	}
        	
        }
        
    }
    

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iF);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double iFTime =  (((end_iF.tv_sec - start_iF.tv_sec) * 1e9)+(end_iF.tv_nsec - start_iF.tv_nsec))*1e-9;

    //cout<<"RAM="<<iForestRamUsed<<endl;
    //cout << "Time taken to create iforest: " << fixed << iFTime<<"sec"<<endl;
	
	 

	/*****************************Anomaly detection(AD): Path length computation*********************************************************/
	
	struct timespec start_AD,end_AD;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_AD);
    
	for(int pointi =0; pointi < testDataObject.getnumInstances();pointi++){
    	iForestObject->computeAnomalyScore(pointi, testDataObject);
   	}
   	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_AD);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double ADTime =  (((end_AD.tv_sec - start_AD.tv_sec) * 1e9)+(end_AD.tv_nsec - start_AD.tv_nsec))*1e-9;
    
    //vector<double> AnomalyScore = iForestObject->anomalyScore;

	delete iForestObject;
	//cout<<"RAM="<<iForestRamUsed<<endl;
    
	cout<<"dPTime: "<<dPTime<<" iFTime: "<<iFTime<<" ADTime: "<<ADTime<<" iFMemUsed:  "<<iForestRamUsed<<endl;


	/****************************************Anomaly Score writing to file**************************************************************/

    string outputFileName="anomalyScores/"+dataFile2.substr(10,dataFile2.length()-14)+"_tested_over_"+dataFile.substr(10);
     cout<<outputFileName<<endl;
	ofstream outAnomalyScore(outputFileName, ios::out|ios::binary);
    outAnomalyScore<<"pointId "<<"Ascore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < testDataObject.getnumInstances(); pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<testDataObject.dataVector[pointi]->label<<endl;
    }



    return 0;
}
