/** @file Trainer.h
@brief declaration of the class Trainer

@author T.Burnett
$Header: /cvsroot/d0cvs/classifier/classifier/Trainer.h,v 1.5 2005/03/31 00:39:18 burnett Exp $
*/
#ifndef Trainer_h
#define Trainer_h

#include <string>
#include <iostream>
#include <vector>

class TrainingInfo;
class DecisionTree;
class BackgroundVsEfficiency;
#include "classifier/RootLoader.h"


/** @class Trainer
@brief manage training of one case.

*/
class Trainer {
public:
    /** @brief 
    @param info Get all the training parameters from this guy
    @param mylog stream to write log info to
    @param trainingset Subset of training data to use
    @param evaluationset Subset of  data to use for evaluation

    */
    Trainer( const TrainingInfo& info, std::ostream& mylog, 
        RootLoader::Subset trainingset=RootLoader::EVEN, 
        RootLoader::Subset evaluationset=RootLoader::ODD);

    void evaluate(  RootLoader& loader, RootLoader::Subset set=RootLoader::EVEN);
    void summarize_setup(std::ostream & out);

    /** @param efficiency minimum efficiency required
    @return the corresponding background ratio
    */

    double background(double efficiency)const;

    ~Trainer();
  
    double total(bool signal){ return  signal? m_signal_total:m_bkgnd_total;}

    ///get variable ratings compiled by the trainer
     const std::vector<double>& variableRatings() const;

     /// measured resolution 
     double sigma()const{return m_sigma;}

     /// control boosting: set >0 for number of boosts
     static int s_boost; 

private:
    const TrainingInfo& m_info;
    std::ostream& log();
    void setLog(std::ostream * log);
    void setLog(std::string name);
    std::ostream* m_log;
    DecisionTree* m_dtree;
    BackgroundVsEfficiency* m_eff;
    double m_signal_total;
    double m_bkgnd_total;
    std::vector<double> m_ratings; ///> list of ratings
    double m_sigma; // measured resolution
};

#endif

