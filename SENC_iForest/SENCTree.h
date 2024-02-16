#ifndef SENCTree_H
#define SENCTree_H
//#include "data.cpp"
#include "treenode.cpp"
//#include <boost/serialization/list.hpp>
#include <vector>

/*typedef struct pToN{
    //double* attributes;
    bool isPresent;
    int coresspondingNode;
}pToN*/

class SENCTree
{
    public:
        SENCTree(const data &);
        SENCTree(const data &,int, int, int, int, int);
        virtual ~SENCTree();
        void constructSENCTree(int);
        void reconstructSENCTree(treenode *, int);
		long double computePathLength(int, const data &);
		long double computeRelativeMass(int, const data &);
        long double pathLengthEstimationForUnbuiltTree(int );
		void insertUpdatesInSENCTree(vector<pair<int, vector<treenode *>>> &, int, int);
		void deleteSubTree(treenode *);
        int findAnomalyRegion();
        // void createBall();
		treenode * rootNode;
        vector<treenode *> leafNodes;
        int anomalyHeight;
        int getLabel(int, vector<treenode *> &);

    protected:

    private:
	int _sampleSize;    
	int _maxTreeHeight;
    int _maxNumOfNodes;
	int _avgPLEstOfBST;
	int _avgPLCompOfBST;
    int _minSizeOfNode;
	const data & _dataObject;
	//treenode * _rootNode;
    

    /*friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & maxTreeHeight;
        ar & maxNumOfNodes;
        ar & BOOST_SERIALIZATION_NVP(pointToNode);
    }*/
};

#endif // SENCTree_H
