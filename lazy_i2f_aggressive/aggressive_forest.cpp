#include <iostream>
#include <string>
# include "data.cpp"
# include "iforest.cpp"


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>



using namespace std;


int main(int argc, char* argv[]){
	srand(time(0));
	const string datasetName = argv[1];
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
	const int no_update_batches = atoi(argv[5]);
	
	//if F0 exist
	//read F0
	
	//else
	//read D0
	data *Di = new data();
	const data &refDi = *Di; 
	string Di_path = "../io_directory/D0.csv";
	Di->createDataVector(Di_path);
	//construct F0
	int previousD_numInstances;
	int Di_ss;
	int Di_numInstances;
	//delete D0
	for( int update = 1; update<=no_update_batches; update++){
		previousD_numInstances = Di->getnumInstances();
		//previousD_numInstances = 0;
		//read deltaD() + previousD;
		string deltaD_path = "../io_directory/deltaD"+to_string(update)+".csv";
		Di->incrementDataVector(deltaD_path);
		//cout<<previousD_numInstances<<endl;
		//create Fi();
		Di_numInstances = Di->getnumInstances();
		Di_ss = Di_numInstances * samplingFactor < minSampleSize ? minSampleSize : Di_numInstances * samplingFactor;
    	Di_ss = Di_numInstances < Di_ss ? Di_numInstances : Di_ss;
    	
    	struct timespec start_aggressive,end_aggressive;
   		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_aggressive);

		iforest *Fi = new iforest(refDi, numOfTrees, Di_ss,5);
		Fi->constructiForest();
		
		//cout<<"update "<<update<<"done"<<endl;
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_aggressive);
		double aggressiveTime =  (((end_aggressive.tv_sec - start_aggressive.tv_sec) * 1e9)+(end_aggressive.tv_nsec - start_aggressive.tv_nsec))*1e-9;
    	cout<<aggressiveTime<<" ";
		
		
		//write Fi();
		
		//evlauate Fi over deltaD();
		vector<double> AnomalyScore;
		for(int pointi = 0; pointi < Di_numInstances;pointi++){
    		AnomalyScore.push_back(Fi->computeAnomalyScore(pointi, refDi));
   		}
		
		//write aggressive_AS_D0();
		ofstream outAnomalyScoreD0("../io_directory/aggressive_AS_D0.csv", ios::out|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	int numAttributes = Di->getnumAttributes();
    	for(int pointi = 0; pointi < previousD_numInstances; pointi++){
    	outAnomalyScoreD0<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refDi.dataVector[pointi]->label<<endl;
    	}
		
		//write aggressive_AS_deltaD();
		ofstream outAnomalyScoredeltaD("../io_directory/aggressive_AS_deltaD.csv", ios::out|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	//int numAttributes = Di->getnumAttributes();
    	for(int pointi = previousD_numInstances; pointi < Di_numInstances; pointi++){
    	outAnomalyScoredeltaD<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refDi.dataVector[pointi]->label<<endl;
    	}
		//delete deltaD;
	}
	
}
