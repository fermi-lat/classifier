/** @file Classifier.h
    @brief declaration of the class Classsifier and various nested helper classes

    $Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/Classifier.h,v 1.3 2005/10/20 14:22:45 burnett Exp $
*/

#ifndef Classifier_h
#define Classifier_h
#include "classifier/DecisionTree.h"

#include <string>
#include <iostream>
#include <vector>
#include <map>

/** @class Classifier
    @brief Manage a classification tree. Nested classes are:
    - Record definition of the record of data
    - Table
    - Node 
    - SplitCriterion abstract base class for splitting
    - Visitor abstract base class allowing visitors to traverse the tree (Visitor Pattern)

*/
class Classifier {
public:
    typedef std::vector<std::string> StringList;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class Record
    @brief entry in a Classifier::Table
    */
    class Record : public std::vector<float> {
    public:

        /** ctor
        @param signal true if this is a signal record
        @param data  a list of data values to copy to the record: 
        the first must be the weight it weights are in the data
        */
        Record( bool signal, const std::vector<float>& data);

        /** ctor
        @param signal true if this is a signal record
        @param begin  begin iterator of a list of data values to copy to the record: 
        the first must be the weight it weights are in the data
        @param end end iterator
        */
        Record( bool signal, std::vector<float>::const_iterator begin,
            std::vector<float>::const_iterator end);


        operator double()const{ return (*this)[s_sort_column]; }
        double operator()()const{return (*this)[s_sort_column];  }

        bool operator<(const Record& other)const{ return (*this)[s_sort_column]< other[s_sort_column];  }
        double weight(bool signal)const{return signal? m_sigwt: m_bkgwt;}
        float& weight(bool signal){return signal? m_sigwt: m_bkgwt;}
        double weight()const{return m_sigwt+m_bkgwt;}   
        float& weight(){return m_sigwt>0? m_sigwt : m_bkgwt;}
        double cum_weight(bool signal)const{return signal? m_cumsig: m_cumbkg;}

        /** @ brief reweight by applying the factor. 
            @param factor multiplicative factor for weight, to implement boost
        */
        void reweight(double factor){ m_sigwt*= factor; m_bkgwt*=factor;}
        bool signal()const{return m_sigwt>0;}

        /// set the cumulative weigths, part of sorting
        void setCumWeights(double sig, double bkg){m_cumsig=sig, m_cumbkg=bkg;}
        /** set static variables 
            @param names vector of column names -- first name is weight if weighted
            @param use_weights  set true to interpret first column as a weight
        */
        static void setup( const std::vector<std::string>& names, bool use_weights);
		static void setup(){s_size=0;
		s_use_weights=false;}
        static const std::string & columnName(int i);

        /// identify the column to use for ordering
        static int s_sort_column;

        /// global for the width of the columns (excluding the weight)
        static int size(){return s_size;}

     private:
        float m_sigwt;
        float m_bkgwt;
        float m_cumsig;
        float m_cumbkg;
    private:
        static std::vector<std::string> s_column_names;
        static bool s_use_weights;
        static int s_size;
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        /** @class Table is a vector of Records
  
      */

    class Table : public std::vector<Record> {
    public:
        /// normalize weights to given signal, background totals
        void normalize(double signal=1.0, double background=1.0);
    };

    class Node; // forward declaration
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class Visitor
        @brief abstract base class for Visitor Pattern . See Node::accept()
    */
    class Visitor  {
    public:
        virtual void visit(const Node& node)=0;
    protected:
        Visitor(){};
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class SplitCriterion
    @brief  abstract base class for split criterion strategy class. It is a functor of two args, the size
    of the signal and background

    */
    class SplitCriterion {
    public:
        /** @param signal total signal
        @param background 
        @return value of the criterion, which is to be minimized
        */
       virtual double operator()(double signal, double background)const=0;
         virtual std::string name()const=0;
    private:
    };
    // Forward declaration of SplitCriterion 
    class Gini ; //: public SplitCriterion;
    class Entropy; // : public SplitCriterioin;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class Node
    @brief a subset of the full table (defined by iterators), with pointers to right and left child nodes

    */
    
    class Node {
    public:
    /**  create a Node from Table iterators
        @param begin beginning of range of Table entries
        @param end   end of range
        @param id [1] identifier: 1 is root, left is 2*parent id, right is left+1.

        A particular element of the Record sequence is expected to be used for sorting and 
        optimizing. It is set by the sort() function
    */
        Node(Table::iterator begin, Table::iterator end, int id=1);

        ~Node();

