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


double treenode::splitInfoSelection(const data &dataObject, int &exLevel, int random_seed){
   
    std::mt19937_64 RandomEngine (random_seed);
    //get maximum and minimum value of every attribute.
    std::vector<double> Xmins, Xmaxs;
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
        Xmins.push_back((double) dataObject.dataVector[(dataPointIndices[0])]->attributes[i]);
        Xmaxs.push_back((double) dataObject.dataVector[(dataPointIndices[0])]->attributes[i]);
        for(int j=1; j<dataPointIndices.size(); j++)
        {
            if(Xmins[i] > (double) dataObject.dataVector[(dataPointIndices[j])]->attributes[i]) Xmins[i]=(double) dataObject.dataVector[(dataPointIndices[j])]->attributes[i];
            if(Xmaxs[i] < (double) dataObject.dataVector[(dataPointIndices[j])]->attributes[i]) Xmaxs[i]=(double) dataObject.dataVector[(dataPointIndices[j])]->attributes[i];
        }
    }

    //get random point from which the hyperplane would pass.
    std::vector<double> point;
    point.resize(dataObject.getnumAttributes(), 0.0);
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
        point[i]=std::uniform_real_distribution<double> (Xmins[i], Xmaxs[i])(RandomEngine);
    }

    //get random normal vector of the hyperplane.
    normal_vector.resize(dataObject.getnumAttributes(), 0.0);
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
        normal_vector[i]=std::normal_distribution<double> (0.0, 1.0)(RandomEngine);
    }

    //get attributes not be used for dividing the data set.
    std::vector<int> normvect_zero_index = sample_without_replacement (dataObject.getnumAttributes()-exLevel-1, dataObject.getnumAttributes());
    //set normal vector component as zero for attributes not to be used for dividing the data set.
    for(int j=0; j<dataObject.getnumAttributes()-exLevel-1; j++)
    {
        normal_vector[normvect_zero_index[j]-1] = 0.0;
    }
    
    //get split value by taking inner product of normal_vector with the random point.
    double  pdotn = inner_product(point, normal_vector);
    pdotn = pdotn*1.5;
    return pdotn; 
}

double treenode::splitInfoSelectionForUpdates(const data &dataObject, int &exLevel, int random_seed){
	   
    std::mt19937_64 RandomEngine (random_seed);
	//get maximum and minimum value of every attribute.
    std::vector<double> Xmins, Xmaxs;
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
    	Xmins.push_back((double) dataObject.dataVector[(updateIndices[0])]->attributes[i]);
        Xmaxs.push_back((double) dataObject.dataVector[(updateIndices[0])]->attributes[i]);
        for(int j=1; j<updateIndices.size(); j++)
        {
            if(Xmins[i] > (double) dataObject.dataVector[(updateIndices[j])]->attributes[i]) Xmins[i]=(double) dataObject.dataVector[(updateIndices[j])]->attributes[i];
            if(Xmaxs[i] < (double) dataObject.dataVector[(updateIndices[j])]->attributes[i]) Xmaxs[i]=(double) dataObject.dataVector[(updateIndices[j])]->attributes[i];
        }
    }

    //get random point from which the hyperplane would pass.
    std::vector<double> point;
    point.resize(dataObject.getnumAttributes(), 0.0);
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
        point[i]=std::uniform_real_distribution<double> (Xmins[i], Xmaxs[i])(RandomEngine);
    }

    //get random normal vector of the hyperplane.
    normal_vector.resize(dataObject.getnumAttributes(), 0.0);
    for(int i=0; i<dataObject.getnumAttributes(); i++)
    {
        normal_vector[i]=std::normal_distribution<double> (0.0, 1.0)(RandomEngine);
    }

    //get attributes not be used for dividing the data set.
    std::vector<int> normvect_zero_index = sample_without_replacement (dataObject.getnumAttributes()-exLevel-1, dataObject.getnumAttributes());
    //set normal vector component as zero for attributes not to be used for dividing the data set.
    for(int j=0; j<dataObject.getnumAttributes()-exLevel-1; j++)
    {
        normal_vector[normvect_zero_index[j]-1] = 0.0;
    }
    
    //get split value by taking inner product of normal_vector with the random point.
    double  pdotn = inner_product(point, normal_vector);
    pdotn = pdotn*1.5;
    return pdotn; 
}


void treenode::newMinMaxSelection(const data &dataObject, double *min, double *max){
	// int maxVal = maximumVal;
	// int minVal = minimumVal;
	// for( int i = 0; i < updateIndices.size(); i++){
	// 			if(maxVal < dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
	// 			maxVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
	// 		}
    //     		if(minVal > dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
    //     			minVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
    //     		}
    //     }
    //     *min = minVal;
    //     *max = maxVal;
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

	
