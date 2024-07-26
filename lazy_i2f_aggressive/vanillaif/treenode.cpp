#include "treenode.h"
#include <math.h>
#include <random>

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

//inner product of two vectors
inline double inner_product(std::vector<double>& point, std::vector<double>& normal_vector)
{
    double result=0.0;
    for(int i=0;i<point.size();i++) result+=point[i]*normal_vector[i];
    return result;
}

treenode::treenode()
{
    isLeaf = bool(0);
    isNew = bool(0);
    nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    nodeMass = 0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
    minimumVal=999999.0;
    maximumVal=-999999.0;
	updateIndices.resize(0);
}

treenode::treenode(int nId): nodeId(nId)
{
    isLeaf = bool(0);
    isNew = bool(0);
    parentId = nodeId == 0 ? 0 : (nodeId-1)/2;
    lChildId = -1;
    rChildId = -1;
	nodeHeight = (int)log2(nodeId+1)+1;
    nodeMass = 0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
    minimumVal=999999.0;
    maximumVal=-999999.0;
	updateIndices.resize(0);

	
}

treenode::~treenode()
{
    //dtor
}


double treenode::splitInfoSelection(const data &dataObject){ 
        
    int maxVal = -999999;
	int minVal = 999999;
	int attempts = 0;
	while(attempts < 10){
		splitAttribute = rand()%dataObject.getnumAttributes();
		for( int i = 0; i < dataPointIndices.size(); i++){
        		if(maxVal < dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute];
        		}
        }
        attempts = attempts + 1;
        double dataDiff = maxVal - minVal;
		if(dataDiff >= 0.0000000000000001){
			attempts = 10;
		}
	}
	maximumVal = maxVal;
	minimumVal = minVal;
	return (minVal + ((double)rand()/RAND_MAX)*(maxVal-minVal));
    
    
}

double treenode::splitInfoSelectionForUpdates(const data &dataObject){
        
    int maxVal = -999999;
	int minVal = 999999;
	int attempts = 0;
	while(attempts < 10){
		splitAttribute = rand()%dataObject.getnumAttributes();
		//cout<<"after attribute selection"<<endl;
		for( int i = 0; i < updateIndices.size(); i++){
        		if(maxVal < dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
        		}
        }
        attempts = attempts + 1;
        double dataDiff = maxVal - minVal;
		if(dataDiff >= 0.0000000000000001){
			attempts = 10;
		}
	}
	maximumVal = maxVal;
	minimumVal = minVal;
	return (minVal + ((double)rand()/RAND_MAX)*(maxVal-minVal));
      
}


void treenode::newMinMaxSelection(const data &dataObject, double *min, double *max){
	 int maxVal = maximumVal;
	 int minVal = minimumVal;
	 for( int i = 0; i < updateIndices.size(); i++){
	 			if(maxVal < dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
	 			maxVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
	 		}
         		if(minVal > dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
         			minVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
         		}
         }
         *min = minVal;
         *max = maxVal;
}



void treenode::createLeftChild(){
	lChildAdd = new treenode(2*nodeId+1);
	lChildAdd->parentAdd = this;
	lChildId = lChildAdd->nodeId;
}

void treenode::createRightChild(){
	rChildAdd = new treenode(2*nodeId+2);
	rChildAdd->parentAdd = this;
	rChildId = rChildAdd->nodeId;
}

	
