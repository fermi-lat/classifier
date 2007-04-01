/** @file RootTuple.cpp
@brief implementation of RootTuple, RootTuple::Entry

$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/RootTuple.cpp,v 1.3 2005/07/30 00:07:16 burnett Exp $
*/
#include "classifier/RootTuple.h"
#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TSystem.h"

#include <iostream>
#include <stdexcept>
#ifdef WIN32
#include <float.h> // used to check for NaN
#else
#include <cmath>
#endif

namespace {

    std::string strip(std::string input)
    {
        size_t i = input.find_first_not_of(" "),
            j = input.substr(i).find_first_of(" ");
        return j==std::string::npos? input.substr(i) : input.substr(i, j-i+1);
    }
      
    // parse a comma-delimeted list into a vector of strings
    void parser(const std::string& input, std::vector<std::string>& output)
    {
        size_t i=0 ;
        for(; i != std::string::npos ; ){
            size_t j = input.substr(i).find_first_of(",");
            if( j==std::string::npos ){
                output.push_back(strip(input.substr(i)));
                break;
            } else{
                output.push_back(strip(input.substr(i, j)));
                i += j+1;
            }
        }
    }

    bool isFinite(double val) {
        using namespace std; // should allow either std::isfinite or ::isfinite
#ifdef WIN32 
        return (_finite(val)!=0);  // Win32 call available in float.h
#else
        return (isfinite(val)!=0); // gcc call available in math.h 
#endif
    }

}// anon namespace

/** @class RootTuple::Entry
@brief nested class is a functor to return the current value of an entry in the tree 
*/
class RootTuple::Entry {
public:
    Entry(TTree* tree, const std::string &name) {
      m_leaf = tree->GetLeaf(name.c_str());
      if( 0==m_leaf){
	m_leaf = tree->GetLeaf(("_" + name).c_str());
	if( 0==m_leaf) {
	  tree->Print();
	  throw std::invalid_argument(std::string("RootTuple::Entry: could not find leaf ")+name);
	}
      }
      // make sure the branch containing this leaf is activated
      TBranch* b = m_leaf->GetBranch();
      tree->SetBranchStatus(b->GetName(),1);
    }
    double operator()()const{return m_leaf->GetValue();}
    operator double ()const{return m_leaf->GetValue();}
private:
    TLeaf* m_leaf;
};

RootTuple::RootTuple(std::string root_file, std::string tree_name)
: m_total_size(0)
{
    add(root_file, tree_name);
}

RootTuple::RootTuple(std::vector<std::string> root_files, std::string tree_name)
: m_total_size(0)
{
#ifdef WIN32 // ROOT work-around
    static bool first=true;
    if(first){ first=false;
    int ret=gSystem->Load("libTree");
    if( ret==1) TTree dummy;
    }
#endif

    std::vector<std::string>::iterator fit = root_files.begin(); 
    for(; fit!= root_files.end(); ++fit) add(*fit, tree_name);
}

inline static void do_load (void)
{
    static bool first = true;
    if( first) {
      gSystem->Load("libTree");
      first=false;
    }
}

void RootTuple::add(std::string root_file, std::string tree_name)
{
  do_load();
    std::cout << "Opening " << root_file << std::endl;
    TFile* f = new TFile(root_file.c_str(), "READ");
    if( ! f->IsOpen()) throw std::invalid_argument(std::string(
        "RootTuple: could not open file ")+root_file);
    m_files.push_back(f);
    TTree* t = static_cast<TTree*>(f->Get(tree_name.c_str()));
    add (t);
}

RootTuple::RootTuple (TTree *t)
: m_total_size(0)
{
  do_load();
  add (t);
}

void RootTuple::add(TTree*t)
{
  m_trees.push_back( t);
  m_sizes.push_back( m_total_size += static_cast<size_t>(t->GetEntries()));
}

RootTuple::~RootTuple()
{ 
    for( std::vector<TFile*>::iterator fit = m_files.begin();
        fit != m_files.end(); ++fit)   delete *fit;
#if 0 // todo: fix this
        for( EntryList::iterator it=m_entries.begin(); it!=m_entries.end(); ++it){
            delete *it;
        }
#endif
}

void RootTuple::selectColumns(const std::vector<std::string>& names, bool weighted)
{
    if( names.empty() ) throw std::invalid_argument(
        "RootTuple::selectColumns: no names in selection");
    m_weighted = weighted;
    m_entries.clear();
    for( std::vector<TTree*>::iterator tit = m_trees.begin(); tit!=m_trees.end(); ++tit){
        TTree* tree = *tit;
        tree->SetBranchStatus("*",0);  // default all branches off -- Entry will active what it needs
        m_entries.push_back(EntryList());
        std::vector<std::string>::const_iterator nit = names.begin();
        for( ; nit!=names.end(); ++nit){
            m_entries.back().push_back(new Entry( tree, *nit));
        }
    }
}

void RootTuple::selectColumns(const std::string& names)
{
    std::vector<std::string> namevec;
    parser(names, namevec);
    selectColumns(namevec, false);
}

/// fill the vector from the selected row
void RootTuple::fill_row(int event, std::vector<float>& row )const
{
    int i=0;
    while( static_cast<size_t>(event)> m_sizes[i])++i;
    if( i>0) event -= m_sizes[i-1];
    m_trees[i]->GetEvent(event);
    row.clear();
    EntryList::const_iterator et=m_entries[i].begin();
    for(; et !=m_entries[i].end(); ++et){
        const Entry& e = **et;
        float v = e();
        if( !isFinite(v) ){
            throw std::runtime_error("RootTuple::fill_row: Nan found in input");

        }
        row.push_back(v );
    }
}

size_t RootTuple::size()const
{ 
    return m_total_size;
}

std::vector<float> RootTuple::operator[](unsigned int i)const
{
    return *Iterator(this,i);

}
