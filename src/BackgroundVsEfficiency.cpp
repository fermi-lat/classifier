/** @file BackgroundVsEfficiency.cpp
@brief implementation of class BackgroundVsEfficiency

$Header: /nfs/slac/g/glast/ground/cvs/classifier/src/BackgroundVsEfficiency.cpp,v 1.2 2005/07/30 00:08:56 burnett Exp $
*/


#include "classifier/BackgroundVsEfficiency.h"
#include <iostream>
#include <iomanip>
#include <cmath>

//  bin size, assuming 0-1 range.
double BackgroundVsEfficiency::s_binsize=0.01;

BackgroundVsEfficiency::BackgroundVsEfficiency(const Classifier& classify)
: m_total_bkg(0)
, m_total_sig(0)
{
    // fill a map from the leaf nodes
    std::map<double, double> probmap;
    classify.purityMap(probmap);
    // bin it.
    for( std::map<double,double>::const_iterator m = probmap.begin(); m!=probmap.end(); ++m){
        double purity = m->first, 
            wt = m->second;
        add(purity, wt*purity, wt*(1-purity));
    }

    setup();
}
void BackgroundVsEfficiency::setup()
{
    // create auxilliary map with efficiency and background for cut on purity
    double efficiency = 1.0, cum_bkg=m_total_bkg; // initial point is zero
    double inverse_variance=0; // accumulate for measure of error in signal
    for( MapPair::const_iterator m = m_probmap.begin(); m!=m_probmap.end(); ++m){
        double prob = m->first, 
            signal = m->second.first,
            background = m->second.second;
        efficiency -= signal/m_total_sig;
        if(efficiency <0) efficiency=0;
        cum_bkg -= background;
        m_auxmap[prob] = std::make_pair(efficiency, cum_bkg/m_total_bkg);
        m_effmap[efficiency] = cum_bkg/m_total_bkg;
        if( signal+background!=0) {
            inverse_variance += signal*signal/(signal+background);
        }else{
            std::cerr << "zero  background, prob, signal = " << prob << ", " << signal << std::endl;
        }
    }
    m_sigma = sqrt(m_total_sig/inverse_variance);
    m_setup=true;
}
BackgroundVsEfficiency::BackgroundVsEfficiency(
    const DecisionTree& dtree, 
    const Classifier::Table & data, int max_tree)
: m_total_bkg(0)
, m_total_sig(0)
{
    for( Classifier::Table::const_iterator i=data.begin(); i!=data.end(); ++i)  {
        const Classifier::Record& r = *i;
        double purity = dtree(r,max_tree); // the predicted purity using the decision tree
        add(purity, r.weight(true), r.weight(false));
    }
    setup();
}

BackgroundVsEfficiency::BackgroundVsEfficiency()
: m_total_bkg(0)
, m_total_sig(0)
, m_setup(false)
{}

void BackgroundVsEfficiency::add(double prob, double goodwt, double badwt)
{
    // bin the prob
    double bin = (floor(prob/s_binsize)+0.5)*s_binsize;
    if(goodwt>0) m_probmap[bin].first+=goodwt;
    if(badwt>0)  m_probmap[bin].second+=badwt;
    m_total_sig += goodwt;
    m_total_bkg += badwt;
}


double BackgroundVsEfficiency::operator()(double efficiency_cut ) const
{
    if(!m_setup) const_cast<BackgroundVsEfficiency*>(this)->setup();
    
    std::map<double,double>::const_iterator m =m_effmap.lower_bound(efficiency_cut);
    return  m!=m_effmap.end()  ? m->second : 1.0;
}


void BackgroundVsEfficiency::print(std::ostream& log, std::string label){
    if(!m_setup) setup();

    log << "Purity map ";
    if(  !label.empty() ) log << label;
    log << std::endl;
    log << "purity\tweight\teff\tcum_bkg\n";
    log << "0\t0\t1\t" <<1.0 << std::endl;
    MapPair::const_iterator m = m_probmap.begin();
    for( ; m!=m_probmap.end(); ++m){
        double prob = m->first, 
            weight = m->second.first + m->second.second,
            efficiency = m_auxmap[prob].first,
            cum_bkg = m_auxmap[prob].second;
        log << prob<< "\t" << std::setprecision(4) << weight << "\t" << efficiency << "\t" << cum_bkg
            << std::endl;
    }
}

