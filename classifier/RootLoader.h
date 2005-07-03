/** @file  RootLoader.h
    @brief declaration of class RootLoader

    $Header: /cvsroot/d0cvs/classifier/classifier/RootLoader.h,v 1.5 2005/03/28 14:42:16 burnett Exp $
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef RootLoader_h
#define RootLoader_h

#include "Classifier.h"
#include <iostream>
class RandFlat;

/** @class RootLoader
    @brief subclass of Classifier::LoadData to load from a set of ROOT files 
    Called to fill a Classifier object.
*/
class RootLoader {
public:
/** @brief ctor
    @param signal_files list of root filenames containing signal
    @param background_files list of root filenames containing background
    @param names  list of the column names, starting with the weights
    @param use_weights [true] set false to test unweighted 
*/
    RootLoader(
        const Classifier::StringList & signal_files, 
        const Classifier::StringList & background_files, 
        const Classifier::StringList & names, 
        bool use_weights=true);

    typedef enum{ALL, EVEN, ODD, RANDOM } Subset;
    /// Fill a table, report contents to log
    void operator()(Classifier::Table& table, Subset set, std::ostream& log);

    double total(bool signal)const{return signal? m_signal_total : m_bkgnd_total;}
private:

   /** @brief load signal or background records from set of files
    */
    void load( Classifier::Table& table,  const Classifier::StringList& files, 
        bool signal, Subset set, std::ostream& log);

    const Classifier::StringList & m_signal_files;
    const Classifier::StringList & m_background_files;
    const Classifier::StringList & m_names;
    bool m_use_weights;
    double m_signal_total;
    double m_bkgnd_total;
    RandFlat* m_rand;
};

#endif

