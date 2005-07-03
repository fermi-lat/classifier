/** @file TrainerApplication.cpp
    @brief implement TrainerApplication 

    $Header: /cvsroot/d0cvs/classifier/src/TrainerApplication.cpp,v 1.4 2005/03/31 00:39:19 burnett Exp $

    */
#include "classifier/TrainerApplication.h"

#include "classifier/TrainingInfo.h"
#include "classifier/Trainer.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#ifdef WIN32
# include <direct.h>
int chdir(const char* path){return _chdir(path);}
#else
# include <unistd.h>
#endif
#include <time.h>
#include <stdio.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TrainerApplication::TrainerApplication(const std::string& datapath, const std::string& outputpath)
        :m_outputpath(outputpath)
    {
        current_time();
        std::cout << "Classification analysis: Reading data files from "
            << datapath << std::endl;
        ::chdir( datapath.c_str() );
        std::cout << "Output path is " << outputpath << std::endl;

        if( Trainer::s_boost==0){
            std::cout << "not boosing" << std::endl;
        }else{
            std::cout << "boosting " << Trainer::s_boost << " times" << std::endl;
        }

        std::ifstream casefile( (outputpath+"/cases.txt").c_str() );
        if( !casefile.is_open() ){
            throw std::runtime_error("Could not open file " + outputpath + "/cases.txt");
        }
        std::vector< std::string > cases;
        while ( !casefile.eof() ){
            char line[80]; casefile.getline(line,sizeof(line));
            if( line[0] ==0 || line[0]=='#' ) continue;
            cases.push_back( std::string(line));
        }

        // now do the training for all the cases above, collecting stats 
        //  on the background vs. efficiency
        m_cases = cases.size();
        m_plot.resize(m_cases);
            
        train(cases);
 
        // done: print it all out
        printEfficiencyTable(std::cout);
	std::ofstream efftable( (outputpath+"/efficiency_table.txt").c_str() );
        printEfficiencyTable(efftable);
	std::ofstream vartable((outputpath+"/variable_rating_table.txt").c_str());
        printVariableRatingTable(vartable);
    }

    void TrainerApplication::train(std::vector<std::string> cases)
    {
        
        for( size_t i =0; i< cases.size(); ++i){
            TrainingInfo info(m_outputpath +"/"+cases[i]);
            // train on even, test on odd events
            Trainer t(info, std::cout, RootLoader::EVEN, RootLoader::ODD);
            m_titles.push_back(info.title());
            for( double eff=0.10; eff<1.001; eff+=0.025){
                m_plot[i][eff]=t.background(eff);
            }
            m_signal_total.push_back( t.total(true) );
            m_bkgnd_total.push_back( t.total(false) );
            m_sigmas.push_back(t.sigma());

            const std::vector<double>& ratings = t.variableRatings();
            std::vector<std::string>::const_iterator iname = info.vars().begin();
            ++iname; // skip the weight
            std::vector<double>::const_iterator ival = ratings.begin();
            for(;  ival!=ratings.end(); ++ival, ++iname){
                std::vector<double> row = m_variable_ratings[*iname];
                if( row.size()==0){ row.resize(m_cases, -1);} // if not set, 
                row[i] = *ival;
                m_variable_ratings[*iname]= row;
            }
            current_time();
        }
    }

    /// simple class to print the gini improvement
    class PrintGini {
    public:
        PrintGini(std::ostream& out): m_out(out){}
        void operator()(double x){
            m_out << "\t";
            if( x<0) m_out << "---" ; else m_out << std::setprecision(3) << 100*x;
        }
        std::ostream& m_out;
    };
            
    void TrainerApplication::printVariableRatingTable(std::ostream& out)
    {
        out << "Table of Variables used with ratings\nname";
        for( int i=0; i<m_cases; ++i) out << "\t"<< m_titles[i];
        out << std::endl;
        for(std::map<std::string, std::vector<double> >::const_iterator irow=m_variable_ratings.begin(); 
            irow!=m_variable_ratings.end(); ++irow)
        {
            const std::string& name = irow->first;
            out << name ;
            const std::vector<double>& row = irow->second;
            std::for_each( row.begin(), row.end(), PrintGini(out));
            out << std::endl;
        }
    }

    void TrainerApplication::printEfficiencyTable(std::ostream& out)
    {
        out << "Table of background fraction vs. efficiency\neff";
        for( int i=0; i<m_cases; ++i) out << "\t"<< m_titles[i];
        out << std::endl;
        for( std::map<double,double>::const_iterator mit= m_plot[0].begin(); mit!=m_plot[0].end(); ++mit){
            double eff= mit->first;
            out <<eff;
            for (int i=0; i<m_cases; ++i){
                out << "\t" << m_plot[i][eff];;
            }
            out << std::endl;
        }
        // summary
        out << "\nsignal";
        for (int i=0; i<m_cases; ++i) out << "\t " << m_signal_total[i];
        out << "\nbkgnd";
        for (int i=0; i<m_cases; ++i) out << "\t " << m_bkgnd_total[i];
        out << "\nsigmas";
        for (int i=0; i<m_cases; ++i) out << "\t " << m_sigmas[i];
        out << std::endl;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void TrainerApplication::current_time(std::ostream& out)
    {   
        static bool first=true;
        static time_t start;
        if(first){ first=false; ::time(&start);}
        time_t aclock;
        ::time( &aclock );   
        char tbuf[25]; ::strncpy(tbuf, asctime( localtime( &aclock ) ),24);
        tbuf[24]=0;
        out<<  "Current time: " << tbuf
            << " ( "<< ::difftime( aclock, start) <<" s elapsed)" << std::endl;
    }
