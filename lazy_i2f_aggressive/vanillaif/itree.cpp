#include "itree.h"
#include "treenode.cpp"
#include <math.h>
#include <queue>


//inner product of two vectors

itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes){}

itree::~itree(){}
//*************************************************STATIC iTree creation*******************************************************************//
void itree::constructiTree(){
    rootNode = new treenode(0);
    rootNode->dataPointIndices = _dataObject.getSample(_sampleSize);
    queue<treenode*> BFTforNodes;
    BFTforNodes.push(rootNode);
    while(!BFTforNodes.empty()){
    	treenode *currNode = BFTforNodes.front();
		BFTforNodes.pop();
		if(currNode){
			currNode->nodeSize = currNode->dataPointIndices.size();
			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
			if(currNode->nodeSize <=1 || currNode->nodeHeight ==_maxTreeHeight){
    			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
        		currNode->isLeaf = bool(1);
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
    		}
    		else{
    			currNode->splitValue = currNode->splitInfoSelection(_dataObject);
    			currNode->createLeftChild();
				currNode->createRightChild();
				for(int i=0; i<currNode->nodeSize; i++){     
            		if(_dataObject.dataVector[currNode->dataPointIndices[i]]->attributes[currNode->splitAttribute]<currNode->splitValue){
                		currNode->lChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
            		}
            		else{
                		currNode->rChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
            		}

        		}
        		
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
        		
        		BFTforNodes.push(currNode->lChildAdd);
        		BFTforNodes.push(currNode->rChildAdd);
    		}
    	}
    }
}
//*******************************************************Static EValuation*****************************************************************//
long double itree::computePathLength(int pointX, const data & testDataObject){
	long double pathLength = 0;
	treenode * node = rootNode;
	while(!node->isLeaf){
		pathLength++;
		if(testDataObject.dataVector[pointX]->attributes[node->splitAttribute] < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	pathLength += node->pathLengthEst;
	//cout<<pathLength<<" ";
	return pathLength;
}

long double itree::computeRelativeMass(int pointX, const data & testDataObject){
	long double currSize=0;
	long double parSize=0;

	treenode * node = rootNode;
	while(!node->isLeaf){
		parSize=node->nodeSize;

		if(testDataObject.dataVector[pointX]->attributes[node->splitAttribute] < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	
	currSize = node->nodeSize;
	if(currSize==0) return 1;
	long double relativeMass=parSize/(currSize*(long double)_sampleSize);
	// cout<<relativeMass<<endl;
	return relativeMass;
}

long double itree::pathLengthEstimationForUnbuiltTree(int nodeSize){
	long double es = 0;
	if(nodeSize == 2){es=1;}
	else if(nodeSize > 2){es = (2 * (log(nodeSize-1) + 0.5772156649)) - (2 * (nodeSize - 1) / nodeSize);}	
	return (es);
}


//***************************************************Incremental iTree creation************************************************************//

void itree::insertUpdatesIniTree(int updateSampleSize,data & refdeltaD){
	
	
	
	//rootNode->updateIndices = _dataObject.getSample(updateSampleSize);
    rootNode->updateIndices = refdeltaD.getSample(updateSampleSize);
    //cout<<"rootNode->updateIndices.size()"<<rootNode->updateIndices.size()<<endl;
	queue<treenode*> BFTforNodes;
	BFTforNodes.push(rootNode);
	while(!BFTforNodes.empty()){ 	
    	treenode *currNode = BFTforNodes.front();
    	//cout<<"currNode->nodeId="<<currNode->nodeId<<endl;
		
		if(currNode == rootNode){currNode->nodeHeight=0;}
		else{currNode->nodeHeight=currNode->parentAdd->nodeHeight +1;}
			
    	BFTforNodes.pop();
    	//cout<<"isBFT empty"<<BFTforNodes.empty()<<endl;
		if(currNode->updateIndices.size()<1){						//*CASE 1:nodes that are not affected by insert*/

			//cout<<"nodes updatepoints=0:currNode->nodeId="<<currNode->nodeId<<endl;
			continue;
		}            	
		currNode->nodeSize +=currNode->updateIndices.size();
		currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
		if(currNode->nodeHeight == _maxTreeHeight){             //*CASE 2:useless nodes beyond this height as per def. of iForest*/
			//cout<<" max height node:currNode->nodeId="<<currNode->nodeId<<"at height"<<currNode->nodeHeight<<endl;
			
			if(!currNode->isLeaf){
				deleteSubTree(currNode->lChildAdd);
				currNode->lChildAdd = nullptr;
				deleteSubTree(currNode->rChildAdd);
				currNode->rChildAdd = nullptr;
				currNode->isLeaf = bool(1);
				currNode->splitAttribute = -1;
				currNode->lChildId = -1;
				currNode->rChildId = -1;
				currNode->updateIndices.clear();
				currNode->updateIndices.resize(0);
			}			
			continue;
		}  
		if(currNode->isLeaf /*&& currNode->nodeHeight < _maxTreeHeight*/){          /*CASE 3:leaf nodes below max height*/
    		//cout<<"Leaf:currNode->nodeId="<<currNode->nodeId<<endl;
			if(currNode->updateIndices.size()>1){                                   /*CASE 3a:leaf needs split*/
				//cout<<"leaf splits"<<endl;
				currNode->splitValue = currNode->splitInfoSelectionForUpdates(refdeltaD);
    			currNode->createLeftChild();
    			currNode->lChildAdd->isLeaf = bool(1);
				currNode->createRightChild();
				currNode->rChildAdd->isLeaf = bool(1);
				currNode->isLeaf = bool(0);
				for(int i=0; i<currNode->updateIndices.size(); i++){     
           		 	if(_dataObject.dataVector[currNode->updateIndices[i]]->attributes[currNode->splitAttribute]<currNode->splitValue){
           	    		currNode->lChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 	}
            		else{
           	    		currNode->rChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 	}
       			}
				
       			BFTforNodes.push(currNode->lChildAdd);
        		BFTforNodes.push(currNode->rChildAdd);
			}
			currNode->updateIndices.clear();
			currNode->updateIndices.resize(0);
			
		}
		else{                                           //*CASE 4: Nonleaf nodes*/
			//cout<<"Non-leaf:currNode->nodeId="<<currNode->nodeId<<endl;
			//cout<<"currNode->updateIndices.size()"<<currNode->updateIndices.size()<<endl;
			currNode->lChildAdd->updateIndices.clear();
			currNode->lChildAdd->updateIndices.resize(0);
			currNode->rChildAdd->updateIndices.clear();
			currNode->rChildAdd->updateIndices.resize(0);
			
			for(int i=0; i<currNode->updateIndices.size(); i++){ 
				//cout<<  "currNode->updateIndices["<<i<<"]"<<currNode->updateIndices[i]<<endl;  
				
				if(_dataObject.dataVector[currNode->updateIndices[i]]->attributes[currNode->splitAttribute]<currNode->splitValue){
       	    		//cout<<"lChildAdd->updateIndices.size()-before"<<currNode->lChildAdd->updateIndices.size();	
					currNode->lChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
       		 		//cout<<"-after"<<currNode->lChildAdd->updateIndices.size()<<endl;
					
				}
				else{
					//cout<<"rChildAdd->updateIndices.size()-before"<<currNode->rChildAdd->updateIndices.size();	
					currNode->rChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
					//cout<<"-after"<<currNode->rChildAdd->updateIndices.size()<<endl;
					
				}
       		}
			//cout<<"Non-leaf:currNode->nodeId="<<currNode->nodeId<<endl;
			
			// currNode->newMinMaxSelection(refdeltaD, &min, &max);
			double min, max;
			currNode->newMinMaxSelection(_dataObject, &min, &max);
			currNode->updateIndices.clear();
			currNode->updateIndices.resize(0);
			
			if(min >= currNode->minimumVal && max <= currNode->maximumVal){               //**************CASE 4a: min-max range unchanged*****************************************/
				//cout<<"Non-leaf-minmax uncahnged"<<endl;
				
				BFTforNodes.push(currNode->lChildAdd);
				BFTforNodes.push(currNode->rChildAdd);
				continue;
			}
			
			if(currNode->minimumVal>min){                    //**************************CASE 4b: min changed- left child needs split**************************************************/
				if(currNode->lChildAdd->isLeaf){
					BFTforNodes.push(currNode->lChildAdd);
				}
				else{
					treenode *currLC = currNode->lChildAdd;
					treenode *nodeP = new treenode(currLC->nodeId);
					treenode *nodeLC = new treenode(nodeP->nodeId*2 +1);
					nodeP->parentAdd = currNode;
					nodeP->updateIndices = currLC->updateIndices;
					nodeP->nodeSize = nodeP->updateIndices.size() + currLC->nodeSize;
					nodeP->pathLengthEst = pathLengthEstimationForUnbuiltTree(nodeP->nodeSize);
					nodeP->splitAttribute = currNode->splitAttribute;
					nodeP->splitValue = currNode->minimumVal;
					nodeP->minimumVal = min;
					nodeP->maximumVal = currLC->maximumVal;
					currNode->minimumVal = min;
					nodeLC->isLeaf = bool(1);
					nodeLC->parentAdd = nodeP;
					nodeP->lChildAdd = nodeLC;
					nodeP->lChildId = nodeLC->nodeId;
					nodeP->rChildAdd = currLC;
					nodeP->rChildId = nodeLC->nodeId +1;
					currLC->nodeId = nodeP->rChildId;
					currLC->rChildId = currLC->nodeId *2 +2;
					currLC->lChildId = currLC->nodeId *2 +1;
					currLC->parentId = nodeP->nodeId;
					currLC->parentAdd = nodeP;
					currLC->nodeHeight = (int)log2(currLC->nodeId+1)+1;
					currLC->updateIndices.resize(0);
					currNode->lChildAdd = nodeP;
					
					for(int i=0; i<nodeP->updateIndices.size(); i++){     
           		 		if(_dataObject.dataVector[nodeP->updateIndices[i]]->attributes[nodeP->splitAttribute]<nodeP->splitValue){
           	    			nodeP->lChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 		}
            			else{
           	    			nodeP->rChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 		}
       				}
					BFTforNodes.push(nodeLC);
					BFTforNodes.push(nodeP->rChildAdd);
				}
			}else{BFTforNodes.push(currNode->lChildAdd);}
			
			
			
			
			if(currNode->maximumVal<max){                            //**************************CASE 4b: min changed- right child needs split**************************************************/
				if(currNode->rChildAdd->isLeaf){
					BFTforNodes.push(currNode->rChildAdd);
				}
				else{
					treenode *currRC = currNode->rChildAdd;
					treenode *nodeP = new treenode(currRC->nodeId);
					treenode *nodeRC = new treenode(nodeP->nodeId*2 +2);
					nodeP->parentAdd = currNode;
					nodeP->updateIndices = currNode->rChildAdd->updateIndices;
					nodeP->nodeSize = nodeP->updateIndices.size() + currRC->nodeSize;
					nodeP->pathLengthEst = pathLengthEstimationForUnbuiltTree(nodeP->nodeSize);
					nodeP->splitAttribute = currNode->splitAttribute;
					nodeP->splitValue = currNode->maximumVal;
					nodeP->minimumVal = currRC->minimumVal;
					nodeP->maximumVal = max;
					currNode->maximumVal = max;
					nodeRC->isLeaf = bool(1);
					nodeRC->parentAdd = nodeP;
					nodeP->rChildAdd = nodeRC;
					nodeP->rChildId = nodeRC->nodeId;
					nodeP->lChildAdd = (treenode*)currRC;
					nodeP->lChildId = nodeRC->nodeId -1;
					currRC->nodeId = nodeP->lChildId;
					currRC->rChildId = currRC->nodeId *2 +2;
					currRC->lChildId = currRC->nodeId *2 +1;
					currRC->parentId = nodeP->nodeId;
					currRC->parentAdd = nodeP;
					currRC->nodeHeight = (int)log2(currRC->nodeId+1)+1;
					currRC->updateIndices.resize(0);
					currNode->rChildAdd = nodeP;
					
					for(int i=0; i<nodeP->updateIndices.size(); i++){     
           		 		if(_dataObject.dataVector[nodeP->updateIndices[i]]->attributes[nodeP->splitAttribute]<nodeP->splitValue){
           	    			nodeP->lChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 		}
            			else{
           	    			nodeP->rChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
           		 		}
       				}
					BFTforNodes.push(nodeP->lChildAdd);
					BFTforNodes.push(nodeRC);
				}
			}else{BFTforNodes.push(currNode->rChildAdd);}
			
				
		}
		currNode->updateIndices.clear();
		currNode->updateIndices.resize(0);
				
    }
}



void itree::deleteSubTree(treenode * node){
    if(node->isLeaf){
        delete node;
    }
    else{
        treenode *left = node->lChildAdd;
        treenode *right = node->rChildAdd;
        delete node;
        deleteSubTree(left);
        deleteSubTree(right);
    }
}

























