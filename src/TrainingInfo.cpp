/** @file TrainingInfo.cxx
    @brief implementaion of the class TrainingInfo

    $Header: /cvsroot/d0cvs/classifier/src/TrainingInfo.cpp,v 1.5 2005/02/08 21:14:21 burnett Exp $
*/
#include "classifier/TrainingInfo.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
namespace {
        char buf[1024]; // used for getline calls below
}
TrainingInfo::TrainingInfo( const std::string& title,  const std::string& varstring,
        const std::string& signals, const std::string& backgrounds, 
        const std::string& log)
        : m_title(title)
        , m_log(log)
    {
        parser(varstring, m_vars);
        parser(signals, m_signalFiles);
        parser(backgrounds, m_backgroundFiles);
    }
 
    TrainingInfo::TrainingInfo(const std::string& filepath, const std::string& rootfilepath)
        : m_filepath(filepath)
    {
        using std::ifstream;
        ifstream title((filepath+"/title.txt").c_str() );
        if( !title.is_open()) throw std::runtime_error("could not find file: "+filepath+"/title.txt");
        title.getline(buf,sizeof(buf));
        m_title = std::string(buf);

        readnames("variables.txt", m_vars);
        bool signal_files = true;;
        std::vector<std::string> files;
        readnames("files.txt", files);
        for(std::vector<std::string>::iterator sit = files.begin(); sit!=files.end(); ++sit){
            std::string file_name = rootfilepath+ (*sit);
            int pos =file_name.find(',') ; 
            if( pos == std::string::npos) {
                if(signal_files)  m_signalFiles.push_back(file_name);
                else m_backgroundFiles.push_back(file_name);
                signal_files=false;
            }else{
                if(signal_files)  m_signalFiles.push_back(file_name.substr(0,pos));
                else m_backgroundFiles.push_back(file_name.substr(0,pos));
            }
        }
        
        m_log = filepath+"/log.txt";
    }
     //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    std::string TrainingInfo::strip(std::string input)
    {
        size_t i = input.find_first_not_of(" "),
            j = input.substr(i).find_first_of(" ");
        return j==std::string::npos? input.substr(i) : input.substr(i, j-i+1);
    }
      
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void TrainingInfo::parser(const std::string& input, std::vector<std::string>& output)
    {
        size_t i=0 ;
        for(; i != std::string::npos ; ){
            size_t j = input.substr(i).find_first_of(",");
            if( j==std::string::npos ){
                output.push_back(strip(input.substr(i)));
                break;
            } else{
                output.push_back(strip(input.substr(i, j)));
                i += j+1;
            }
        }
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void TrainingInfo::readnames(const std::string& filename, std::vector<std::string>& output)
    {
        std::string fullpath(m_filepath+"/"+filename);
        std::ifstream input( fullpath.c_str());
        if( ! input.is_open() ) throw std::runtime_error("TrainingInfo: could not find file "+fullpath);

        while( ! input.eof() ){
            input.getline(buf,sizeof(buf));
            if( buf[0]=='#' || buf[0]=='\0' ) continue;
            std::stringstream str(buf);
            std::string name;
            str >> name;
            
            if( ! name.empty()) output.push_back(name);
        }
    }
