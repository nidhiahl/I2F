#ifndef TREENODE_H
#define TREENODE_H
# include "data.h"
#include <vector>
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
        int splitAttribute;
        double splitValue;
        double minimumVal;
        double maximumVal;
        int nodeSize;   
        int nodeHeight;
		long double pathLengthEst;
        bool isLeaf;
        
		
        treenode();
        treenode(int);
        virtual ~treenode();
        double splitInfoSelection(const data &);
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
            ar & splitAttribute;
            ar & splitValue;
			ar & minimumVal;
			ar & maximumVal;
            ar & nodeSize;
            ar & pathLengthEst;
            ar & nodeHeight;
            ar & isLeaf; 
			
            
        }
};

#endif // TREENODE_H
