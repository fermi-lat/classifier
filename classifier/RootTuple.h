/** @file  RootTuple.h
    @brief declaration of class RootTuple

    @author T. Burnett
    $Header: /cvsroot/d0cvs/classifier/classifier/RootTuple.h,v 1.6 2005/02/08 21:16:59 burnett Exp $
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef RootTuple_h
#define RootTuple_h
// forward declarations
class TTree;
class TFile;
class TBranch;
class TLeaf;
class RootTree;

#include <string>
#include <vector>

/** @class RootTuple
@brief Adapt a ROOT TTree, or set of files, to make it like an STL container

Example of usage
@verbatim
   RootTuple t("myfile.root");
   t.selectColumns("x,y,z");
   for( RootTuple::const_iterator it=t.begin(); it!=t.end(); ++it){
     std::vector<float>& row = *it;
     double x = row[0];
   }
@endverbatim

*/
class RootTuple {
public:

    /** @brief constructor opens the file constaining a tree
        @param filename name of a ROOT file
        @param tree_name The TTree to use - if not present, use the first found

        Throws std::invalid_argument exception if cannot open the file, or find the TTree
    */
    RootTuple(std::string root_file, std::string tree_name="");

   /** @brief constructor opens a set of  files each with a tree to concatentate
        @param root_files list of root files
        @param tree_name The TTree to use - if not present, use the first found

        Throws std::invalid_argument exception if cannot open the file, or find the TTree
    */
    RootTuple(std::vector<std::string> root_files, std::string tree_name="");

    /** special short cut to just apply use an existing TTree 
    
    */
    RootTuple (TTree *tree);

    /** dtor cleans up
    */
    ~RootTuple();

    /** @brief select a subset of columns for the Iterator
        @param names vector of TLeaf names.
    */
    void selectColumns(const std::vector<std::string>& names, bool weighted);


    /** @brief select a subset of columns for the Iterator
        @param names comma-separated list of TLeaf names.
    */
    void selectColumns(const std::string& names);

    /** @brief fill the vector from the selected row
        @param event Event number
        @param row Reference to vector of float to return 
        which will be filled from the given event, 
        in the order selected by the selectColumns list

    */
    void fill_row(int event, std::vector<float>& row )const;

    // forward declaration of nested class representing an entry
    class Entry;

    typedef std::vector<RootTuple::Entry*> EntryList;

    /** @class Iterator 
        @brief A forward iterator for access to the combined rows
    */
    class Iterator {
    public:
        Iterator(const RootTuple * tuple, bool end=false)
            :m_tuple(tuple), m_pos( end? tuple->size() : 0 )
        {}
        Iterator(const RootTuple * tuple, unsigned int total)
            :m_tuple(tuple), m_pos( std::min( tuple->size(), total) )
        {}

        Iterator& operator++(){m_pos++; return *this;}
        const std::vector<float> & operator*()
        {
            m_tuple->fill_row(m_pos, m_row );
            return m_row;
        }
        /// for comparison
        operator unsigned int(){return m_pos;}

    private:
        const RootTuple* m_tuple;
        size_t m_pos;
        std::vector<float> m_row;
    };
    typedef RootTuple::Iterator const_iterator;
    ///@brief the begin iterator, set to first entry
    Iterator begin(){return  Iterator(this);}
    ///@brief the end iterator, set to just past the last entry
    Iterator end(){return  Iterator(this,true);}

    ///@brief iterator set to given entry number
    Iterator iterator(unsigned int i)const{return Iterator(this, i);}

    /** @brief access to a row by index

    */
    std::vector<float> operator[](unsigned int i)const;
        
    size_t size()const;
    bool weighted()const{return m_weighted;}

private:
    void add(std::string root_file, std::string tree_name);
  void add (TTree *t);

    std::vector<TFile*> m_files; ///> list of open files
    std::vector<TTree*> m_trees; ///> list of the trees
    std::vector <size_t> m_sizes; ///> list of cumulative size
    size_t m_total_size;
    /// list of the selected entries associated with this tuple
    std::vector<EntryList> m_entries;
    int m_start, m_increment; ///> Increment to allow every-other
    bool m_weighted;
};
#endif

