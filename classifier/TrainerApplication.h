/** @file TrainerApplication.h
    @brief define the class

    $Header: /cvsroot/d0cvs/classifier/classifier/TrainerApplication.h,v 1.3 2005/03/28 14:39:47 burnett Exp $

    */
#ifndef classifier_TrainerApplication_h
#define classifier_TrainerApplication_h
#include "classifier/TrainingInfo.h"
#include "classifier/Trainer.h"

#include <string>
#include <vector>
#include <map>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** @class TrainerApplication
     @brief application class

     @author T. Burnett

     Manage analysis of classification training.  

     Writes a file with a table of background fractions vs efficiency.

  */

class TrainerApplication {
public:
   /** @brief run everything from constructor
   @param datapath File path to the data files,
   @param outputpath File path to the cases, where individual output will go

   */
    TrainerApplication(const std::string& datapath, const std::string& outputpath);
private:
    void train(std::vector<std::string> cases);
    void printVariableRatingTable(std::ostream& out);

    void printEfficiencyTable(std::ostream& out);
    static void current_time(std::ostream& out=std::cout);
    std::string m_outputpath;
    std::vector<std::map<double, double> > m_plot;
    std::vector<std::string> m_titles;
    int m_cases;
    std::vector<double> m_signal_total, m_bkgnd_total;
    std::map<std::string, std::vector<double> > m_variable_ratings;
    std::vector<double> m_sigmas; 
};

#endif

