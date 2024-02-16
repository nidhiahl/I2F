#include "iforest.h"
#include "itree.cpp"
#include <queue>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <math.h>
using namespace std;

iforest::iforest(const data & dataObject): _dataObject(dataObject){}

iforest::iforest(const data & dataObject, int numiTrees, int sampleSize, int exLevel):_dataObject(dataObject), _numiTrees(numiTrees), _sampleSize(sampleSize), _exLevel(exLevel){
	_maxTreeHeight = (int)log2(_sampleSize);
	_maxNumOfNodes = (int)pow(2.0,_maxTreeHeight+1)-1;
	_avgPLEstimationOfBST = avgPathLengthEstimationOfBST(_sampleSize);
	//cout<<"_avgPLEstimationOfBST============="<<_avgPLEstimationOfBST<<endl;
	_iTrees.resize(_numiTrees);
	//anomalyScore.resize(_dataObject.getnumInstances());
	
	
}

iforest::~iforest(){}

//***************************************************STATIC iForest creation***************************************************************//
void iforest::constructiForest(){
	std::random_device random_seed_generator;

    for(int treeId = 0; treeId < _numiTrees; treeId++){
		//cout<<"treeId===="<<treeId<<endl;
		_iTrees[treeId] = new itree(_dataObject, _sampleSize, _maxTreeHeight, _maxNumOfNodes, _avgPLEstimationOfBST, _exLevel);
		_iTrees[treeId]->constructiTree(random_seed_generator());
	}
	//_avgPLComputationOfBST = avgPathLengthComputationOfBST();
}



//***************************************************INCREMENTAL iForest creation**********************************************************//
void iforest::constructINCiForest(int updateSampleSize,data &refdeltaD){
	std::random_device random_seed_generator;

	_sampleSize += updateSampleSize; 
	_avgPLEstimationOfBST = avgPathLengthEstimationOfBST(_sampleSize);
	for(int treeId = 0; treeId < _numiTrees; treeId++){
	 //cout<<"treeId========================================================="<<treeId<<endl;
		//cout<<"updateSampleSize="<<updateSampleSize<<endl;
		_iTrees[treeId]->insertUpdatesIniTree(updateSampleSize, random_seed_generator(),refdeltaD);
	}
	//_avgPLComputationOfBST = avgPathLengthComputationOfBST();
		
	
}


