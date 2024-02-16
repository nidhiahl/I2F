#include "data.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>


data::data()
{
    //ctor
}

data::~data()
{
    //dtor
}



/**************Getter methods*****************/


void data::setnumInstances(int numInstances){
    this->numInstances = numInstances;
}

void data::setnumAttributes(int numAttributes){
    this->numAttributes = numAttributes;
}

/**************Setter methods*****************/


int data::getnumInstances()const{
    return this->numInstances;
}

int data::getnumAttributes() const {
    return this->numAttributes;
}

vector<point*> data::getDataVector() const {
    return dataVector;
}


/*******takes complete dataset File as input************/

void data::createDataVector(const string & dataFile){
	ifstream inDataFile(dataFile.c_str(),ios::in|ios::binary) ;
	if(!inDataFile){
		cout<<"Can not open input data file: "<<dataFile<<endl;
		exit(0);
	}
	inDataFile>>numInstances>>numAttributes;
	//cout<<"numInstances="<<numInstances<<" numAttributes="<<numAttributes<<endl;
	
	//dataVector.resize(numInstances);
	for(int instance = 0; instance < numInstances; instance++){
		point *newPoint = new point();
		newPoint->attributes = new double[numAttributes];
		int id;
		//inDataFile>>id;
		for(int attribute = 0; attribute < numAttributes; attribute++){
			inDataFile>>newPoint->attributes[attribute];
			//cout<<newPoint->attributes[attribute]<<" ";
		}
		
		
		inDataFile>>newPoint->label;
		//cout<<newPoint->label;
		//cout<<endl;
		//dataVector[instance] = newPoint;
		dataVector.push_back(newPoint);
		
	}
	//cout<<"dataVector.size()"<<dataVector.size()<<endl;
	inDataFile.close();
}
/*************************************read added data into the already available data vector*****************************************/
void data::incrementDataVector(const string & updateFile){
	ifstream inUpdateFile(updateFile.c_str(),ios::in|ios::binary);
	if(!inUpdateFile){
		cout<<"Can not open input update file:"<<updateFile<<endl;
		exit(0);
	}
	int num_updatePoints;
	inUpdateFile>>num_updatePoints>>numAttributes;
	numInstances+=num_updatePoints;
	//cout<<"numInstances="<<numInstances<<" numAttributes="<<numAttributes<<endl;
	for(int instance = 0; instance < num_updatePoints; instance++){
		point *newPoint = new point();
		newPoint->attributes = new double[numAttributes];
		int id;
		//inUpdateFile>>id;
		for(int attribute = 0; attribute < numAttributes; attribute++){
			inUpdateFile>>newPoint->attributes[attribute];
		}
		
		inUpdateFile>>newPoint->label;
		dataVector.push_back(newPoint);
	}
	inUpdateFile.close();
}




//***************************************Select sample from the original dataset i.e for static run****************************************//
vector<int> & data::getSample(int sampleSize) const {
	vector<int>* sample = new vector<int>;            
	vector<int> &refSample = *sample;
	int cnt = 0;
	for(int i = 0; i < numInstances; i++){
		(*sample).push_back(i);
		cnt++;
	}
	std::random_shuffle((*sample).begin(),(*sample).end());
	(*sample).erase((*sample).begin()+sampleSize,(*sample).begin()+cnt);
	return refSample;
}
