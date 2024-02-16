#ifndef SENCForest_H
#define SENCForest_H
//#include "data.cpp"
#include "SENCTree.cpp"

class SENCForest
{
    public:
	//SENCForest();
    SENCForest(const data &);                              //used for: create an empty forest followed by reading the history in blank forest
	SENCForest(const data &, int, int, int);                    //used for: create a forest and initialize _sampleSize and _numSENCTrees
    virtual ~SENCForest();
    void constructSENCForest(int);    						//used for creating an SENCForest (static vesion) 
    void constructINCSENCForest(vector<pair<int, vector<treenode *>>> &, int);      				//used for: create an i^2Forest (incremental SENCForest version of SENCForest)
    void writeFOREST(const string &);					//writes history in file.
    void readFOREST(const string &);        			//reads history from file.
	long double computeAnomalyScore(int, const data &);		        //compute anomaly score of a point passed as an int argument
	long double computeRelativeMass(int, const data &);		        //compute anomaly score of a point passed as an int argument
	void deploySENCForest(int);

	private:    
	long double computeAvgPathLength(int, const data &);              //compute average path length of point passed as an argumnet i.e Averaged across all the SENCTrees
    long double avgPathLengthEstimationOfBST(int);      //compute the average path length estimation of a 'n' sized BST, 'n' is passed as an argument
    
	public:
    //vector<double> anomalyScore;						//contains the computed anomaly score of all the points, evaluated using the SENCForest in hand 
    
    private:
    int _numSENCTrees;										//number of SENCTrees in the SENCForest.
	int _sampleSize;									//sample size representing the SENCForest.
    int _maxTreeHeight;									//max Height of each SENCTree in SENCForest.
  	int _maxNumOfNodes;									//max number of node possible in each SENCTree.
	int _avgPLEstimationOfBST;							//average path length estimation of BST having '_sampleSize' number of nodes. 
	const data & _dataObject;							//reference of the input dataObject, only a container not responsible for deletion of the object.
	vector<SENCTree*> _SENCTrees;								//list of pointers to the SENCTrees in the forest.
    int _minSizeOfNode;
    int _numberOfClasses;
	
    
    /*friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
		//ar & BOOST_SERIALIZATION_NVP(anomalyScore);
        ar & _sampleSize;
        ar & _maxTreeHeight;
  		ar & _maxNumOfNodes;
		ar & _numSENCTrees;
		ar & _avgPLEstimationOfBST;
        ar & _minSizeOfNode;
        ar & _numberOfClasses;
	}*/
};

#endif // SENCForest_H
