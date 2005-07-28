/** @file Classifier.cpp
@brief implementation of Classifier, Classifier::Node, Classifier::Record

$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/Classifier.cpp,v 1.1.1.1 2005/07/03 21:31:35 burnett Exp $
*/

#include "classifier/Classifier.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cmath>
//#define verbose
int Classifier::Record::s_sort_column=0;
int Classifier::Record::s_size=0;
std::vector<std::string> Classifier::Record::s_column_names;
bool Classifier::Record::s_use_weights=false;

namespace {
#ifdef WIN32
#include <float.h> // used to check for NaN
#else
#include <cmath>
#endif

    bool isFinite(double val) {
        using namespace std; // should allow either std::isfinite or ::isfinite
#ifdef WIN32 
        return (_finite(val)!=0);  // Win32 call available in float.h
#else
        return (isfinite(val)!=0); // gcc call available in math.h 
#endif
    }
} // anom namespace


int Classifier::Node::s_nodes=0;
int Classifier::Node::s_leaves=0;
// select the criterion
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**  @class Gini  
     @brief subclass of Classifier::SplitCriterion
*/
class Gini : public Classifier::SplitCriterion {
public:
    virtual double operator()(double signal, double background)const
    {
        double total=signal+background;
        return total>0? 2*signal*background/total : 0;
    };
    virtual std::string name()const{return "Gini";}
private:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**  @class Entropy  
     @brief subclass of Classifier::SplitCriterion
*/
class Entropy : public Classifier::SplitCriterion {
public:
    virtual double operator()(double signal, double background)const
    {
        double p = signal/(signal+background);
        return (p>0 && p<1)? -signal*log(p) - background*log(1-p) : 0;
    };
    virtual std::string name()const{return "Entropy";}
private:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// used below -- need to make flexible
const Classifier::SplitCriterion& splitCriterion = Gini();
//const Classifier::SplitCriterion& splitCriterion = Entropy();

namespace {

