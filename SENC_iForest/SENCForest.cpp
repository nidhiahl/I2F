#include "SENCForest.h"
#include <queue>
/*#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>*/
#include <math.h>
using namespace std;

//SENCForest::SENCForest(){}

SENCForest::SENCForest(const data & dataObject): _dataObject(dataObject){}

SENCForest::SENCForest(const data & dataObject, int numSENCTrees, int sampleSize, int minSizeOfNode):_dataObject(dataObject), _numSENCTrees(numSENCTrees), _sampleSize(sampleSize), _minSizeOfNode(minSizeOfNode){
	_maxTreeHeight = (int)log2(_sampleSize);
	_maxNumOfNodes = (int)pow(2.0,_maxTreeHeight+1)-1;
	_avgPLEstimationOfBST = avgPathLengthEstimationOfBST(_sampleSize);
	//cout<<"_avgPLEstimationOfBST============="<<_avgPLEstimationOfBST<<endl;
	_SENCTrees.resize(_numSENCTrees);
	//anomalyScore.resize(_dataObject.getnumInstances());
	
	
}

SENCForest::~SENCForest(){}

//***************************************************STATIC SENCForest creation***************************************************************//
void SENCForest::constructSENCForest(int numberOfClasses){
    for(int treeId = 0; treeId < _numSENCTrees; treeId++){
		// cout<<"treeId===="<<treeId<<endl;
		_SENCTrees[treeId] = new SENCTree(_dataObject, _sampleSize, _maxTreeHeight, _maxNumOfNodes, _avgPLEstimationOfBST, _minSizeOfNode);
		_SENCTrees[treeId]->constructSENCTree(numberOfClasses);
		_SENCTrees[treeId]->anomalyHeight = _SENCTrees[treeId]->findAnomalyRegion();
		// _SENCTrees[treeId]->createBall();
	}

	_numberOfClasses = numberOfClasses;
	//_avgPLComputationOfBST = avgPathLengthComputationOfBST();
}



//***************************************************INCREMENTAL SENCForest creation**********************************************************//
void SENCForest::deploySENCForest(int sampleSize)
{
	vector<pair<int, vector<treenode *>>> newClass;
	int i;
	for( i=0; i<_dataObject.dataVector.size(); i++)
	{
		vector<int> fre(_numberOfClasses+1, 0);
		vector<treenode *> nodes;
		for(int treeId = 0; treeId < _numSENCTrees; treeId++)
		{
			fre[_SENCTrees[treeId]->getLabel(i, nodes)]++;
		}

		int label=0;
		for(int j=0;j<fre.size();j++)
		{
			if(fre[j]>fre[label]) label=j;
		}
		//cout<<label<<endl;
		if(label==_numberOfClasses)
		{
			newClass.push_back({i, nodes});
		}
	}
	cout<<"i="<<i<<endl;

	constructINCSENCForest(newClass, sampleSize);
}

void SENCForest::constructINCSENCForest(vector<pair<int, vector<treenode *>>>& newClass, int sampleSize){
		sampleSize = min(sampleSize, (int)newClass.size());
		_sampleSize += sampleSize; 
		_avgPLEstimationOfBST = avgPathLengthEstimationOfBST(_sampleSize);
		
		for(int treeId = 0; treeId < _numSENCTrees; treeId++){
			// cout<<treeId<<endl;
			_SENCTrees[treeId]->insertUpdatesInSENCTree(newClass, sampleSize, treeId);
		}
}


//***************************************************Write SENCForest to file*************************************************************//
/*void SENCForest::writeFOREST(const string & FORESTFile){
    //cout<<"/**************SAVE forest******************"<<endl;
    string treenodeFile = FORESTFile + "_TREENODES";
    ofstream storeNodes(treenodeFile.c_str(),ios::out|ios::binary);
    if(!storeNodes){
        cout<<"Cannot open the output file "<<FORESTFile<<endl;
        exit(0);
    }
    boost::archive::binary_oarchive storeNodesArchive(storeNodes);
    for(int treeId = 0; treeId < _numSENCTrees; treeId++){
        queue<treenode*> BFTforNodes;
		BFTforNodes.push(_SENCTrees[treeId]->rootNode);
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
    string SENCForestFile = FORESTFile + "_SENCForest";
    ofstream storeForest(SENCForestFile.c_str(),ios::out|ios::binary);
    if(!storeForest){
            cout<<"Cannot open the output file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_oarchive storeForestArchive(storeForest);
    storeForestArchive << *(this);
    storeForest.close();
}*/


//***************************************************read SENCForest from file**************************************************************//
/*void SENCForest::readFOREST(const string & FORESTFile){
    //cout<<"/**********************read forest**************"<<endl;
    string SENCTreesFile = FORESTFile + "_SENCForest";
    ifstream readForest(SENCTreesFile.c_str(),ios::in|ios::binary);
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
    _SENCTrees.resize(_numSENCTrees);
    for(int treeId = 0; treeId < _numSENCTrees; treeId++){
    	_SENCTrees[treeId] = new SENCTree(_dataObject,_sampleSize,_maxTreeHeight,_maxNumOfNodes,_avgPLEstimationOfBST, _minSizeOfNode);
    	
    	treenode *rootNode = new treenode();
    	readNodesArchive >> *(rootNode);
    	_SENCTrees[treeId]->rootNode = rootNode;
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
}*/

//***************************************************STATIC SENCForest Evaluation*************************************************************//
long double SENCForest::computeAnomalyScore(int pointX, const data & testDataObject){
	long double avgPathLength = computeAvgPathLength(pointX, testDataObject);
	//long double AscoreComputed = pow(2,-(avgPathLength/_avgPLComputationOfBST));   
	//cout<<"SENCForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<avgPathLength<<endl;
	//cout<<"SENCForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<_avgPLEstimationOfBST<<endl;
	
	// long double AscoreEstimated = avgPathLength;
	long double AscoreEstimated = pow(2,-(avgPathLength/_avgPLEstimationOfBST));

	// long double AscoreEstimated = computeRelativeMass(pointX, testDataObject);

	//cout<<"SENCForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<AscoreEstimated<<endl;
	//anomalyScore.push_back(AscoreEstimated);

	
	return AscoreEstimated;
}

long double SENCForest::avgPathLengthEstimationOfBST(int size){
	long double avgPathLengthEstimationOfBST = 0;
	if(size == 2){avgPathLengthEstimationOfBST=1;}
	else if(size > 2){avgPathLengthEstimationOfBST = (2 * (log(size-1) + 0.5772156649)) - (2 * (size - 1) / size);}	
	return (avgPathLengthEstimationOfBST);
}



long double SENCForest::computeAvgPathLength(int pointX, const data & testDataObject){
	long double avgPathLength = 0;
	for(int treeId = 0; treeId < _numSENCTrees; treeId++){
		avgPathLength += _SENCTrees[treeId]->computePathLength(pointX, testDataObject);
	}
	avgPathLength /=_numSENCTrees;
	//cout<<"SENCForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<avgPathLength<<endl;
	
	return avgPathLength;
}

long double SENCForest::computeRelativeMass(int pointX, const data & testDataObject)
{
	long double avgRelativeMass = 0.0;
	for(int treeId = 0; treeId < _numSENCTrees; treeId++){
		avgRelativeMass += _SENCTrees[treeId]->computeRelativeMass(pointX, testDataObject);
	}
	// cout<<AscoreComputed<<endl;
	avgRelativeMass /=_numSENCTrees;
	return avgRelativeMass;
}

















