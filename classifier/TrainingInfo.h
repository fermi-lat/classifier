/** @file TrainingInfo.h
    @brief declaration of the class TrainingInfo

    $Header: /cvsroot/d0cvs/classifier/classifier/TrainingInfo.h,v 1.3 2005/02/04 12:56:19 burnett Exp $
*/
#ifndef TrainingInfo_h
#define TrainingInfo_h

#include <string>
#include <vector>

/** @class TrainingInfo
@brief contain the information necessary to train a tree

*/
  
class TrainingInfo {
public:
    typedef std::vector<std::string> StringList;

    /** @brief ctor with explicit values
    */
    TrainingInfo( const std::string& title,  const std::string& varstring,
        const std::string& signals, const std::string& backgrounds, 
        const std::string& log);

    /**  @brief ctor to get stuff from the folder
    @param filepath file path to files with input
    */
    TrainingInfo(const std::string& filepath);

    const std::string& title()const{return m_title;}
    const StringList& vars()const{return m_vars;}
    const StringList& signalFiles()const{return m_signalFiles;}
    const StringList& backgroundFiles()const{return m_backgroundFiles;}
    const std::string& log()const{return m_log;}
    const std::string& filepath()const{return m_filepath;}

    bool weighted()const{return true;}
    
    std::string strip(std::string input);
      
    void parser(const std::string& input, std::vector<std::string>& output);
    void readnames(const std::string& filename, std::vector<std::string>& output);

private:
    std::string m_title;
    StringList m_vars;
    StringList m_signalFiles;
    StringList m_backgroundFiles;
    std::string m_log;
    std::string m_filepath;
};

#endif

