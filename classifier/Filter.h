/** @file Filter.h
@brief declaration of the class Filter

@author T.Burnett
$Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/Filter.h,v 1.1.1.1 2005/07/03 21:31:35 burnett Exp $
*/
#ifndef classifier_Filter_h
#define classifier_Filter_h

#include <string>
#include <vector>
#include <fstream>

#include "classifier/DecisionTree.h"

/** @class Filter
    @brief Create a special decision tree that is a simple filter.

    It will return either 0 or 1. 

*/
class Filter { 
public:
    /** ctor
    @param vars Array of variable names. Will append variables not found
    @param Should be a new tree: nodes will be added.


    */
    Filter(std::vector<std::string>& vars, DecisionTree& tree);


    /** @brief append a filter tree from the file

    */
    void makeTree(std::ifstream& input);

    /// append a new cut
    /// @param name variable name: will be appended to the vars if not there
    /// @param op one of "<", or ">=". 

    void addCut(std::string name, std::string op, double value);

private:

    int find_index(const std::string&);

    std::vector<std::string>& m_vars;

    DecisionTree& m_tree;

    DecisionTree::Identifier_t m_id;

};
#endif