        /**  @return  Iterator corresponding to the threshold value */
        Table::iterator lower_bound( double threshold);
        /**  \return  Iterator corresponding to the threshold value 
        */
        Table::const_iterator lower_bound(double threshold)const;

        /** sort and add cumulative weights;
        @param sort_column the column to specify for sorting @return gini value
        */
        double sort(int sort_column=0);

        double minimize_gini(double a, double b, int iteration=0);

        double minimize_gini();
        double gini(const Record& rec )const;
        double gini(double value)const;

        /** @brief split the node, forming two children nodes, 
           according to the optimization scheme

        @param recursive if true, continue down to leaves
        */
        void split( bool recursive=true);

        /// prune the tree
        void prune();
            
        /// return cumulative event weight betweent the two limits
        double weight(double a, double b, bool signal)const;

        double totalWeight()const{return m_signal+m_background;}
        Node & left()const{return *m_left;}
        Node & right()const{return *m_right;}

        size_t size()const{return m_end-m_begin;}
        Table::iterator begin(){return m_begin;}
        Table::iterator end(){return m_end;}
        Table::const_iterator begin()const{return m_begin;}
        Table::const_iterator end()const{return m_end;}

        /// apply selection if not a leaf
        const Node& select(const std::vector<float>& event)const; 

        int index()const{return m_split_index;}
        double total_gini()const{return m_gini;}
        double split_gini()const{return m_left==0? m_gini: m_left->total_gini() + m_right->total_gini();}
        int id() const {return m_id;}

        double purity()const { return m_signal/(m_signal + m_background);}
        double value()const { return m_split_value;}
        bool isLeaf()const { return m_left==0;}

        /// accept a Visitor object, call its visit method, pass on the children
	 void accept(Visitor& v)const;
	 void accept(Classifier::Visitor& v);
 
        /** evaluate training error
        @param purity [0.5] threshold to use to classify signal or background
        @return the error associated with the training
        */
        double error(double purity=0.5)const;
        
        static const int s_minsize=100;
        // keep track of the total number of nodes, leaves
        static int s_nodes;
        static int s_leaves;

        /// global to limit the gini improvement allowed, avoiding many insignificant cuts.
        static double s_improvement_minimum;


    private:
        /// the id: 1 for root, 2*parent for left, 2*parent+1 for right
        int m_id;
        Classifier::Table::iterator m_begin;
        Classifier::Table::iterator m_end;
        /// variable index for the split
        int m_split_index;
        /// value for the split: left branch is less than this
        double m_split_value;

        double m_gini;
        /// total of signal and background weights
        double m_signal, m_background;
        /// pointers to child nodes (zero if this is a leaf node)
        Node* m_left;
        Node* m_right;

    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Classifier(Classifier::Table& data);

    /// ctor that also sets up the names to be used
    Classifier(Classifier::Table& data,
       const std::vector<std::string>& names, bool event_weights=false);
    ~Classifier();

    /** create a classification tree from the daata
    @param recursive [true] allow to only make a top-level split if false
    */
    void makeTree(bool recursive=true);
    /// make a tab-delimited table of the  tree
    void printTree( std::ostream & out= std::cout);

    /// generate cross-tab of signal vs background sepation.
    void crossTab(const Classifier::Table& data, std::ostream & out= std::cout);

    /** @param event vector of values corresponding to the training sample
    @return probability for that the given event is signal
    -- actually the purity of the leaf node for this set of parameters
    */
    double probability(const std::vector<float>& event)const;

    /// fill a purity map: pair(purity, weight)  from the leaf node
    void purityMap(std::map<double, double>& map)const;

    /// rate the variables used for making the currrent tree
    void rateVariables(std::vector<double>& ratings)const;

    /// print a summary of the variables and their ratings
    void printVariables(std::ostream& log= std::cout)const;

    /// start a visitor at the root of the tree
    void accept(Classifier::Visitor& visitor)const;
    void accept(Classifier::Visitor& visitor);

    /// Return the error for the model, defined as the 
    double error(const Classifier::Table& data, double purity=0.5)const;

    /// create a simplified decision tree
    DecisionTree* createTree(std::string title="Decision Tree", double weight=1.);
#if 0
    static  Classifier::SplitCriterion Classifier::splitCriterion; 
#endif
    /**    @param log set the stream for logging
     */
    static void setLogStream(std::ostream& log) ;
private:
#if 1
    /// access to the root node
    Node& root() { return *m_root; }
    const Node& root()const { return *m_root; }
#endif

    Node* m_root;
};

#endif

