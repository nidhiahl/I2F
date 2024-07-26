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
	data *D0 = new data();
	const data &refD0 = *D0; 
	string D0_path = "io_directory/D0.csv";
	D0->createDataVector(D0_path);
	//construct F0
	int D0_ss;
	int D0_numInstances = D0->getnumInstances();
	D0_ss = D0_numInstances * samplingFactor < minSampleSize ? minSampleSize : D0_numInstances * samplingFactor;
    D0_ss = D0_numInstances < D0_ss ? D0_numInstances : D0_ss;
	iforest *F0 = new iforest(refD0, numOfTrees, D0_ss,5);
	F0->constructiForest();
	//delete D0
	for( int update = 1; update<=no_update_batches; update++){
	
		//read deltaD();
		data *deltaD = new data();
		const data &refdeltaD = *deltaD;
		string deltaD_path = "io_directory/deltaD"+to_string(update)+".csv";
		deltaD->createDataVector(deltaD_path);
		
		
		int deltaD_numInstances = deltaD->getnumInstances();
		
		//evlauate deltaD();
		vector<double> AnomalyScore;
		for(int pointi =0; pointi < deltaD_numInstances;pointi++){
    		AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refdeltaD));
   		}
		
		//write inc_AS_deltaD();
		ofstream outAnomalyScore("io_directory/lazy_AS_deltaD"+to_string(update)+".csv", ios::app|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	for(int pointi = 0; pointi < deltaD_numInstances; pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refdeltaD.dataVector[pointi]->label<<endl;	
	}
	
	//evlauate D0();
		//vector<double> AnomalyScore;
		AnomalyScore.clear();
		AnomalyScore.resize(0);
		for(int pointi =0; pointi < D0_numInstances;pointi++){
    		AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refD0));
   		}
		
		//write inc_AS_D0();
		ofstream outAnomalyScoreD0("io_directory/lazy_AS_D0"+to_string(update)+".csv", ios::app|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	for(int pointi = 0; pointi < D0_numInstances; pointi++){
    	outAnomalyScoreD0<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refD0.dataVector[pointi]->label<<endl;	
	}
		//delete deltaD;
	}
	
}
