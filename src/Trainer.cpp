/** @file Trainer.cxx
    @brief implementation of  class Trainer

    @author T. Burnett
    $Header: /cvsroot/d0cvs/classifier/src/Trainer.cpp,v 1.7 2005/03/31 00:39:19 burnett Exp $
*/

#include "classifier/Trainer.h"
#include "classifier/TrainingInfo.h"

#include "classifier/Classifier.h"
#include "classifier/BackgroundVsEfficiency.h"
#include "classifier/AdaBoost.h"
#include "classifier/DecisionTree.h"
#include <string>
#include <vector>
#include <fstream>
#include <iterator>

int Trainer::s_boost=0; // set bootsing 

Trainer::Trainer( const TrainingInfo& info, std::ostream& mylog,
                 RootLoader::Subset trainingset, 
                RootLoader::Subset evaluationset) 
        : m_info(info)
        , m_log(&mylog)
        , m_dtree(0)
        , m_eff(0)
        , m_sigma(0)
    {
        setLog(info.log());
        Classifier::setLogStream(*m_log);

        summarize_setup(std::cout);
        summarize_setup(log());

        // create data table and pass file names to it.
        RootLoader loader(info.signalFiles(), 
            info.backgroundFiles(), info.vars(), info.weighted());
        // note that we report the total
        Classifier::Table training;

        loader(training, trainingset, log());
        training.normalize(1.0,1.0);
        m_signal_total=2.* loader.total(true);
        m_bkgnd_total=2.* loader.total(false);

        // create Classifier object with the training sample
        Classifier classify(training);
        classify.makeTree(true);

        classify.printVariables(log());
#ifdef VERBOSE
        classify.printTree(log());
#endif

       //
        // Get the variable performace info
        //
        classify.rateVariables(m_ratings);

        //
        // create the decision tree,  write to the log file and a special one
        //

        if( s_boost==0) {

            // single tree, no boosting
            m_dtree = classify.createTree(info.title());

        }else{

            // create boosted trees
            DecisionTree* boostedtree;
            double adaBeta = 0.5; // why is this wired in?
            AdaBoost booster(training,adaBeta);
            double boostwt = booster(classify); //weight of first tree, boost training sample
            m_dtree = classify.createTree(info.title(),boostwt);
            for (int itree = 1; itree < s_boost; ++itree) {
                log() << "Making boosted tree #" << itree << std::endl;
                training = booster.data(); //get boosted training sample
                Classifier classify(training);
                classify.makeTree(true);
                boostwt = booster(classify);//weight of itree, boost training sample
                boostedtree = classify.createTree(info.title(),boostwt);
                m_dtree->addTree(boostedtree);
            }
        }

        if(! info.filepath().empty()){
	  std::ofstream dtree_file( (info.filepath()+"/dtree.txt").c_str()); 
            m_dtree->print(dtree_file);
        }else{
            m_dtree->print(log());
        }
        evaluate(loader, evaluationset);
    }
    void Trainer::evaluate( RootLoader& loader, RootLoader::Subset set)
    {
        //
        // now evaluate it with events the from specified set
        //
        log() << "\nLoad ";
        switch (set){
            case RootLoader::ALL: log() << "ALL" ; break;
            case RootLoader::EVEN: log() << "EVEN"; break;
            case RootLoader::ODD: log() << "ODD"; break;
            default: log() << "unknown" ;
        }
         log()   <<"events for test" << std::endl;
        Classifier::Table testing_sample;
        loader(testing_sample, set, log());
        m_eff = new BackgroundVsEfficiency(*m_dtree, testing_sample);
        m_eff->print(log());
        if(! m_info.filepath().empty()){
	  std::ofstream test_file( (m_info.filepath()+"/test.txt").c_str());
	  m_eff->print( test_file);
        }
        log() << "Signal resolution: " << (m_sigma =m_eff->sigma()) << std::endl;
    }
    const std::vector<double>&  Trainer::variableRatings()const
    {
        return m_ratings;
    }

    void Trainer::summarize_setup(std::ostream & out)
    {
        out  << "\nTraining\t" << m_info.title()
            << "\n\toutput to\t" << m_info.filepath() <<std::endl;
        out << "\tsignal        \n \t\t"; 
        std::copy(m_info.signalFiles().begin(), m_info.signalFiles().end(), 
		  std::ostream_iterator<std::string>(out,"\n\t\t"));
        out << "\n\tbackground\n\t\t"; 
        std::copy(m_info.backgroundFiles().begin(), m_info.backgroundFiles().end(), 
		  std::ostream_iterator<std::string>(out,"\n\t\t"));
        out << "\n\tvariables\n\t\t"; 
        std::copy(m_info.vars().begin(), m_info.vars().end(), 
		  std::ostream_iterator<std::string>(out,"\n\t\t"));
        out << std::endl;
    }

    double Trainer::background(double efficiency)const {
        return (*m_eff)(efficiency);
    }

    Trainer::~Trainer()
    {
        delete m_dtree;
        delete m_eff;
    }
    std::ostream& Trainer::log(){return m_log==0? std::clog :  *m_log;}
    void Trainer::setLog(std::ostream * log){m_log = log;}
    void Trainer::setLog(std::string name){
        m_log = new std::ofstream( name.c_str());
    }

