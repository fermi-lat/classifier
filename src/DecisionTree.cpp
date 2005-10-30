/** @file  DecisionTree.cpp
@brief implementation  of class DecisionTree

@author T. Burnett
$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/DecisionTree.cpp,v 1.2 2005/10/29 17:30:13 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "classifier/DecisionTree.h"

#include <stdexcept>
#include <sstream>
#include <cassert>

DecisionTree::DecisionTree(std::string title)
: m_title(title)
{
}

DecisionTree::DecisionTree(std::ifstream& input){
    // first line is the title
    if( ! input.is_open() ) throw std::invalid_argument("DecisionTree::DecisionTree: bad input file");
    std::string buffer;
    std::getline(input, buffer);
    m_title = buffer;

    while( ! input.eof() ) {
        Identifier_t id;
        int index; double value;
        input >> id >> index >> value;
        if (id >= 0) {
            addNode(id, index, value);
        }
    }
}
//! @class DecisionTree::Node
//! @brief Nested class manages the structure of nodes
class DecisionTree::Node {
public:
    Node(int index, double value)
        :m_index(index), m_value(value), m_left(0), m_right(0)
    {
        assert(index>=-10 && index<100); // check for bad logic
    }
    ~Node(){ 
        delete m_left; delete m_right;
    }
    /// set a child, left or right depending on odd or even id
    void setChild(Identifier_t child_id, Node* child) 
    {
        if( (child_id & 1)!=0) m_right = child;
        else m_left = child;
    }
    /// templated member function needs an array-type guy
    template<class C>  
        double evaluate(const C& values)const
    {
        //std::cerr << (m_index>=0? values[m_index]: -1) << "< " << m_value << std::endl;
        if( isLeaf() ) return m_value;
        if( isWeight() ) return m_right->evaluate(values);
        return values[m_index]<m_value
            ? m_left->evaluate(values)
            : m_right->evaluate(values) ;
    }
    bool isLeaf()const{return m_index == -1;}
    bool isWeight()const{return m_index == -10;}

    Node* left()const{return m_left;}
    Node* right()const{return m_right;}
    int index()const{return m_index;}
    double value()const{return m_value;}
private:
    int m_index;
    double m_value;
    Node* m_left;
    Node* m_right;
};
DecisionTree::~DecisionTree()
{ 
}
double DecisionTree::operator()(const std::vector<float>& row, int tree_count)const
{
    class FloatVector : public Values 
    { public:
    FloatVector(const std::vector<float>& row): m_row(row){}
    double operator[](int index)const{return m_row[index];}
    std::vector<float> m_row; // make a local copy
    } vals(row);

    return (*this)(vals);
}
double DecisionTree::operator ()(const Values& vals)const
{
    double weighted_sum=0, sum_of_weights=0;
    std::vector<std::pair<double, Node*> >::const_iterator it= m_rootlist.begin();
    for( ; it!=m_rootlist.end(); ++it){ 
        double 
            weight = (*it).first, // weight associated with this tree
            value = (*it).second->evaluate(vals); // get the value for input vars
        if( weight <= 0. ){
            // this is a filter: if zero result, just return
            if( value == 0) return 0;
            if( value != 1.0 ) {
                throw std::runtime_error(
                    "DecisionTree::operator(): processing a filter, expect only 0 or 1 leaf nodes");
            }
            continue; // otherwise go to next tree
        }
        // not a filter: continue
        sum_of_weights += weight;
        weighted_sum += weight * value;
    }
    // done with loop: note that if there were no trees, we accept.
    return sum_of_weights != 0 ? weighted_sum/sum_of_weights : 1;
}

DecisionTree::Node* DecisionTree::find(Identifier_t id)
{
    static int nbits=8*sizeof(Identifier_t);
    static Identifier_t hibit=(Identifier_t(1)<<(nbits-1));
    Node* node = m_rootlist.back().second; // get the node with tree weight
    if( id==0) return node;
    node = node->right();  // get the first actual node 
    if( id==1) return node;
    int depth=nbits;
    while( (id & hibit) ==0) {id= id << 1; --depth;}
    id = id<<1; --depth; // one more to shift off the root node
    for( int i =0; i<depth; ++i, id=id<<1){
        if( (id & hibit)!=0 ) node=node->right();
        else  node=node->left();
        if( node==0) {
            std::stringstream buf;
            buf <<"DecisionTree::find - node "<< id << " not found";
            throw std::runtime_error(buf.str());
        }
    }
    return node;
}

void DecisionTree::addNode(Identifier_t id, int index, double value)
{
    Node * child = new Node(index, value);
    if ( id==0 ) { 

        // create the node with tree weight
        m_rootlist.push_back(std::make_pair(value,child));

    } else if( id==1) { 

        // create the node with first data
        if ( m_rootlist.size() == 0) { 

            // backward compatibility: force a tree node if first node added
            // is a root.
            Node * orig = new Node(-10, 1);
            orig->setChild(id,child);
            m_rootlist.push_back(std::make_pair(1,orig));

        } else { 

            // set the first node as right child to node 0 for multiple trees
            std::pair<double, Node*> id0pair = m_rootlist.back();
            (id0pair.second)->setChild(id,child);
        }
    } else {
        Node* parent = find(id/2);
        parent->setChild(id,child);
    }
}

void DecisionTree::addTree(const DecisionTree * tree)
{
    if( m_title != tree->title()) {
        throw std::runtime_error("DecisionTree::addTree - merging trees of different flavours");
    } else {
        m_rootlist.insert(m_rootlist.end(),tree->m_rootlist.begin(),tree->m_rootlist.end());
    }
}

void DecisionTree::printNode(std::ostream& out , const DecisionTree::Node * node, Identifier_t id)const
{
    assert (node!=0); // baad logic!
    out << "\t"<< id << "\t" << node->index() <<"\t" << node->value() << std::endl;
    if( node-> isLeaf()) return;
    // special treatment for node 0, with no left child
    if( node-> isWeight()) printNode(out,node->right(),1);
    else {
        printNode(out,node->left(), 2*id);
        printNode(out,node->right(), 2*id+1);
    }
}
void DecisionTree::print(std::ostream& out)const
{
    out << m_title << std::endl;
    std::vector<std::pair<double, Node*> >::const_iterator it= m_rootlist.begin();
    for( ; it!=m_rootlist.end(); ++it){ 
        printNode(out, (*it).second,0);
    }
}
