#ifndef TREENODE_H
#define TREENODE_H
# include "data.h"
#include <vector>
#include <unordered_set>
#include <boost/serialization/list.hpp>


class treenode
{
    public:

        int nodeId;
        int parentId;
        int lChildId;
        int rChildId;
        treenode *parentAdd;
        treenode *lChildAdd;
        treenode *rChildAdd;
        vector<int> dataPointIndices;
        // int splitAttribute;
        double splitValue;
        std::vector<double> normal_vector;
        double minimumVal;
        double maximumVal;
        int nodeSize;   
        int nodeHeight;
		long double pathLengthEst;
        bool isLeaf;
        bool isNew;
		int nodeMass;
		vector<int> updateIndices;
        

        treenode();
        treenode(int);
        virtual ~treenode();
        double splitInfoSelection(const data &, int &, int);
        double splitInfoSelectionForUpdates(const data &, int &, int);
        void newMinMaxSelection(const data &, double *min, double *max);
		void createLeftChild();
		void createRightChild();

    protected:

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & nodeId;
            ar & parentId;
            ar & lChildId;
            ar & rChildId;
            //ar & parentAdd;
            //ar & lChildAdd;
            //ar & rChildAdd;
            //ar & BOOST_SERIALIZATION_NVP(dataPointIndices);
            // ar & splitAttribute;
            ar & normal_vector;
            ar & splitValue;
			ar & minimumVal;
			ar & maximumVal;
            ar & nodeSize;
            ar & pathLengthEst;
            ar & nodeHeight;
            ar & isLeaf; 
			//ar & nodeMass;
            
        }
};

#endif // TREENODE_H
