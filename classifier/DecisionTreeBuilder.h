/** @file Tree.h
*   @brief  Builds DecisionTrees from a given input file.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/classification/classification/Tree.h,v 1.21 2004/11/10 22:45:07 jrb Exp $
*/

#ifndef DecisionTreeBuilder_h
#define DecisionTreeBuilder_h

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class  DOMDocument;
class  DOMElement;
XERCES_CPP_NAMESPACE_END
    
#include "classifier/DecisionTree.h"

using XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
/** @class DecisionTreeBuilder
*  @brief  Apply a classification Tree derived from Insightful Miner .
*
*  The idea is to encapuslate the xml so that the user only
*   has to provide a filename or string and we will give them access to the
*   linked list of the data which will have the ability to apply the tree 
*   classification algorithms.
*/

class DecisionTreeBuilder
{
public:
    typedef std::vector<std::string> StringList;

    /** 
        @param searcher call-back to connect with input data at runtime
        @param log  [cout]  ostream for output
        @param iVerbosity [0] -1 no output; 0 errors only; 1 info 2 debug
    */
    DecisionTreeBuilder(const DOMDocument* document, std::ostream& log=std::cout,  int iVerbosity=0);
    ~DecisionTreeBuilder();

    /** initialize the tree from an XML file, dictionary 
        @param filename XML file name
    */
    DecisionTree* buildTree(const std::string &treeName);

    /** Build a DecisionTree straight from the top DOMElement
        @param xmlActivityNode is a pointer to the top DOMElement in the forest
    */
    DecisionTree* parseForest(const DOMElement* xmlActivityNode);

    /** Return the name of the variable output by this CT
    */
    const std::string& getOutVarName() const {return m_outVarName;}

    /** Return the list of variable names used by this tree
    */
    const StringList& getVarNames() const {return m_varNames;}

    /** @class Exception 
        @brief hold a string
    */
    class Exception : public std::exception
    {
    public: 
        Exception(std::string error):m_what(error){}
        ~Exception() throw() {;}
        virtual const char *what( ) const  throw() { return m_what.c_str();} 
        std::string m_what;
    };

private:

    // Mapping between the independent variable in this CT and its "index"
    typedef std::map<std::string, int> tupleVarIndexMap;

    void parseTree(DOMElement* xmlTreeModel, DecisionTree* decisionTree, std::map<std::string, int>& varIndexMap);

    void parseNode(DOMElement* xmlElement, int nodeId, DecisionTree* decisionTree, std::map<std::string, int>& varIndexMap);

    /// Finds first child given path
    const DOMElement* findXPath(const DOMElement* xmlParent, const std::vector<std::string>& nodeNames);

    /// Find output variable name for given Classification Tree
    std::string getCTOutputName(const DOMElement* xmlActivityNode);

    /// Find the list of independent variables used by this Classification Tree
    tupleVarIndexMap buildVarIndexMap(DOMElement* xmlNode);

    /// Name of the output variable (e.g. Pr(CORE) or something)
    std::string                  m_outVarName;

    /// "specified category" name - for determining which "yprob" value to use
    std::string                  m_specCatName;

    /// List of variable names used by this tree
    StringList                   m_varNames;

    /// Index to use when extracting value of yprob
    int                          m_yProbIndex;
    int                          m_numProbVals;

    /// reference to object implementing method to return a pointer to a double
    //ILookUpIndex&                m_searcher;
    //    const VariableMap *  m_fields; // variable names
    const DOMDocument*           m_domDocument;   //! File name for reference.

    std::ostream&                m_log;         //! output to this stream
    int                          m_outputLevel; //! output level (verbosity)
};

#endif // ifdef CLASSIFY_H_
