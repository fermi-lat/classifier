/** @file Tree.h
*   @brief  Builds DecisionTrees from a given input file.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/classification/classification/Tree.h,v 1.21 2004/11/10 22:45:07 jrb Exp $
*/

#ifndef ImSheetBuilder_h
#define ImSheetBuilder_h

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class  DOMDocument;
class  DOMElement;
XERCES_CPP_NAMESPACE_END

using XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class DecisionTree;
class DecisionTreeBuilder;
/** @class ImActivityNode
*  @brief  Describes an ActivityNode encountered in reading an IM xml file
*
*/

class ImActivityNode
{
public:
    typedef std::vector<std::string> StringList;
    typedef std::vector<ImActivityNode*> ImActivityNodeVec;

    ImActivityNode(const std::string& type, const std::string& name, const std::string& id);
    ~ImActivityNode() {}

    // 
    const std::string& getType()          const {return m_type;}
    const std::string& getName()          const {return m_name;}
    const std::string& getId()            const {return m_id;}
    const ImActivityNodeVec& getNodeVec() const {return m_nodeVec;}
    const StringList&  getOutputVarList() const {return m_outputVar;}
    const StringList&  getInputVarList()  const {return m_inputVar;}

    DecisionTree*      getDecisionTree()  const {return m_decisionTree;}

    void setNodeLink(ImActivityNode* linkToNode) {m_nodeVec.push_back(linkToNode);}
    void setDecisionTree(DecisionTree* decision) {m_decisionTree = decision;}
    void setInputVar(const StringList& inVars)   {m_inputVar = inVars;}
    void setOutputVar(const StringList& outVars) {m_outputVar = outVars;}
    void addOutputVar(const std::string& outVar) {m_outputVar.push_back(outVar);}

    void print(std::ostream& out=std::cout, int depth=0) const;

private:
    std::string       m_type;
    std::string       m_name;
    std::string       m_id;
    ImActivityNodeVec m_nodeVec;
    StringList        m_outputVar;
    StringList        m_inputVar;
    DecisionTree*     m_decisionTree;
};

/** @class ImSheetBuilder
*  @brief  Apply a classification Tree derived from Insightful Miner .
*
*  The idea is to encapuslate the xml so that the user only
*   has to provide a filename or string and we will give them access to the
*   linked list of the data which will have the ability to apply the tree 
*   classification algorithms.
*/

class ImSheetBuilder
{
public:
    typedef std::vector<std::string> StringList;

    /** 
        @param searcher call-back to connect with input data at runtime
        @param log  [cout]  ostream for output
        @param iVerbosity [0] -1 no output; 0 errors only; 1 info 2 debug
    */
    ImSheetBuilder(const DOMDocument* document, std::ostream& log=std::cout);
    ~ImSheetBuilder();

    // Return a list of activity nodes
    std::vector<ImActivityNode*> getActivityNodeVec(std::string& type);

    // For output
    void print(std::ostream& out=std::cout) const;

private:

    // Mapping between a node ID and the ImActivityNode object
    typedef std::map<std::string, ImActivityNode*> idToNodeMap;

    // Mapping between the ActivityNode type and a vector of pointers to these nodes
    typedef std::map<std::string, std::vector<ImActivityNode*> > typeToNodeVecMap;

    // Parse the file to find all Activity Nodes
    int findAllActivityNodes(const DOMDocument* document);

    // Parse the link list to connect the Activity Nodes
    int linkActivityNodes(const DOMDocument* document);

    // Parsing methods
    ImActivityNode* parseAppendEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseCreateColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseFilterColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseFilterRowsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseMissingValuesEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseModifyColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parsePredictEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseReadTextFileEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseShuffleEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseSplitEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);
    ImActivityNode* parseWriteTextFileEngineNode(const std::string& sType, const DOMElement* xmlActivityNode);

    /// Finds first child given path
    DOMElement* findXPath(DOMElement* xmlParent, const std::vector<std::string>& nodeNames);

    // Vector of Activity Nodes
    std::vector<ImActivityNode*> m_nodeVec;

    // The "head" ActivityNode
    ImActivityNode*              m_headNode;

    // Map between the node ID and the Activity Node object
    idToNodeMap                  m_idToNodeMap;

    // Map between ActivityNode type and vector of pointers to objects
    typeToNodeVecMap             m_typeToNodeVecMap;

    // Pointer to DecisionTreeBuilder until better idea 
    DecisionTreeBuilder*         m_builder;

    // output related info
    std::ostream&                m_log;         //! output to this stream
};

#endif // ifdef CLASSIFY_H_
