#ifndef ITREE_H
#define ITREE_H
#include "data.h"
#include "treenode.h"
#include <boost/serialization/list.hpp>
#include <vector>
#include <random>

/*typedef struct pToN{
    //double* attributes;
    bool isPresent;
    int coresspondingNode;
}pToN*/

class itree
{
    public:
        itree(const data &);
        itree(const data &,int, int, int, int, int);
        virtual ~itree();
        void constructiTree(int);
		long double computePathLength(int, const data &);
		long double computeRelativeMass(int, const data &);
        long double pathLengthEstimationForUnbuiltTree(int );
		void insertUpdatesIniTree(int, int,data &);
		void deleteSubTree(treenode *);
		treenode * rootNode;
    protected:

    private:
	int _sampleSize;    
	int _maxTreeHeight;
    int _maxNumOfNodes;
	int _avgPLEstOfBST;
	int _avgPLCompOfBST;
	int _exLevel;
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

#endif // ITREE_H
