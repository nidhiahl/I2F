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
	
	const string update_type = argv[5];
	
	const double & anomlayrate = atof(argv[6]);
	
	/************************************************D0 dataPreparation******************************************************************/
	
    data *D0 = new data();
	const data &refD0 = *D0; 
	string D0_path = "../io_directory/Di.csv";
	D0->createDataVector(D0_path);
	int D0_ss;
	int D0_numInstances = D0->getnumInstances();
	D0_ss = D0_numInstances * samplingFactor < minSampleSize ? minSampleSize : D0_numInstances * samplingFactor;
    D0_ss = D0_numInstances < D0_ss ? D0_numInstances : D0_ss;
	
	/************************************************static iForest creation over D0***************************************************/
    
	iforest *F0 = new iforest(refD0, numOfTrees, D0_ss);
	F0->constructiForest();
	
	double anomaly_rate=anomlayrate;
	
    vector<pair<double, int>> AnomalyScores;
    //vector<double> AnomalyScore;

    
    F0->anomalyScore.clear();
    AnomalyScores.clear();
    long double temp_ascore;
    for(int pointi=0; pointi<D0_numInstances; pointi++)
    {
	     temp_ascore = F0->computeAnomalyScore(pointi, refD0);
         AnomalyScores.push_back({temp_ascore, pointi});
    }
    sort(AnomalyScores.begin(), AnomalyScores.end());
    reverse(AnomalyScores.begin(), AnomalyScores.end());
	
	//cout<<"anomaly_rate*D0_numInstances="<<anomaly_rate*D0_numInstances<<" ";
	//cout<<"AnomalyScores[0].first="<<AnomalyScores[0].first<<" ";
	//cout<<"AnomalyScores[int(anomaly_rate*D0_numInstances)].first="<<AnomalyScores[int(anomaly_rate*D0_numInstances)].first<<" ";
	double ascore_threshold= AnomalyScores[int(anomaly_rate*D0_numInstances)].first;
	//cout<<"ascore_threshold="<<ascore_threshold<<" ";
	
	/************************************************deltaD dataPreparation**********************************************************/
	int update =1;
	data *deltaD = new data();
	const data &refdeltaD = *deltaD;
	string deltaD_path = "../io_directory/deltaD_"+update_type+".csv";
	deltaD->createDataVector(deltaD_path);
			
	int deltaD_ss;
	int deltaD_numInstances = deltaD->getnumInstances();
	deltaD_ss = deltaD_numInstances < D0_ss ? deltaD_numInstances : D0_ss;	
    
    /************************************************iFADS updates over deltaD***************************************************/
    
    struct timespec start_iFADS,end_iFADS;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iFADS);
	
    vector<double> AnomalyScore;
    F0->anomalyScore.clear();
    AnomalyScores.clear();
    int predicted_anomalies = 0;
    for(int pointi=0; pointi<deltaD_numInstances; pointi++)
    {	
    	double as = F0->computeAnomalyScore(pointi, refdeltaD);
    	//cout<<"AS="<<as<<" ";
    	if(as>=ascore_threshold){
    		predicted_anomalies++;
    	}
    }
    cout<<"predicted_anomalies/deltaD_numInstances="<<float(predicted_anomalies)/float(deltaD_numInstances)<<"---anomaly_rate="<<anomaly_rate;
    if(float(predicted_anomalies)/float(deltaD_numInstances) > anomaly_rate){
    	delete F0;
        F0 = new iforest(refdeltaD, numOfTrees, deltaD_ss);
		F0->constructiForest();
		cout<<"newforestcreated ";

       	
    }else{cout<<" ";}
    

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iFADS);
	double iFTime =  (((end_iFADS.tv_sec - start_iFADS.tv_sec) * 1e9)+(end_iFADS.tv_nsec - start_iFADS.tv_nsec))*1e-9;

    cout <<fixed << iFTime<<" "<<endl;
	
	 

	/*****************************Anomaly detection(AD): Path length computation*********************************************************/
	
	struct timespec start_AD,end_AD;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_AD);
    
	for(int pointi =0; pointi < deltaD_numInstances;pointi++){
    	AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refdeltaD));
   	}
   	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_AD);
	double ADTime =  (((end_AD.tv_sec - start_AD.tv_sec) * 1e9)+(end_AD.tv_nsec - start_AD.tv_nsec))*1e-9;
    
    
    string outputFileName = "../io_directory/iFADS_AS_deltaD.csv";
    ofstream outAnomalyScore_deltaD(outputFileName, ios::out|ios::binary);
    //outAnomalyScore<<"pointId "<<"Ascore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < deltaD_numInstances; pointi++){
    	outAnomalyScore_deltaD<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refdeltaD.dataVector[pointi]->label<<endl;
    }
    
    
    AnomalyScore.clear();
    AnomalyScore.resize(0);
    for(int pointi =0; pointi < D0_numInstances;pointi++){
    	AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refD0));
   	}
   	
    
    outputFileName = "../io_directory/iFADS_AS_Di.csv";
    ofstream outAnomalyScore_D0(outputFileName, ios::out|ios::binary);
    //outAnomalyScore<<"pointId "<<"Ascore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < D0_numInstances; pointi++){
    	outAnomalyScore_D0<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refD0.dataVector[pointi]->label<<endl;
    }



    return 0;
}
