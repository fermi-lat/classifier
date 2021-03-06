/** @file Filter.h
@brief declaration of the class Filter

@author T.Burnett
$Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/Filter.h,v 1.2 2005/10/30 01:26:55 burnett Exp $
*/
#ifndef classifier_Filter_h
#define classifier_Filter_h

#include <string>
#include <vector>
#include <ostream>

#include "classifier/DecisionTree.h"

/** @class Filter
    @brief Create a special decision tree that is a simple filter.

    It will return either 0 or 1. 

*/
class Filter { 
public:
    /** ctor
    @param vars Array of variable names. Will append variables not found
    @param tree Should be a new tree: nodes will be added.

    */
    Filter(std::vector<std::string>& vars, DecisionTree& tree);

    ~Filter();

 
    /// append a new cut
    /// @param name variable name: will be appended to the vars if not there
    /// @param op one of "<", or ">=". 
    void addCut(std::string name, std::string op, double value);

    /// add cuts from the file
    void addCutsFrom(const std::string& filename);

    /// optional finisher
    void close();

    /// dump
    void print(std::ostream& out)const;

private:

    int find_index(const std::string&);

    std::vector<std::string>& m_vars;

    DecisionTree& m_tree;

    DecisionTree::Identifier_t m_id;

};
#endif
