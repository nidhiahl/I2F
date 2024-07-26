#include <iostream>
#include <string>
#include "./vanillaif/data.cpp"
#include "./vanillaif/iforest.cpp"


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
	const string update_type = argv[6];
	
	
	//if F0 exist
	//read F0
	
	//else
	//read D0
	data *D0 = new data();
	const data &refD0 = *D0; 
	string D0_path = "../io_directory/Di.csv";
	D0->createDataVector(D0_path);
	//construct F0
	int D0_ss;
	int D0_numInstances = D0->getnumInstances();
	D0_ss = D0_numInstances * samplingFactor < minSampleSize ? minSampleSize : D0_numInstances * samplingFactor;
    D0_ss = D0_numInstances < D0_ss ? D0_numInstances : D0_ss;
	//iforest *F0 = new iforest(refD0, numOfTrees, D0_ss,5);
	iforest *F0 = new iforest(refD0, numOfTrees, D0_ss);
	F0->constructiForest();
	
	//evlauate D0 over F0;
	vector<double> AnomalyScore;
	for(int pointi =0; pointi < D0_numInstances;pointi++){
    	AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refD0));
   	}
   	
   	//write lazy_AS_D0();
	ofstream outAnomalyScore_lazyD0("../io_directory/lazy_AS_Di.csv", ios::out|ios::binary);
    //outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < D0_numInstances; pointi++){
    	outAnomalyScore_lazyD0<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refD0.dataVector[pointi]->label<<endl;	
	}
	//cout<<"lazyD0done"<<endl;
	
	
	
	for( int update = 1; update<=no_update_batches; update++){
	 //cout<<"update "<<update<<"started"<<endl;
	 
		//read deltaD();
		
		data *deltaD = new data();
		data &refdeltaD = *deltaD;
		string deltaD_path = "../io_directory/deltaD_"+update_type+".csv";
		deltaD->createDataVector(deltaD_path);
		
		
		int deltaD_ss;
		int deltaD_numInstances = deltaD->getnumInstances();
		deltaD_ss = deltaD_numInstances * samplingFactor < minSampleSize ? minSampleSize : deltaD_numInstances * samplingFactor;
    	deltaD_ss = deltaD_numInstances < deltaD_ss ? deltaD_numInstances : deltaD_ss;
    	
    	
		//evaluate deltaD over F0
		//vector<double> AnomalyScore;
		AnomalyScore.clear();
		AnomalyScore.resize(0);
		for(int pointi =0; pointi < deltaD_numInstances;pointi++){
    		AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refdeltaD));
   		}
		//write lazy_AS_deltaD();
		ofstream outAnomalyScore_lazydeltaD("../io_directory/lazy_AS_deltaD.csv", ios::out|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
   		for(int pointi = 0; pointi < deltaD_numInstances; pointi++){
    		outAnomalyScore_lazydeltaD<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refdeltaD.dataVector[pointi]->label<<endl;	
		}
		//cout<<"lazydeltaDdone"<<endl;
		
	/*for(int pointi =0; pointi < deltaD_numInstances;pointi++){
    	AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refdeltaD));
   	}
   	*/
		
		
		//create F'_update();
		
		struct timespec start_i2f,end_i2f;
   		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_i2f);

		F0->constructINCiForest(deltaD_ss,refdeltaD);
		//cout<<"update "<<update<<"done"<<endl;
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_i2f);
		double i2fTime =  (((end_i2f.tv_sec - start_i2f.tv_sec) * 1e9)+(end_i2f.tv_nsec - start_i2f.tv_nsec))*1e-9;
    	cout<<i2fTime<<" ";
    	//cout<<i2fTime<<endl;;
		//write F'_update();
		
		//evlauate deltaD over F'1;
		//vector<double> AnomalyScore;
		AnomalyScore.clear();
		AnomalyScore.resize(0);
		for(int pointi =0; pointi < deltaD_numInstances;pointi++){
    		AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refdeltaD));
   		}
		
		//write inc_AS_deltaD();
		ofstream outAnomalyScore_incdeltaD("../io_directory/i2f_AS_deltaD.csv", ios::out|ios::binary);
		//ofstream outAnomalyScore("io_directory/inc_AS_deltaD"+to_string(update)+".csv", ios::app|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	for(int pointi = 0; pointi < deltaD_numInstances; pointi++){
    		outAnomalyScore_incdeltaD<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refdeltaD.dataVector[pointi]->label<<endl;	
		}
		//cout<<"incdeltaDdone"<<endl;
		
		//evlauate D0 over F'1;
		//vector<double> AnomalyScore;
		AnomalyScore.clear();
		AnomalyScore.resize(0);
		for(int pointi =0; pointi < D0_numInstances;pointi++){
    		AnomalyScore.push_back(F0->computeAnomalyScore(pointi, refD0));
   		}
		
		//write inc_AS_D0();
		ofstream outAnomalyScore_incD0("../io_directory/i2f_AS_Di.csv", ios::out|ios::binary);
		//ofstream outAnomalyScore("io_directory/inc_AS_deltaD"+to_string(update)+".csv", ios::app|ios::binary);
    	//outAnomalyScore<<"pointId "<<"anomalyscore "<<"actuallabel"<<endl;
    	for(int pointi = 0; pointi < D0_numInstances; pointi++){
    		outAnomalyScore_incD0<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refD0.dataVector[pointi]->label<<endl;	
		}
		//cout<<"incD0done"<<endl;
		delete deltaD;
		//cout<<"update "<<update<<"done"<<endl;
	
	}
	
}