    std::ostream* thelog = &std::cout;
    std::ostream & logstream(){ return  *thelog ;}
}
void Classifier::Record::setup( const std::vector<std::string>& names, bool use_weights)
{
    s_use_weights = use_weights;
    s_column_names = names;
    s_size = names.size(); 
    if( use_weights) s_size--;
}
const std::string & Classifier::Record::columnName(int i){
    return s_column_names[ s_use_weights? i+1:i];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Record::Record( bool signal, const std::vector<float>& data)
{
    std::vector<float>::const_iterator id = data.begin();
    // if using weights, get it from the first column, Otherwise 1.0   
    double wt = s_use_weights ?  *id++: 1;
   
    m_sigwt = signal? wt : 0;
    m_bkgwt = signal? 0 : wt;
    int size = data.end()-id; // should be current size
    if( s_size==0)   s_size = size;
    if (size != s_size) throw std::invalid_argument("Record::Record: size of data record changed!");
    for(; id!=data.end(); ++id) push_back(*id);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Record::Record( bool signal, std::vector<float>::const_iterator id,
                           std::vector<float>::const_iterator end)
{
    // if using weights, get it from the first column, Otherwise 1.0   
    double wt = s_use_weights ?  *id++: 1;
   
    m_sigwt = signal? wt : 0;
    m_bkgwt = signal? 0 : wt;
    int size = end-id; // should be current size
    if( s_size==0)   s_size = size;
    if (size != s_size) throw std::invalid_argument("Record::Record: size of data record changed!");
    for(; id!=end; ++id) push_back(*id);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::Table::normalize(double signal, double background)
{
    double tsig=0, tbkg = 0;
    for( iterator i = begin(); i!= end(); ++i){
        tsig += i->weight(true);
        tbkg+= i->weight(false);
    }
    if( tsig==0 || tbkg==0) throw std::runtime_error("Table::normalize: no signal or background?");
    for( iterator i = begin(); i!= end(); ++i){
        i->weight(true) *= signal/tsig;
      i->weight(false)*=background/tbkg;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Node::Node(Table::iterator begin, Table::iterator end, int id)
: m_id(id)
, m_begin(begin)
, m_end(end)
, m_split_index(-1)
, m_gini(0)
, m_left(0)
, m_right(0)

{
    // measure the Gini
    double totsig=0, totbkg=0;
    for( Table::iterator i=begin; i!=end; ++i){

        totsig += i->weight(true);
        totbkg += i->weight(false);
    }
    m_signal = totsig;
    m_background = totbkg;
    m_gini = splitCriterion(totsig,totbkg);
#ifdef verbose
    logstream() << "Created node "<< id << " with " << size() <<" records" <<std::endl;
#endif
    s_nodes++;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Node::~Node()
{
    delete m_left;
    delete m_right;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::Node::accept(Classifier::Visitor& v)const
{
     v.visit(*this);
     if( ! isLeaf() ) {
        right().accept(v);
        left().accept(v);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::Node::accept(Classifier::Visitor& v)
{
     v.visit(*this);
     if( ! isLeaf() ) {
        right().accept(v);
        left().accept(v);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::accept(Classifier::Visitor& visitor)const
{
    root().accept(visitor);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::accept(Classifier::Visitor& visitor)
{
    root().accept(visitor);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Table::iterator Classifier::Node::lower_bound( double threshold)
{
    Table::iterator it= std::lower_bound(begin(), end(),threshold);
    if( it == end() ) --it;
    return it;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Classifier::Table::const_iterator Classifier::Node::lower_bound(double threshold)const
{
    Table::const_iterator it= std::lower_bound(begin(), end(),threshold); 
    if( it == end() ) --it;
    return it;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::sort(int sort_column)
{
    // set static for sorting, analysis
    Record::s_sort_column = m_split_index=sort_column;
    std::sort(begin(), end());
    double lastsig=0, lastbkg=0;
    for( Table::iterator i=begin(); i!=end(); ++i){

       lastsig += i->weight(true);
       lastbkg += i->weight(false);
        i->setCumWeights(lastsig, lastbkg);
    }
    // after sorting, set the gini.
    m_signal = lastsig;
    m_background = lastbkg;
    return m_gini = splitCriterion(lastsig, lastbkg);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::minimize_gini(double a, double b, int iteration)
{
    static int max_iter=4, divide=8;
    double t = 1e9, u =a, range = b-a;
    for( double x = a; x<=b; x+=range/divide){
        double g = gini(x);
        if( g < t ){
            u =x;
            t =g;
        }
    }
    //log << " iteration, min gini, x = " << iteration << " " << u << " "<< t << std::endl;
    if(iteration> max_iter) return u;

    return minimize_gini(u- range/8, u+range/8, iteration+1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::minimize_gini()
{
    int s = size();
    const Record& first = *(begin()+s/8);
    const Record& last =  *(end()-s/8);
    double a = first(), b=last(), range = b-a;
    if( ! ::isFinite(range) ) {
        throw std::runtime_error("Classifier::Node::minimize_gini: NaN found, quitting");
    }

    if(range==0) return gini(a);
    double t = 1e9, u =a;
    for( double x = a; x<=b; x+=range/8){
        double g = gini(x);
        if( g<t ){
            u =x;
            t =g;
        }
    }
    return minimize_gini(u- range/8, u+range/8, 0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::gini(const Record& rec )const
{
    double 
        sig_l = rec.cum_weight(true),
        sig_r = m_signal-sig_l,
        bkg_l = rec.cum_weight(false),
        bkg_r = m_background - bkg_l;
    if( sig_l + bkg_l == 0 || sig_r+bkg_r==0){
        return m_gini; // should prevent being considered
    }
    return splitCriterion(sig_l, bkg_l) + splitCriterion(sig_r,bkg_r);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::gini(double value)const
{
    Table::const_iterator entry= lower_bound(value);
    const Record& rec =  *entry;
    return gini(rec);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::Node::split( bool recursive)
{
    int nvar = Record::size();
    if( nvar==0) throw std::invalid_argument("No variables to split");
    if( size()< (size_t) s_minsize ) return;

    double best=1e9, xbest;
    int ibest=-1;

#ifdef verbose
    logstream() << "Splitting node " << id() 
        << "\n    index   improvement   at value" << std::endl;
#endif
    for(int n=0 ; n<nvar ; ++n){
        double gtot=sort(n);
        double x = minimize_gini();
        double gx = gini(x);
        if( gx < best) {xbest = x;  best= gx; ibest=n;}
#ifdef verbose
        logstream() 
            << std::setw(10) << n
            << std::setw(12) << std::setprecision(4) << gtot-gx
            << std::setw(10) << x << std::endl;
#else
        gtot=gtot; // to avoid gcc warning
#endif
    }

    sort(ibest);
    Table::iterator split_at = lower_bound(xbest);

    // if if either child is too small this is a leaf node
    int nleft = split_at-begin(), nright = end()-split_at;
    if( nleft< s_minsize || nright < s_minsize ) return;

    m_left = new Node(begin(), split_at, 2*m_id);
    m_right = new Node(split_at, end(), 2*m_id+1);
    m_split_index = ibest;
    m_split_value = xbest;
    if( recursive){
        m_left->split( true);
        m_right->split(true);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::Node::prune()
{
    /// @todo: add prune critera possibility
    delete m_left;
    delete m_right;
    m_left = m_right=0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::Node::weight(double a, double b, bool signal) const
{
    const Record& ra = *lower_bound(a);
    const Record& rb = *lower_bound(b);

    double wa = ra.cum_weight(signal),
        wb=rb.cum_weight(signal);
    return wb-wa;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const Classifier::Node& Classifier::Node::select(const std::vector<float>& event)const
{
    if( isLeaf() ) return *this;
    return event[index()]< value() ? *m_left : *m_right;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                 Classifier implementation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Classifier::Classifier(Classifier::Table& data)
{
    if( data.empty()) throw std::invalid_argument("Classifier: table is empty");
    m_root=new Classifier::Node(data.begin(), data.end() );
}

Classifier::Classifier(Classifier::Table& data, const std::vector<std::string>& names,
                       bool event_weights)
{
    if( data.empty()) throw std::invalid_argument("Classifier: table is empty");
    m_root=new Classifier::Node(data.begin(), data.end() );
    Record::setup(names, event_weights);
}

void Classifier::setLogStream(std::ostream& log) { thelog = &log;}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::makeTree(bool recursive)
{
	std::cout << "Making tree..."
			  << std::endl;
    m_root->split(recursive);
}

Classifier::~Classifier()
{
    delete m_root;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::printTree( std::ostream & out)
{
    class TreePrinter : public Classifier::Visitor {
    public:
        TreePrinter(std::ostream&out):m_out(out){}
        void visit(const Node& node)
        {    m_out << node.id() << "\t" 
            << node.size() << "\t"
            << std::setprecision(5) << node.totalWeight() << "\t"
            << node.total_gini() << "\t"
            << std::setprecision(2) << node.purity() << "\t";
        if( node.isLeaf() ){
            m_out << "(leaf)";
        }else{
            m_out << Classifier::Record::columnName(node.index())
            << " < "<< std::setprecision(4) << node.value();
        }
        m_out  << std::endl;
        }
        std::ostream& m_out;
    }printer(out);

    out << "-------------------------------\n";
    out << "--------- tree summary ----------\n"
        << "id\tentries\tweight\t"<< splitCriterion.name() << "\tpurity\tleft branch"
        << std::endl;
    accept(printer);
    out << "-------------------------------\n";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::printVariables(std::ostream& log)const
{
    std::vector<double> ratings;
    rateVariables(ratings);
    int i= 0; 
    log <<  "\nVariable summary\nName\timprovement\n" << std::setprecision(4);
    for( std::vector<double>::const_iterator rit= ratings.begin(); rit!=ratings.end(); ++rit, ++i){
        log <<Classifier::Record::columnName(i)<< "\t" << *rit << std::endl;
    }
} 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::probability(const std::vector<float>& event)const
{
    const Node* pnode = &root();
    while( !pnode->isLeaf() ){
        pnode = &pnode->select(event);
    }

    return pnode->purity();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::crossTab(const Classifier::Table& data, std::ostream & out)
{
    double t00=0, t01=0, t10=0, t11=0;
    for(Table::const_iterator ti = data.begin(); ti!= data.end(); ++ti){
        double p = probability(*ti);
        double sig = ti->weight(true), bkg = ti->weight(false);
        if( sig==0 && bkg==0 ) continue;
        if( sig> 0 ) {
            if( p > 0.5 ) t00+= sig; else t01 += sig;
        }else{
            if( p > 0.5 ) t10+= bkg; else t11 += bkg;
        }
    }
    out << "Cross tab: for predicted vs actual signal and background "
        <<"\ntype\t     predicted"
        <<"\n\tsignal\tbackgnd"<< std::endl;
    out 
        <<   "signal\t"<<std::setprecision(5) <<  t00 << "\t" << t01 << "\n" 
        <<  "backgnd\t"<< t10 << "\t" << t11 
        << "\n-------------------------------------"<<std::endl;
}
    


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::purityMap(std::map<double, double>& map )const
{
    class ProbMap : public Visitor {
    public:
        ProbMap(std::map<double, double>& map): m_map(map){}
        void visit(const Node& node)
        {       
            if( !node.isLeaf()) return;
            m_map[node.purity()] +=  node.totalWeight();
        }
        std::map<double, double>& m_map;
    } probvisit(map);
    accept(probvisit);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Classifier::rateVariables(std::vector<double>& ratings)const
{
    class RateVars : public Visitor {
    public:
        RateVars(std::vector<double>& ratings): m_ratings(ratings){}
        void visit(const Node& node)
        {
            if( node.isLeaf())return;
            m_ratings[node.index()]+= node. total_gini() - node.split_gini();
        }
        std::vector<double>& m_ratings;
    } rater(ratings);

    ratings.clear();
    ratings.resize(Record::size());
    accept(rater);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double Classifier::error(const Classifier::Table& data, double purity)const
{
    double sumwt=0, sumerr=0;
    for( Table::const_iterator i = data.begin(); i!=data.end(); ++i){
        const Record& rec = *i;
        bool type = rec.signal(); // true if signal
        double wt = rec.weight();
        bool classify = probability(rec) > purity;
        if( type != classify) {
            sumerr += wt;
        }
        sumwt += wt;
    }
    return sumerr/sumwt;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DecisionTree* Classifier::createTree(std::string title, double weight)
{
    class DumpNodes : public Classifier::Visitor {
    public:
        DumpNodes(DecisionTree* dtree, double wt): m_dtree(dtree){
	  m_dtree->addNode(0, -10, wt); //create first node with tree weight
	}
        void visit(const Node& node)
        {
            if( node.isLeaf() )     m_dtree->addNode(node.id(), -1, node.purity());
            else m_dtree->addNode(node.id(), node.index(), node.value());
        }
        DecisionTree* m_dtree;
    };
    DecisionTree* dtree = new DecisionTree(title);
    DumpNodes dumper(dtree, weight);
    accept(dumper);
    return dtree;
}