//***************************************************Write iforest to file*************************************************************//
void iforest::writeFOREST(const string & FORESTFile){
    //cout<<"/**************SAVE forest******************///////"<<endl;
    string treenodeFile = FORESTFile + "_TREENODES";
    ofstream storeNodes(treenodeFile.c_str(),ios::out|ios::binary);
    if(!storeNodes){
        cout<<"Cannot open the output file "<<FORESTFile<<endl;
        exit(0);
    }
    boost::archive::binary_oarchive storeNodesArchive(storeNodes);
    for(int treeId = 0; treeId < _numiTrees; treeId++){
        queue<treenode*> BFTforNodes;
		BFTforNodes.push(_iTrees[treeId]->rootNode);
		while(!BFTforNodes.empty()){
    		treenode *currNode = BFTforNodes.front();
    		BFTforNodes.pop();
	        if(currNode->lChildAdd != nullptr){
				BFTforNodes.push(currNode->lChildAdd);
			}
			if(currNode->rChildAdd != nullptr){
				BFTforNodes.push(currNode->rChildAdd);
			}
            storeNodesArchive << *(currNode);
		}        
    }
    storeNodes.close();
    string iForestFile = FORESTFile + "_iFOREST";
    ofstream storeForest(iForestFile.c_str(),ios::out|ios::binary);
    if(!storeForest){
            cout<<"Cannot open the output file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_oarchive storeForestArchive(storeForest);
    storeForestArchive << *(this);
    storeForest.close();
}


//***************************************************read iforest from file**************************************************************//
void iforest::readFOREST(const string & FORESTFile){
    //cout<<"/**********************read forest**************/"<<endl;
    string iTreesFile = FORESTFile + "_iFOREST";
    ifstream readForest(iTreesFile.c_str(),ios::in|ios::binary);
    if(!readForest){
            cout<<"Cannot open the input file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_iarchive readForestArchive(readForest);
    readForestArchive >> *(this);
    readForest.close();
    string treenodeFile = FORESTFile + "_TREENODES";
    ifstream readNodes(treenodeFile.c_str(),ios::in|ios::binary);
    if(!readNodes){
            cout<<"Cannot open the input file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_iarchive readNodesArchive(readNodes);
    _iTrees.resize(_numiTrees);
    for(int treeId = 0; treeId < _numiTrees; treeId++){
    	_iTrees[treeId] = new itree(_dataObject,_sampleSize,_maxTreeHeight,_maxNumOfNodes,_avgPLEstimationOfBST, _exLevel);
    	
    	treenode *rootNode = new treenode();
    	readNodesArchive >> *(rootNode);
    	_iTrees[treeId]->rootNode = rootNode;
    	queue<treenode*> BFTforNodes;
		BFTforNodes.push(rootNode);
    	while(!BFTforNodes.empty()){
    		treenode *currNode = BFTforNodes.front();
    		BFTforNodes.pop();
	    	if(currNode->lChildId > 0){
	    		treenode *lChild = new treenode();
    			readNodesArchive >> *(lChild);
    			currNode->lChildAdd = lChild;
    			lChild->parentAdd = currNode;
    			BFTforNodes.push(lChild);    	
	    	}
	    	if(currNode->rChildId > 0){
	    		treenode *rChild = new treenode();
    			readNodesArchive >> *(rChild);
    			currNode->rChildAdd = rChild;
    			rChild->parentAdd = currNode;
    			BFTforNodes.push(rChild);    	
	    	}
	    }
    }
    readNodes.close();
}

//***************************************************STATIC iForest Evaluation*************************************************************//
long double iforest::computeAnomalyScore(int pointX, const data & testDataObject){
	long double avgPathLength = computeAvgPathLength(pointX, testDataObject);
	//long double AscoreComputed = pow(2,-(avgPathLength/_avgPLComputationOfBST));   
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<avgPathLength<<endl;
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<_avgPLEstimationOfBST<<endl;
	
	// long double AscoreEstimated = avgPathLength;
	 long double AscoreEstimated = pow(2,-(avgPathLength/_avgPLEstimationOfBST));
	//long double AscoreEstimated = computeRelativeMass(pointX, testDataObject);

	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<AscoreEstimated<<endl;
	//anomalyScore.push_back(AscoreEstimated);
	//cout<<"AscoreEstimated("<<pointX<<")="<<AscoreEstimated<<endl;
	//cout<<pointX<<" "<<AscoreEstimated<<" "<<testDataObject.dataVector[pointX]->label<<endl;
	return AscoreEstimated;
}

long double iforest::avgPathLengthEstimationOfBST(int size){
	long double avgPathLengthEstimationOfBST = 0;
	if(size == 2){avgPathLengthEstimationOfBST=1;}
	else if(size > 2){avgPathLengthEstimationOfBST = (2 * (log(size-1) + 0.5772156649)) - (2 * (size - 1) / size);}	
	return (avgPathLengthEstimationOfBST);
}



long double iforest::computeAvgPathLength(int pointX, const data & testDataObject){
	long double avgPathLength = 0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		long double pl = _iTrees[treeId]->computePathLength(pointX, testDataObject);
		avgPathLength += pl;
		//cout<<pl<<" ";
	}
	avgPathLength /=_numiTrees;
	//cout<<"iForestObject->computeAnomalyScore("<<pointX<<",refTestDataObject)="<<avgPathLength<<endl;
	//cout<<avgPathLength<<endl;
	
	return avgPathLength;
}


long double iforest::computeRelativeMass(int pointX, const data & testDataObject)
{
	long double avgRelativeMass = 0.0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		avgRelativeMass += _iTrees[treeId]->computeRelativeMass(pointX, testDataObject);
	}
	// cout<<AscoreComputed<<endl;
	avgRelativeMass /=_numiTrees;
	return avgRelativeMass;
}


/******************************Improved ascore computation*********************************************************/

/*
long double iforest::computePrunedAnomalyScore(int pointX, const data & testDataObject){
	long double avgPathLength = computeAvgPathLength(pointX, testDataObject);
	//long double AscoreComputed = pow(2,-(avgPathLength/_avgPLComputationOfBST));   
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<avgPathLength<<endl;
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<_avgPLEstimationOfBST<<endl;
	
	// long double AscoreEstimated = avgPathLength;
	 long double AscoreEstimated = pow(2,-(avgPathLength/_avgPLEstimationOfBST));
	//long double AscoreEstimated = computeRelativeMass(pointX, testDataObject);

	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<AscoreEstimated<<endl;
	//anomalyScore.push_back(AscoreEstimated);
	return AscoreEstimated;
}
*/







