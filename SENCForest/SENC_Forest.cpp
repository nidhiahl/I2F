#include <iostream>
#include <string>
# include "data.cpp"
# include "SENCForest.cpp"
/*#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>*/

using namespace std;


int main(int argc, char* argv[])
{
	srand(time(0));
	// const string run_type = argv[1];
	const string & dataset = argv[1];
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
	const int &minSizeOfNode = atoi(argv[5]);
	const string update_type = argv[6];
	string Di_path = "../io_directory/Di.csv";
	string deltaD_path = "../io_directory/deltaD_"+update_type+".csv";
	
	

/********************************Create data vector***********************/
	struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	data *Di = new data();
     data &refDi = *Di;
	Di->createDataVector(Di_path);
    
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;


/********************************training SENCForest***************************/
	struct timespec start_train,end_train;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	
	int sampleSize;
	int totalPoints = Di->getnumInstances();
	sampleSize = totalPoints * samplingFactor < minSampleSize ? minSampleSize : totalPoints * samplingFactor;
    sampleSize = totalPoints < sampleSize ? totalPoints : sampleSize;
    SENCForest *SENCForestObject = new SENCForest(refDi, numOfTrees, sampleSize, minSizeOfNode);
    
    //cout<<"SENCForest object created"<<endl;
	int totalLabels=0;
	for(int i=0; i<Di->dataVector.size(); i++)
	{
		totalLabels=max(totalLabels, Di->dataVector[i]->label+1);
	}
	//cout<<"total labels"<<totalLabels<<endl;
	SENCForestObject->constructSENCForest(totalLabels);

	
/***************************Evaluate the created SENCForest***************************************/
/*
	data *testDataObject = new data();
    const data &refTestDataObject = *testDataObject;
	testDataObject->createDataVector(deltaD_path);
	vector<double> AnomalyScore;

	for(int pointi =0; pointi < refTestDataObject.getnumInstances();pointi++){
    	AnomalyScore.push_back(SENCForestObject->computeAnomalyScore(pointi, refTestDataObject));
   	}
*/
	
/*******************************Write anomaly scores to file***********************************/	
/*
	ofstream outAnomalyScore("../io_directory/lazy_SENCForest_AS_deltaD.csv", ios::out|ios::binary);
    //outAnomalyScore<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < testDataObject->getnumInstances(); pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refTestDataObject.dataVector[pointi]->label<<endl;	
	}
*/
/*******************************Update the SENCForest***************************************/
	struct timespec start_updateSENC,end_updateSENC;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_updateSENC);

	
	data *deltaD = new data();
	 data &refdeltaD = *deltaD;
	deltaD->createDataVector(deltaD_path);


	int deltaD_ss;
	int deltaD_numInstances = deltaD->getnumInstances();
	deltaD_ss = deltaD_numInstances * samplingFactor < minSampleSize ? minSampleSize : deltaD_numInstances * samplingFactor;
    deltaD_ss = deltaD_numInstances < deltaD_ss ? deltaD_numInstances : deltaD_ss;

	SENCForestObject->deploySENCForest(deltaD_ss,refdeltaD);


	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_updateSENC);
    double updateSENCTime =  (((end_updateSENC.tv_sec - start_updateSENC.tv_sec) * 1e9)+(end_updateSENC.tv_nsec - start_updateSENC.tv_nsec))*1e-9;

	cout<<updateSENCTime<<" ";
    		
	
	vector<double> AnomalyScore;
	AnomalyScore.resize(0);

	for(int pointi =0; pointi < refdeltaD.getnumInstances();pointi++){
    	AnomalyScore.push_back(SENCForestObject->computeAnomalyScore(pointi, refdeltaD));
   	}

	ofstream outAnomalyScore1("../io_directory/SENCForest_AS_deltaD.csv", ios::out|ios::binary);
    //outAnomalyScore1<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < deltaD->getnumInstances(); pointi++){
    	outAnomalyScore1<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refdeltaD.dataVector[pointi]->label<<endl;	
	}  
	
	AnomalyScore.resize(0);

	for(int pointi =0; pointi < Di->getnumInstances();pointi++){
    	AnomalyScore.push_back(SENCForestObject->computeAnomalyScore(pointi, refDi));
   	}
	cout<<"refDi.getnumInstances()="<<Di->getnumInstances()<<endl;
	ofstream outAnomalyScore2("../io_directory/SENCForest_AS_Di.csv", ios::out|ios::binary);
    //outAnomalyScore2<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < Di->getnumInstances(); pointi++){
    	outAnomalyScore2<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refDi.dataVector[pointi]->label<<endl;	
	}  
	
	return 0;
}

















