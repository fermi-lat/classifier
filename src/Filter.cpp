/** @file Filter.cpp
@brief implementation of the class Filter

@author T.Burnett
$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/Filter.cpp,v 1.1 2005/10/29 17:30:13 burnett Exp $
*/

#include "classifier/Filter.h"
#include <algorithm>
#include <stdexcept>


Filter::Filter(std::vector<std::string>& vars, DecisionTree& tree)
: m_vars(vars)
, m_tree(tree)
, m_id(1)
{}

void Filter::makeTree(std::ifstream& input)
{
    if( ! input.is_open() ) throw std::invalid_argument("Filter::makeTree: bad input file");

    m_tree.addNode(0, -10, 0.);  //the tree node, flagged with zero weight

    while( ! input.eof() ) {
        std::string buffer;
        std::getline(input, buffer);
        std::string name, op;
        double value;
        input >> name >> op >> value;
        if( name.empty() || name[0]=='#') continue; // ignore, a comment
        addCut(name, op, value);
    }
    m_tree.addNode(m_id, -1, 1.0); // final node
}


void Filter::addCut(std::string name, std::string op, double value)
{
    int left = 0, index=-1;
    if( op == ">=" ){
        left =0;
    }else if( op=="<") {
        left = 1;
    }else {
        throw std::invalid_argument("Filter::makeTree: only allow '<' and '>=', found "+op);
    }
    m_tree.addNode(m_id, find_index(name), value); // put in the cut, a branch node
    m_tree.addNode(2*m_id+left, -1, 0); // node to die
    m_id = 2*m_id + 1-left;
}


int Filter::find_index(const std::string& name)
{
    std::vector<std::string>::iterator test = std::find(m_vars.begin(),m_vars.end(), name);
    if( test== m_vars.end()){
        m_vars.push_back(name);
    }
    return test-m_vars.begin();
}
