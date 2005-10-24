/** @file AdaBoost.cpp
    @brief implementation of AdaBoost

    $Header: /nfs/slac/g/glast/ground/cvs/classifier/src/AdaBoost.cpp,v 1.1.1.1 2005/07/03 21:31:35 burnett Exp $
*/
#include "classifier/AdaBoost.h"
#include "classifier/Classifier.h"

#include <stdexcept>
#include <cmath>

double AdaBoost::s_purity = 0.5;



AdaBoost::AdaBoost(Classifier::Table& data, double beta)
: m_data(data)
, m_beta(beta)
{


}

double AdaBoost::operator()(const Classifier& tree)
{

    double err = tree.error(m_data, s_purity),
        factor = exp( m_beta * log((1-err)/err));
    double sumwts=0;
    for( Classifier::Table::iterator i=m_data.begin(); i!= m_data.end(); ++i){
        Classifier::Record& rec = *i;
        bool type = rec.signal(); // true if signal
	//        double wt = rec.weight();
        bool classify = tree.probability(rec) > s_purity;
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


