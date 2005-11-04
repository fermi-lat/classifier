/** @file  DecisionTree.h
    @brief declaration of class DecisionTree

    @author T. Burnett
    $Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/DecisionTree.h,v 1.4 2005/10/30 23:31:02 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef DecisionTree_h
#define DecisionTree_h
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


/** @class DecisionTree
@brief Define and implement a decision tree, or set of trees, with minimal information

This is a functor class, with argument an object that behaves like a vector.

Each tree has an associated weight, and returns a value, usually the purity from 
the training. The function returns the weighted sum of the values. 

A special tree may be defined with zero weight: this is presumed to be a <i>filter</i>,
and must return either 0 or 1. If 0, the function will return 0. If 1, the value will be that of 
the subsequent trees.


*/

class DecisionTree {
public:
    typedef long long Identifier_t;

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
    where: 
    @param id     the node id: 0 for tree, 1 for root node, otherwise a child node, which must have have been preceded by its parent  
    @param index  if non-negative, then the index of the Value object. -1 for a leaf node
    @param value  either the cut value, or the purity of a leaf node, signified by index<0. 

    Parent nodes must precede children; the first id must be 0 to for tree properties, then 1 for the root.
    */
    DecisionTree(std::ifstream& in);

    /// @class Values
    /// @brief Abstract class to interface to a source of values.
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
    double operator()(const std::vector<float>& row, int tree_count=0) const;

    double operator()(const Values& vals) const;

    ~DecisionTree();

    
    /**@brief add a node
        @param id The node id, 0:tree weight, 1:root, even: left child of id/2, odd right child of (id-1)/2
        @param index Identifies the value to test. (-1 means a leaf node, -10 means tree weight)
        @param value Either the value to test, or, for a leaf node, the  purity of the node. tree weight for index=-10
    */
    void addNode(Identifier_t id, int index, double value);

    /**@brief add DecisionTree
        @param tree DecisionTree to be appended

        The new tree must have the same title.
    */
    void addTree(const DecisionTree * tree);

    void print(std::ostream& out=std::cout)const;
    std::string title()const{ return m_title;}


    /** @brief formatted print of the tree, assuming it is a filter.
        @param varnames list of corresponding variable names
        @param out [cout] output stream
        @param indent ["\t"] string to precede each line.

    */
    void printFilter(const std::vector<std::string>& varnames, std::ostream& out=std::cout, std::string indent="\t")const;

    /// forward declaration of nested class representing a node.
    class Node;

private:
    Node* find(Identifier_t id);
    void printNode(std::ostream& out , const DecisionTree::Node * node, Identifier_t id)const;

    std::vector<std::pair<double, Node*> > m_rootlist; ///< vector of pointers to root nodes
    std::string m_title;
};
#endif
