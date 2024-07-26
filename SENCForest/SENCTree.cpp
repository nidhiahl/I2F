#include "SENCTree.h"
#include <math.h>
#include <queue>
#include <random>
#include <unordered_set>
#include <climits>
//Standard Deviation Calculator
class StdDevCalc{
    private:

        double count;
        double meanPrev, meanCurr;
        double sPrev, sCurr;
        double varianceCurr;

    public:

        StdDevCalc() {
            count = 0;
        }

        void append(double d) {
            count++;

            if (count == 1) {
                // Set the very first values.
                meanCurr     = d;
                sCurr        = 0;
                varianceCurr = sCurr;
            }
            else {
                // Save the previous values.
                meanPrev     = meanCurr;
                sPrev        = sCurr;

                // Update the current values.
                meanCurr     = meanPrev + (d - meanPrev) / count;
                sCurr        = sPrev    + (d - meanPrev) * (d - meanCurr);
                varianceCurr = sCurr / (count - 1);
            }
        }

    double get_std_dev() {
        return sqrt(varianceCurr);
    }
};

//get K random samples out of N without replacement.
inline std::vector<int> sample_without_replacement(int k, int N)
{
    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    std::unordered_set<int> samples;

    for(int i=N-k+1; i<N+1; i++)
    {
        int v=std::uniform_int_distribution<>(1, i)(RandomEngine);
        if(!samples.insert(v).second) samples.insert(i);
    }

    std::vector<int> result(samples.begin(), samples.end());
    std::shuffle(result.begin(), result.end(), RandomEngine);

    return result;    
}

long double getRadius(double* curr, vector<long double>& centroid)
{
	long double radius=0.0;
	for(int i=0;i<centroid.size();i++)
	{
		radius+=(curr[i] - centroid[i])*(curr[i] - centroid[i]);
	}

	return sqrt(radius);
}

long double getRadius2(vector<long double>& curr, vector<long double>& centroid)
{
	long double radius=0.0;
	for(int i=0;i<centroid.size();i++)
	{
		radius+=(curr[i] - centroid[i])*(curr[i] - centroid[i]);
	}

	return sqrt(radius);
}

SENCTree::SENCTree( data & dataObject): _dataObject(dataObject){}

SENCTree::SENCTree( data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation, int minSizeOfNode): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes), _minSizeOfNode(minSizeOfNode){}

