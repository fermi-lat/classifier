/** @file BackgroundVsEfficiency.h
@brief declaration of the class BackgroundVsEfficiency

$Header: /cvsroot/d0cvs/classifier/classifier/BackgroundVsEfficiency.h,v 1.6 2005/04/04 21:30:09 burnett Exp $
*/

#ifndef BackgroundVsEfficiency_h
#define BackgroundVsEfficiency_h

#include "classifier/Classifier.h"
/**
    @class BackgroundVsEfficiency 
    A functor class to measure the estimated background for a given signal efficiency

*/
class BackgroundVsEfficiency
{
public:
    /**
    @param classify reference to the Classifier object to analyze
    */
    BackgroundVsEfficiency(const Classifier& classify);

    /**@brief load from a Decision tree, and a data set to apply to it
    @param max_tree [0] if non zero, maximum number of trees
    */
    BackgroundVsEfficiency(const DecisionTree& dtree, const Classifier::Table & data, int max_tree=0);

    /**@brief default constructor if use the add function
    */
    BackgroundVsEfficiency();

    /**@brief add an entry, intended for evaluating external correlation
    @param prob -- the predicted probability for this entry to be goo
    @param goodwt -- actual good (=1 if not weighted)
    @param badwt  -- actual bad (=1 if not weighted)
    */
    void add(double prob, double goodwt, double badwt);

    /** overloaded version: if good is true, add one to good, otherwise to bad 
    */
    void add(double prob, bool good){
        add(prob, good? 1:0, good? 0:1);
    }
    /**
        @return the background for a given threshold on the efficiency

    */
    double operator()(double efficiency_cut) const;

    /** @brief print the table to the stream
    */
    void print(std::ostream& log=std::cout);

    /** The rms for the signal measurement for one event, assuming inormalized to total background
    */
    double sigma()const{return m_sigma;}

private:
    void setup();
    typedef std::map<double, std::pair<double, double> > MapPair;
//    const Classifier& m_classifier;
    std::map<double, std::pair<double, double> > m_probmap;
    std::map<double, std::pair<double, double> > m_auxmap;
    std::map<double, double> m_effmap;
    double m_total_bkg, m_total_sig;
    bool m_setup;
    double m_sigma; ///< predicted error in measuring the signal
    static double s_binsize;
};

#endif

