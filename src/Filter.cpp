/** @file Filter.cpp
@brief implementation of the class Filter

@author T.Burnett
$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/Filter.cpp,v 1.3 2005/10/30 01:26:55 burnett Exp $
*/

#include "classifier/Filter.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>


Filter::Filter(std::vector<std::string>& vars, DecisionTree& tree)
: m_vars(vars)
, m_tree(tree)
, m_id(0)
{}

Filter::~Filter()
{
    close();
}

void Filter::addCutsFrom(const std::string& filename)
{
    std::ifstream input(filename.c_str());
    if( ! input.is_open() ) throw std::invalid_argument("Filter::addCutsFrom: cannot open file "+filename);

    while( ! input.eof() ) {
        std::string line;
        std::getline(input, line);
        if( line.empty() || line[0]=='#') continue; // ignore, a comment
        if( line[0] == '@' ) { // indirection
            int pos = filename.find_last_of("/");
            std::string newfile(filename.substr(0,pos+1)+line.substr(1));
            addCutsFrom( newfile );
            continue;
        }
        std::string name, op;
        double value;
        std::stringstream s(line);
        s >> name >> op >> value;
        addCut(name, op, value);
    }
}

void Filter::close()
{
    if( m_id>0) m_tree.addNode(m_id, -1, 1.0); // final node
    m_id=-1; // flag
}

void Filter::addCut(std::string name, std::string op, double value)
{
    if( m_id==0 ) {
        // this is the root node: need a tree node
        m_tree.addNode(0, -10, 0.);  //the tree node, flagged as such by zero weight
        m_id = 1; 
    }
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
    int index = test-m_vars.begin();
    if( test== m_vars.end()){
        m_vars.push_back(name);
    }
    return index;
}

void Filter::print(std::ostream& out)const
{
   m_tree.printFilter( m_vars, out);

}