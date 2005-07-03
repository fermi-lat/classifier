/** @file  RootDecision.h
    @brief declaration of class RootDecision

    @author T. Burnett
    $Header: /cvsroot/d0cvs/classifier/classifier/RootDecision.h,v 1.3 2005/02/04 12:56:19 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef RootDecision_h
#define RootDecision_h

#include <string>

#include "classifier/RootTuple.h"
class DecisionTree;
class TTree;

/** @class RootDecision
    @brief  adapts a root tree to a container 

    The root file(s) are accessed via a RootTuple container adapter,
    and the DecisionTree is created from the information in
    the tree information folder used in its training.

*/
class RootDecision 
{
public:
    RootDecision( RootTuple& tree, 
            const std::string& treeInfoFolder, bool weighted);

    ~RootDecision();

/** @class Iterator 
        @brief A forward iterator for access to the value

    */
    class Iterator {
    public:
        Iterator(const RootDecision * rd, bool end=false);
        Iterator(const RootDecision * rd, unsigned int index);

        Iterator& operator++();
        std::pair<double,double>  operator*();
        /// for comparison
        operator unsigned int();

    private:
        RootTuple::Iterator  m_root_iterator;
        const DecisionTree* m_dtree;
        bool m_weighted;
    };
    typedef RootDecision::Iterator const_iterator;
    Iterator begin()const{return  Iterator(this);}
    Iterator end()const {return  Iterator(this,true);}
    Iterator iterator(unsigned int index)const {return  Iterator(this,index);}

    size_t size()const{return m_tuple->size();}

    RootTuple* tuple()const{return m_tuple;}
    const DecisionTree* dtree()const{return m_dtree;}

private:
    RootTuple* m_tuple;
    const DecisionTree* m_dtree;

};
#endif
