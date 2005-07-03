/** @file  RootLoader.cpp
@brief implementation  of class RootLoader

$Header: /cvsroot/d0cvs/classifier/src/RootLoader.cpp,v 1.5 2005/03/28 14:42:16 burnett Exp $

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "classifier/RootLoader.h"
#include "classifier/RootTuple.h"
#include "classifier/Classifier.h"
#include "CLHEP/Random/RandFlat.h"

#include <iomanip>
#include <stdexcept>
#include <iterator>

RootLoader::RootLoader(
                       const Classifier::StringList & signal_files, 
                       const Classifier::StringList & background_files, 
                       const Classifier::StringList & names, 
                       bool use_weights)
                       :m_signal_files(signal_files)
                       , m_background_files(background_files)
                       , m_names(names)
                       , m_use_weights(use_weights)
                       , m_rand(new RandFlat(HepRandom::getTheEngine()))

{
    if(m_names.size()<2) { throw std::invalid_argument(
        " RootLoader::RootLoader -- must be at least 2 variable names");}
    // set static variables in Classifier::Record
    Classifier::Record::setup(names, use_weights);
}

void RootLoader::operator()(Classifier::Table& table, Subset set, std::ostream& log)
{
    log << "Loading from root files, ";
    switch (set){
        case ALL: log << "all"; break;
        case EVEN: log << "even"; break;
        case ODD : log << "odd"; break;
        case RANDOM : log << "random"; break;
    }
    log << " events\n";
    log << "Selected variables:\n\t ";
    std::copy(m_names.begin(), m_names.end(), 
        std::ostream_iterator<std::string>( log, "\n\t"));

    log << "\n  file   records   weights\n";
    log << "---------signal--------------\n";
    load(table, m_signal_files, true, set, log);
    log << "---------background----------\n";
    load(table, m_background_files, false, set, log);
    log << "-----------------------------\n";
}

/** 
@brief load signal or background records from set of files
*/
void RootLoader::load(Classifier::Table& table, 
                      const Classifier::StringList& files, 
                      bool signal, Subset set,  std::ostream& log)
{
    double total_count=0, total_sum=0;


    for(Classifier::StringList::const_iterator i = files.begin(); i!=files.end(); ++i){
        RootTuple t(*i+".root", "TopTree");
        t.selectColumns(m_names, m_use_weights); 
        double sum=0, count=0;
        RootTuple::Iterator rit = t.begin();
        if( set==EVEN) ++rit;
	if( set==RANDOM && m_rand->shoot()>0.5) ++rit; // skip first
        for( ; rit!=t.end(); ++rit){
            const std::vector<float> & row = *rit;
            double weight = row[0]; // weight must be first column
            std::vector<float> rowx(row); // make a copy
            sum+= weight;
            ++count;
            // create a record with the signal or background weight

            table.push_back( Classifier::Record(signal, rowx));
            // if doing alternate or random, skip the next record here.
            if( set !=ALL || set==RANDOM && m_rand->shoot()>0.5 ){
                ++rit; if( rit == t.end() )break;
            }

        }
        log << std::setw(10) << (*i) 
            << std::setw(6) << count 
            << std::setw(10) << sum << std::endl;
        total_count += count;
        total_sum += sum;
    }
    if( signal) m_signal_total = total_sum; 
    else        m_bkgnd_total=total_sum;
    log << std::setw(10) << "total" 
        << std::setw(6) << total_count 
        << std::setw(10) << total_sum << std::endl;
}

