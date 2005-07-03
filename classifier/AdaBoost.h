/** @file  AdaBoost.h
    @brief declaration of class AdaBoost

    $Header: /cvsroot/d0cvs/classifier/classifier/AdaBoost.h,v 1.5 2005/03/22 23:10:36 yann Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef AdaBoost_h
#define AdaBoost_h
#include "classifier/Classifier.h"

/** @class AdaBoost
@brief Implement the AdaBoost boosting algorithm


*/
class AdaBoost {
public:

    /** @brief constructor 
    */
    AdaBoost(Classifier::Table& data, double beta=0.5);
    double operator()(const Classifier& tree);
    Classifier::Table& data() {return m_data;};
    ~AdaBoost();

private:
    Classifier::Table& m_data;
    double m_beta;
};
#endif

