/** @file  AdaBoost.h
    @brief declaration of class AdaBoost

    $Header: /nfs/slac/g/glast/ground/cvs/classifier/classifier/AdaBoost.h,v 1.1.1.1 2005/07/03 21:31:35 burnett Exp $

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

    static inline void set_purity (double p)
    {
        s_purity = p;
    }

private:
    Classifier::Table& m_data;
    double m_beta;
	static double s_purity;
};
#endif

