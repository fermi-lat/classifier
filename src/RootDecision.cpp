/** @file  RootDecision.h
    @brief implementation of class RootDecision

    @author T. Burnett
    $Header: /cvsroot/d0cvs/classifier/src/RootDecision.cpp,v 1.4 2005/02/04 12:45:27 burnett Exp $

*/

#include "classifier/RootDecision.h"

#include "classifier/DecisionTree.h"
#include "classifier/TrainingInfo.h"

#include <fstream>
#include <stdexcept>

RootDecision::RootDecision( RootTuple& tuple, 
        const std::string& treeInfoFolder, bool weighted)
        : m_tuple(&tuple)
{
    std::string filename(treeInfoFolder+"/dtree.txt");
    std::ifstream input(filename.c_str());
    if( !input.is_open()) {
        throw std::invalid_argument("RootDecision::RootDecision: could not open file '"+filename+"'");
    }
    m_dtree = new DecisionTree(input);
    TrainingInfo info(treeInfoFolder);
    if( info.vars().size() < 2) throw std::invalid_argument("RootDecision: need at least two variables");
    m_tuple->selectColumns(info.vars(), weighted );
}

RootDecision::~ RootDecision(){
    delete m_dtree;
}

RootDecision::Iterator::Iterator(const RootDecision* rd, bool end)
: m_dtree(rd->dtree())
, m_root_iterator(rd->tuple(), end)
, m_weighted(rd->tuple()->weighted())
{

}
RootDecision::Iterator::Iterator(const RootDecision* rd, unsigned int index)
: m_dtree(rd->dtree())
, m_root_iterator(rd->tuple(), index)
, m_weighted(rd->tuple()->weighted())
{

}
std::pair<double, double>  RootDecision::Iterator::operator*()
{
    std::vector<float> row = *m_root_iterator;

    std::vector<float>::iterator icol=row.begin();
    // if the tuple has weighted events, the first col is the weigth
    double weight= (m_weighted)? *icol++ : 1.0;
    std::vector<float> t(icol, row.end());
    return std::make_pair( (*m_dtree)(t), weight);
}

RootDecision::Iterator& RootDecision::Iterator::operator++()
{
    ++m_root_iterator;
    return *this;
}

RootDecision::Iterator::operator unsigned int()
{
    return m_root_iterator;
}
