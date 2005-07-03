/** @file  DecisionTree.h
    @brief declaration of class DecisionTree

    @author T. Burnett
    $Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/DecisionTree.h,v 1.1.1.1 2005/07/03 21:31:35 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef DecisionTree_h
#define DecisionTree_h
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


/** @class DecisionTree
@brief Define and implement a decision tree with minimal information

This is a functor class, with argument an object that behaves like a vector.

*/
class DecisionTree {
public:

    /// @brief default constructor
    DecisionTree(std::string title="Decision Tree");

    /** @brief read a tree from the input stream
    The ascii file format is 
    @verbatim
    title
    id index value
    id index value
    ...
    @endverbatim
    where id is the node id, index is, if non-negative, then index of the Value object,
    and value is either the cut value, or the purity of a leaf node, signified by index<0. 
    Parent nodes must precede children; the first id must be 1 for the root.
    */
    DecisionTree(std::ifstream& in);

    /// Abstract class to interface to a source of values
    class Values {
    public:
        virtual double operator[](int index)const=0;
        virtual ~Values(){}
    private:
    };
    /** evaluate, returning purity of appropriate leaf node
    @param row
    @param tree_count [0] if nonzero, maximum trees to evaluate
    */
    double operator()(const std::vector<float> row, int tree_count=0)const;

    double operator()(const Values& vals)const;
    ~DecisionTree();

    
    /**@brief add a node
        @param id The node id, 0:tree weight, 1:root, even: left child of id/2, odd right child of (id-1)/2
        @param index Identifies the value to test. (-1 means a leaf node, -10 means tree weight)
        @param value Either the value to test, or, for a leaf node, the  purity of the node. tree weight for index=-10
    */
    void addNode(unsigned int id, int index, double value);

    /**@brief add DecisionTree
        @param tree DecisionTree to be appended
    */
    void addTree(DecisionTree * tree);

    void print(std::ostream& out=std::cout)const;
    std::string title()const{ return m_title;}

private:
    /// forward declaration of nested class
    class Node;
    Node* find(unsigned int id);
    void printNode(std::ostream& out , const DecisionTree::Node * node, unsigned int id)const;

    std::vector<std::pair<double, Node*> > m_rootlist;
    std::string m_title;
    double m_lastWeight;
};
#endif


