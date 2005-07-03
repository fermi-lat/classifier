/** @file AdaBoost.cpp
    @brief implementation of AdaBoost

    $Header: /cvsroot/d0cvs/classifier/src/AdaBoost.cpp,v 1.5 2005/03/22 23:10:37 yann Exp $
*/
#include "classifier/AdaBoost.h"
#include "classifier/Classifier.h"

#include <stdexcept>
#include <cmath>


AdaBoost::AdaBoost(Classifier::Table& data, double beta)
: m_data(data)
, m_beta(beta)
{


}

double AdaBoost::operator()(const Classifier& tree)
{

    static double purity=0.5; 
    double err = tree.error(m_data, purity),
        factor = exp( m_beta * log((1-err)/err));
    double sumwts=0;
    for( Classifier::Table::iterator i=m_data.begin(); i!= m_data.end(); ++i){
        Classifier::Record& rec = *i;
        bool type = rec.signal(); // true if signal
	//        double wt = rec.weight();
        bool classify = tree.probability(rec) > purity;
        if( type != classify) {
            rec.reweight(factor);
        }
        sumwts += rec.weight();
    }
    // now renormalize (?)
    double invsum = 1.0/sumwts;
    for( Classifier::Table::iterator i=m_data.begin(); i!= m_data.end(); ++i){
        Classifier::Record& rec = *i;
        rec.reweight(invsum);
    }
    return factor;
}

AdaBoost::~AdaBoost()
{ 
}


