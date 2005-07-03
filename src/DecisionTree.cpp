/** @file  DecisionTree.cpp
@brief implementation  of class DecisionTree

@author T. Burnett
$Header: /cvsroot/d0cvs/classifier/src/DecisionTree.cpp,v 1.9 2005/04/04 21:31:40 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "classifier/DecisionTree.h"

#include <stdexcept>

DecisionTree::DecisionTree(std::string title)
: m_title(title),m_lastWeight(0)
{
}

DecisionTree::DecisionTree(std::ifstream& input){
    // first line is the title
    if( ! input.is_open() ) throw std::invalid_argument("DecisionTree::DecisionTree: bad input file");
    char buffer[80];
    input.getline(buffer,sizeof(buffer));
    m_title = std::string(buffer);
    m_lastWeight = 0;
    while( ! input.eof() ) {
        int id, index; double value;
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
    {}
    ~Node(){ 
        delete m_left; delete m_right;
    }
    /// set a child, left or right depending on odd or even id
    void setChild(int child_id, Node* child) 
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
double DecisionTree::operator()(const std::vector<float> row, int tree_count)const
{
  double weighted_sum=0, tree_wt=0;
  if( tree_count==0) tree_count = m_rootlist.size();
  std::vector<std::pair<double, Node*> >::const_iterator it= m_rootlist.begin();
  for( ; it!=m_rootlist.end(); ++it){ 
    weighted_sum += (*it).first * (*it).second->evaluate(row);
    tree_wt += (*it).first;
    if(tree_count-- ==0)  break;
  }
  return tree_wt != 0 ? weighted_sum/tree_wt : 0;
}
double DecisionTree::operator ()(const Values& vals)const
{
  double weighted_sum=0, tree_wt=0;
  std::vector<std::pair<double, Node*> >::const_iterator it= m_rootlist.begin();
  for( ; it!=m_rootlist.end(); ++it){ 
    weighted_sum += (*it).first * (*it).second->evaluate(vals);
    tree_wt += (*it).first;
  }
  return tree_wt != 0 ? weighted_sum/tree_wt : 0;
}

DecisionTree::Node* DecisionTree::find(unsigned int id)
{
    static int nbits=32;
    static unsigned int hibit=(1<<(nbits-1));
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
            throw std::runtime_error("DecisionTree::find - node not found");
        }
    }
    return node;
}

void DecisionTree::addNode(unsigned int id, int index, double value)
{
  Node * child = new Node(index, value);
  if ( id==0 ) { // create the node with tree weight
    m_lastWeight = value;
    m_rootlist.push_back(std::make_pair(value,child));
  } else if( id==1) { // create the node with first data
    if (m_lastWeight == 0) { // backward compatibility (read single tree files)
      Node * orig = new Node(-10, 1);
      orig->setChild(id,child);
      m_rootlist.push_back(std::make_pair(1,orig));
    } else { // set the first node as right child to node 0 for multiple trees
      std::pair<double, Node*> id0pair = m_rootlist.back();
      (id0pair.second)->setChild(id,child);
    }
  } else {
    Node* parent = find(id/2);
    parent->setChild(id,child);
  }
}

void DecisionTree::addTree(DecisionTree * tree)
{
  if( m_title != tree->title()) {
    throw std::runtime_error("DecisionTree::addTree - merging trees of different flavours");
  } else {
    m_rootlist.insert(m_rootlist.end(),tree->m_rootlist.begin(),tree->m_rootlist.end());
  }
}

void DecisionTree::printNode(std::ostream& out , const DecisionTree::Node * node, unsigned int id)const
{
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