SENCTree::~SENCTree(){}
//*************************************************STATIC SENCTree creation*******************************************************************//
void SENCTree::constructSENCTree(int numberOfClasses){
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

			if(currNode->nodeSize <= _minSizeOfNode || currNode->nodeHeight==30){
				// cout<<currNode->nodeId<<endl;
				currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
        		currNode->isLeaf = bool(1);

				//Creating Ball
				if(currNode->centroid.size()==0)
					currNode->centroid.resize(_dataObject.getnumAttributes(), 0.0);

				for(int i=0; i<currNode->dataPointIndices.size(); i++)
				{
					for(int j=0;j<_dataObject.getnumAttributes();j++)
					{
						currNode->centroid[j]+=(_dataObject.dataVector[currNode->dataPointIndices[i]]->attributes[j] - currNode->centroid[j])/(i+1);
					}
				}
				for(int i=0;i<currNode->dataPointIndices.size();i++)
				{
					currNode->radius=max(currNode->radius, getRadius(_dataObject.dataVector[currNode->dataPointIndices[i]]->attributes, currNode->centroid));
				}

				//Maintain frequency
				currNode->fre.resize(numberOfClasses, 0);
				for(int i=0; i<currNode->dataPointIndices.size(); i++)
				{
					currNode->fre[_dataObject.dataVector[currNode->dataPointIndices[i]]->label]++;
					if(currNode->fre[_dataObject.dataVector[currNode->dataPointIndices[i]]->label] > currNode->fre[currNode->label])
					{
						currNode->label=_dataObject.dataVector[currNode->dataPointIndices[i]]->label;
					}
				}

        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
				leafNodes.push_back(currNode);
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

int SENCTree::findAnomalyRegion()
{
	vector<int> heights;
	for(int i=0; i<leafNodes.size(); i++)
	{
		heights.push_back(leafNodes[i]->nodeHeight);
	}

	// sort(heights.begin(), heights.end());

	vector<long double> stddev_pass1;
	StdDevCalc pass1;
	for(int i=0; i<heights.size(); i++)
	{
		while(i+1<heights.size() && heights[i]==heights[i+1])
		{
			pass1.append(heights[i]);
			i++;
		}
		stddev_pass1.push_back(pass1.get_std_dev());
	}	

	vector<long double> stddev_pass2;
	StdDevCalc pass2;

	for(int i=heights.size()-1; i>=0; i--)
	{
		while(i-1>=0 && heights[i]==heights[i-1])
		{
			pass2.append(heights[i]);
			i--;
		}
		stddev_pass2.push_back(pass2.get_std_dev());
	}

	int ind=0;
	int height=0;
	long double stddev_diff=INT_MAX;

	for(int i=0; i<heights.size(); i++)
	{

		while(i+1<heights.size() && heights[i]==heights[i+1])
		{
			i++;
		}
		
		if(abs(stddev_pass1[ind]-stddev_pass2[stddev_pass1.size()-1-ind]) < stddev_diff)
		{
			height=heights[i];
		}

	}

	return height;
}

//*******************************************************Static EValuation*****************************************************************//
int SENCTree::getLabel(int pointX, vector<treenode *>& newClass, data & refdeltaD)
{
	//_dataObject = refdeltaD;
	treenode * node = rootNode;
	while(!node->isLeaf){
		if(refdeltaD.dataVector[pointX]->attributes[node->splitAttribute] < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	
	newClass.push_back(node);

	if(getRadius(refdeltaD.dataVector[pointX]->attributes, node->centroid) > node->radius)
	{	

		return node->fre.size(); 
	} 
	return node->label;
}

long double SENCTree::computePathLength(int pointX,  data & testDataObject){
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
	//pathLength = node->nodeHeight - 1 + node->pathLengthEst;
	return pathLength;
}

long double SENCTree::computeRelativeMass(int pointX,  data & testDataObject){
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

long double SENCTree::pathLengthEstimationForUnbuiltTree(int nodeSize){
	long double es = 0;
	if(nodeSize == 2){es=1;}
	else if(nodeSize > 2){es = (2 * (log(nodeSize-1) + 0.5772156649)) - (2 * (nodeSize - 1) / nodeSize);}	
	return (es);
}


//***************************************************Incremental SENCTree creation************************************************************//

void SENCTree::insertUpdatesInSENCTree(vector<pair<int, vector<treenode *>>>& indices, int sampleSize, int treeId, data & refdeltaD){
	vector<int> samples=sample_without_replacement(sampleSize, indices.size());
	for(int j=0; j<samples.size(); j++)
	{
		indices[samples[j]-1].second[treeId]->updateIndices.push_back(indices[samples[j]-1].first);
	}
	int numLeafNodes=leafNodes.size();

	for(int i=0; i<numLeafNodes; i++)
	{
		if(leafNodes[i]->updateIndices.size()>0)
		{
			int pseudoPoints=0;
			for(int j=0;j<leafNodes[i]->fre.size();j++)
			{
				pseudoPoints+=leafNodes[i]->fre[j];
			}
			reconstructSENCTree(leafNodes[i], pseudoPoints, refdeltaD);

			// leafNodes[i]->isLeaf = false;
			leafNodes.erase(leafNodes.begin()+i);
			i--;
			numLeafNodes--;
		}

	}
}

void SENCTree::reconstructSENCTree(treenode * rootNode, int pseudoPoints, data & refdeltaD){
	
    queue<pair<treenode *, bool>> BFTforNodes;
    BFTforNodes.push({rootNode, true});
    while(!BFTforNodes.empty()){
    	pair<treenode *, bool> tmp = BFTforNodes.front();
		treenode * currNode = tmp.first;
		BFTforNodes.pop();
		if(currNode){
			currNode->nodeSize += currNode->updateIndices.size();
			currNode->isLeaf = false;
			if(tmp.second)
			{
				currNode->nodeSize += pseudoPoints;
			}
			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
			if(currNode->nodeSize <= _minSizeOfNode || currNode->updateIndices.size()==0){
        		currNode->isLeaf = bool(1);

// 				//Creating Ball
				
				currNode->centroid.resize(refdeltaD.getnumAttributes(), 0.0);
				for(int i=0; i<currNode->updateIndices.size(); i++)
				{
					for(int j=0;j<refdeltaD.getnumAttributes();j++)
					{
						currNode->centroid[j]+=(refdeltaD.dataVector[currNode->updateIndices[i]]->attributes[j] - currNode->centroid[j])/(i+1);
					}
				}

				if(tmp.second)
				{
					for(int i=0; i<pseudoPoints; i++)
					{
						for(int j=0;j<refdeltaD.getnumAttributes();j++)
						{
							currNode->centroid[j]+=(rootNode->centroid[j] - currNode->centroid[j])/(currNode->updateIndices.size()+i+1);
						}
					}
				}

				for(int i=0;i<currNode->updateIndices.size();i++)
				{
					currNode->radius=max(currNode->radius, getRadius(refdeltaD.dataVector[currNode->updateIndices[i]]->attributes, currNode->centroid));
				}
				if(tmp.second)
				{
					currNode->radius=max(currNode->radius, getRadius2(rootNode->centroid, currNode->centroid));
				}

// 				//Maintain frequency
				currNode->fre.resize(rootNode->fre.size()+1, 0);
				currNode->fre[rootNode->fre.size()]=currNode->updateIndices.size();
				if(tmp.second)
				{
					for(int i=0; i<rootNode->fre.size(); i++)
					{
						currNode->fre[i]=rootNode->fre[i];
					}
				}

        		currNode->updateIndices.clear();
        		currNode->updateIndices.resize(0);
				leafNodes.push_back(currNode);
    		}
    		else{
    			currNode->splitValue = currNode->splitInfoSelectionForUpdates(refdeltaD, rootNode->centroid, tmp.second);
    			currNode->createLeftChild();
				currNode->createRightChild();
				for(int i=0; i<currNode->updateIndices.size(); i++){     
            		if(refdeltaD.dataVector[currNode->updateIndices[i]]->attributes[currNode->splitAttribute]<currNode->splitValue){
                		currNode->lChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
            		}
            		else{
                		currNode->rChildAdd->updateIndices.push_back(currNode->updateIndices[i]);
            		}

        		}
        		
        		currNode->updateIndices.clear();
        		currNode->updateIndices.resize(0);
        		
				if(tmp.second)
				{
					if(rootNode->centroid[currNode->splitAttribute] < currNode->splitValue)
					{
						BFTforNodes.push({currNode->lChildAdd, true});
						BFTforNodes.push({currNode->rChildAdd, false});
					}
					else
					{
						BFTforNodes.push({currNode->lChildAdd, false});
						BFTforNodes.push({currNode->rChildAdd, true});
					}
				}
				else
				{
					BFTforNodes.push({currNode->lChildAdd, false});
        			BFTforNodes.push({currNode->rChildAdd, false});
				}
        		
    		}
    	}
    }
}


void SENCTree::deleteSubTree(treenode * node){
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

























