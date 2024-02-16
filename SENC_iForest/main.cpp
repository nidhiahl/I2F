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
	string D0_path = "../io_directory/D0.csv";
	string deltaD_path = "../io_directory/deltaD1.csv";
	
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
	const int &minSizeOfNode = atoi(argv[5]);

/********************************Create data vector***********************/
	struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	data *D0 = new data();
    const data &refD0 = *D0;
	D0->createDataVector(D0_path);
    
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;


/********************************training SENCForest***************************/
	struct timespec start_train,end_train;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	
	int sampleSize;
	int totalPoints = D0->getnumInstances();
	sampleSize = totalPoints * samplingFactor < minSampleSize ? minSampleSize : totalPoints * samplingFactor;
    sampleSize = totalPoints < sampleSize ? totalPoints : sampleSize;
    SENCForest *SENCForestObject = new SENCForest(refD0, numOfTrees, sampleSize, minSizeOfNode);
    
    cout<<"SENCForest object created"<<endl;
	int totalLabels=0;
	for(int i=0; i<D0->dataVector.size(); i++)
	{
		totalLabels=max(totalLabels, D0->dataVector[i]->label+1);
	}
	cout<<"total labels"<<totalLabels<<endl;
	SENCForestObject->constructSENCForest(totalLabels);

	
/***************************Evaluate the created SENCForest***************************************/

	data *testDataObject = new data();
    const data &refTestDataObject = *testDataObject;
	testDataObject->createDataVector(deltaD_path);
	vector<double> AnomalyScore;

	for(int pointi =0; pointi < refTestDataObject.getnumInstances();pointi++){
    	AnomalyScore.push_back(SENCForestObject->computeAnomalyScore(pointi, refTestDataObject));
   	}
	
	
/*******************************Write anomaly scores to file***********************************/	
	ofstream outAnomalyScore("anomalyScores/Fi_AnomalyScore.csv", ios::out|ios::binary);
    outAnomalyScore<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < testDataObject->getnumInstances(); pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refTestDataObject.dataVector[pointi]->label<<endl;	
	}

/*******************************Update the SENCForest***************************************/
	D0 = new data();
	D0->createDataVector(deltaD_path);

	totalPoints = D0->getnumInstances();
	sampleSize = totalPoints * samplingFactor < minSampleSize ? minSampleSize : totalPoints * samplingFactor;
    sampleSize = totalPoints < sampleSize ? totalPoints : sampleSize;

	SENCForestObject->deploySENCForest(sampleSize);

	testDataObject = new data();
	testDataObject->createDataVector(deltaD_path);
	AnomalyScore.resize(0);

	for(int pointi =0; pointi < refTestDataObject.getnumInstances();pointi++){
    	AnomalyScore.push_back(SENCForestObject->computeAnomalyScore(pointi, refTestDataObject));
   	}

	ofstream outAnomalyScore2("anomalyScores/IncFi+1_AnomalyScore.csv", ios::out|ios::binary);
    outAnomalyScore2<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < testDataObject->getnumInstances(); pointi++){
    	outAnomalyScore2<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refTestDataObject.dataVector[pointi]->label<<endl;	
	}  
	
	return 0;
}

















