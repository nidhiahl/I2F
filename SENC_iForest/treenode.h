#ifndef TREENODE_H
#define TREENODE_H
//# include "data.cpp"
#include <vector>
//#include <boost/serialization/list.hpp>


class treenode
{
    public:

        long long nodeId;
        long long parentId;
        long long lChildId;
        long long rChildId;
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
        bool isNew;
		int nodeMass;
		vector<int> updateIndices;
        vector<long double> centroid;
        long double radius;
        vector<int> fre;
        int label;

        treenode();
        treenode(int);
        virtual ~treenode();
        double splitInfoSelection(const data &);
        double splitInfoSelectionForUpdates(const data &, vector<long double> &, bool);
        void newMinMaxSelection(const data &, double *min, double *max);
		void createLeftChild();
		void createRightChild();

    protected:

    private:
        /*friend class boost::serialization::access;
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
            ar & splitAttribute;
            ar & splitValue;
			ar & minimumVal;
			ar & maximumVal;
            ar & nodeSize;
            ar & pathLengthEst;
            ar & nodeHeight;
            ar & isLeaf; 
            
            ar & centroid;
            ar & radius;
            ar & fre;
			//ar & nodeMass;
            
        }*/
};

#endif // TREENODE_H
