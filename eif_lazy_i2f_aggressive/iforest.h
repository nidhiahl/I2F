#ifndef IFOREST_H
#define IFOREST_H
#include "data.h"
#include "itree.h"
#include <random>

class iforest
{
    public:
	//iforest();
    iforest(const data &);                              //used for: create an empty forest followed by reading the history in blank forest
	iforest(const data &, int, int, int);                    //used for: create a forest and initialize _sampleSize and _numiTrees
    virtual ~iforest();
    void constructiForest();    						//used for creating an iForest (static vesion) 
    void constructINCiForest(int,data &);      				//used for: create an i^2Forest (incremental iforest version of iForest)
    void writeFOREST(const string &);					//writes history in file.
    void readFOREST(const string &);        			//reads history from file.
	long double computeAnomalyScore(int, const data &);		        //compute anomaly score of a point passed as an int argument
	long double computeRelativeMass(int, const data &);

	private:    
	long double computeAvgPathLength(int, const data &);              //compute average path length of point passed as an argumnet i.e Averaged across all the iTrees
    long double avgPathLengthEstimationOfBST(int);      //compute the average path length estimation of a 'n' sized BST, 'n' is passed as an argument
    
	public:
    //vector<double> anomalyScore;						//contains the computed anomaly score of all the points, evaluated using the iForest in hand 
    
    private:
    int _numInstances;
    int _numiTrees;										//number of iTrees in the iForest.
	int _sampleSize;									//sample size representing the iForest.
    int _maxTreeHeight;									//max Height of each iTree in iForest.
  	int _maxNumOfNodes;									//max number of node possible in each iTree.
	int _avgPLEstimationOfBST;							//average path length estimation of BST having '_sampleSize' number of nodes. 
	int _exLevel;
    const data & _dataObject;							//reference of the input dataObject, only a container not responsible for deletion of the object.
	vector<itree*> _iTrees;								//list of pointers to the iTrees in the forest.
	    
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
		//ar & BOOST_SERIALIZATION_NVP(anomalyScore);
        ar & _sampleSize;
        ar & _maxTreeHeight;
  		ar & _maxNumOfNodes;
		ar & _numiTrees;
		ar & _avgPLEstimationOfBST;
        ar & _exLevel;
	}
};

#endif // IFOREST_H
