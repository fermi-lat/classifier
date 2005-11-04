/** @file ImSheetBuilder.cxx
 *    @brief implementation of classification::Tree; declaration and implementation or its private helper classification::ImSheetBuilder::Node
 *
 *    $Header: /nfs/slac/g/glast/ground/cvs/classification/src/Tree.cxx,v 1.22 2005/01/03 19:30:52 jrb Exp $
 */

#include "classifier/ImSheetBuilder.h"
    
#include "classifier/DecisionTreeBuilder.h"
    
#include "xmlBase/XmlParser.h"
#include "facilities/Util.h"
#include <xercesc/dom/DOMElement.hpp>
#include "xmlBase/Dom.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <set> 

namespace {

    // Helper function creates a std::string from a DOMString
    // Usage:
    // a = getNextWord(sList, iDelimPos=-1);
    // b = getNextWord(sList, iDelimPos);

    std::string getNextWord(std::string &sList, int &iEnd)
    {
        // This allows space delimited data as well as comma, semicolon, etc.
        static char  cDELIM = '.'; 
        const char *cList;
        int iStart = iEnd + 1; // Start is previous end plus one.
        if(iStart > (int)sList.length())
        {
            // Serious error when you don't initialize iDelimPos=-1
            // before calling this function!
            throw std::exception("Error with getNextWord!");
        }
        cList = sList.c_str();
        while((cList[iStart] == cDELIM) && cList[iStart] != 0)
        { 
            iStart++; 
        }
        iEnd = sList.find(cDELIM, iStart);
        if(iEnd > (int)sList.length())
        {
            throw std::exception("Error with getNextWord");
        }
        return sList.substr(iStart, iEnd - iStart);
    }

    double getNextDouble(std::string &sList, int &iEnd)
    {
        std::string sIValue = getNextWord(sList, iEnd);
        return atof(sIValue.c_str());
    }
    
    std::string indent(int depth)
    {
        std::string ret("  ");; 
        for( int i =0; i < depth; ++i) ret += "  ";
        return ret;
    }

    // I am tired of typing this....
    typedef std::vector<DOMElement*> DOMEvector;
} // anonomous namespace

XERCES_CPP_NAMESPACE_USE

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
* The idea for classification::Tree is to encapuslate the xml so that the
*   user only
*   has to provide a filename or string and we will give them access to the
*   linked list of the data which will have the ability to do neural 
*   networking algorithms.
*/


ImSheetBuilder::ImSheetBuilder(const DOMDocument* document, std::ostream& log) : m_headNode(0), m_log(log)
{
    // Check document validity
    if(document == 0)
    {
        // Error checking usually for a missing file.
        // Or sometimes due to a bad XML document.
        // Remember <P><BR></P> is only valid
        // in poorly formed html, not xml.
        // When we get a schema for UserLibrary, we'll
        // be able to use that as a validator also.
        throw std::exception( "Error: invalid input file ");
    }

    // Clear everything just to be sure
    m_nodeVec.clear();
    m_idToNodeMap.clear();
    m_typeToNodeVecMap.clear();

    // DecisionTree builder
    m_builder = new DecisionTreeBuilder(document);

    // Find the activity nodes in the document
    int numNodes = findAllActivityNodes(document);

    // Link them together
    int numLinks = linkActivityNodes(document);
  
    return;
}


ImSheetBuilder::~ImSheetBuilder()
{
    return;
}
  
int ImSheetBuilder::findAllActivityNodes(const DOMDocument* document)
{
    // Root...    
    DOMElement* domRoot = document->getDocumentElement();

    // This assumes that the node under the root node is Activity Node.
    // In IMW files, this is not the case.
    // IMML/Worksheet/ActivityNodeList
    //std::vector<std::string> sActNodeListPath;
    //sActNodeListPath.push_back("Worksheet");
    //sActNodeListPath.push_back("ActivityNodeList");

    //DOMElement* xmlList = findXPath(domRoot, sActNodeListPath);

    // If they are using the user library instead of IMW file, this will work.
    //if(xmlList == 0) xmlList = domRoot;

    // Build map between types and implementation of that type
    typedef ImActivityNode* (ImSheetBuilder::* ParseFunction)(const std::string&, const DOMElement*);
    std::map<std::string, ParseFunction> m_parseMap;
    m_parseMap.clear();

    m_parseMap["AppendEngineNode"]        = &ImSheetBuilder::parseAppendEngineNode;
    m_parseMap["CreateColumnsEngineNode"] = &ImSheetBuilder::parseCreateColumnsEngineNode;
    m_parseMap["FilterColumnsEngineNode"] = &ImSheetBuilder::parseFilterColumnsEngineNode;
    m_parseMap["FilterRowsEngineNode"]    = &ImSheetBuilder::parseFilterRowsEngineNode;
    m_parseMap["MissingValuesEngineNode"] = &ImSheetBuilder::parseMissingValuesEngineNode;
    m_parseMap["ModifyColumnsEngineNode"] = &ImSheetBuilder::parseModifyColumnsEngineNode;
    m_parseMap["PredictEngineNode"]       = &ImSheetBuilder::parsePredictEngineNode;
    m_parseMap["ReadTextFileEngineNode"]  = &ImSheetBuilder::parseReadTextFileEngineNode;
    m_parseMap["ShuffleEngineNode"]       = &ImSheetBuilder::parseShuffleEngineNode;
    m_parseMap["SplitEngineNode"]         = &ImSheetBuilder::parseSplitEngineNode;
    m_parseMap["WriteTextFileEngineNode"] = &ImSheetBuilder::parseWriteTextFileEngineNode;

    // We now need to ensure that we're getting 
    // ActivityNode[@engineClass=='com.insightful.miner.PredictEngineNode']
    std::vector<DOMElement *> xmlActivityNodes;
    //xmlBase::Dom::getChildrenByTagName(xmlList, "ActivityNode", xmlActivityNodes);
    xmlBase::Dom::getDescendantsByTagName(domRoot, "ActivityNode", xmlActivityNodes);

    // Loop over the ActivityNodes in the input xml file
    for(DOMEvector::iterator actNodeIter = xmlActivityNodes.begin();
        actNodeIter != xmlActivityNodes.end(); actNodeIter++)
    {
        DOMElement* xmlActivityNode = *actNodeIter;

        std::string sType = xmlBase::Dom::getAttribute(xmlActivityNode, "engineClass");  
        std::string sId   = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

        // Get the label for this node
        DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
        std::string sDisplay    = xmlBase::Dom::getTagName(displayInfo);
        std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");

        // Parse the engine class a bit to get to the unique type identifier
        int iDelim = -1;
        std::string sNewType = getNextWord(sType, iDelim);

        while(iDelim > -1)
        {
            sNewType = getNextWord(sType, iDelim);
        }

        // Parse the node if further action necessary
        std::map<std::string, ParseFunction>::iterator parseFuncIter = m_parseMap.find(sNewType);

        // Check to make sure we "know" the type
        if (parseFuncIter == m_parseMap.end())
        {
            throw std::invalid_argument("IM file parsing finds unknown node type: "+sNewType);
        }

        // Get this node
        ImActivityNode* activityNode = (this->*(parseFuncIter->second))(sNewType, xmlActivityNode);

        // Add to the list
        m_nodeVec.push_back(activityNode);

        // Store in maps
        m_idToNodeMap[sId] = activityNode;
        m_typeToNodeVecMap[sNewType].push_back(activityNode);
    }

    //done
    return m_nodeVec.size();
}
ImActivityNode* ImSheetBuilder::parseAppendEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}

ImActivityNode* ImSheetBuilder::parseCreateColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    // Need to find the list of variables...
    StringList outVarNames;

    // Get the list of arguments
    DOMElement* xmlArgumentList = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "ArgumentList");
    DOMElement* xmlXtProps      = xmlBase::Dom::findFirstChildByName(xmlArgumentList, "XTProps");

    // Now the vector of properties
    DOMEvector xmlPropertyVec;
    xmlBase::Dom::getChildrenByTagName(xmlXtProps, "Property", xmlPropertyVec);

    for(DOMEvector::iterator propIter = xmlPropertyVec.begin(); propIter != xmlPropertyVec.end(); propIter++)
    {
        DOMElement* xmlProperty = *propIter;

        std::string sName = xmlBase::Dom::getAttribute(xmlProperty, "name");

        if (sName == "newColumns")
        {
            DOMEvector xmlColumnsVec;
            xmlBase::Dom::getChildrenByTagName(xmlProperty, "Property", xmlColumnsVec);

            for(DOMEvector::iterator colPropIter = xmlColumnsVec.begin(); colPropIter != xmlColumnsVec.end(); colPropIter++)
            {
                DOMElement* xmlColVar = *colPropIter;

                DOMEvector xmlColVarVec;
                xmlBase::Dom::getChildrenByTagName(xmlColVar, "Property", xmlColVarVec);

                std::string sVarName = xmlBase::Dom::getAttribute(xmlColVar, "name");

                outVarNames.push_back(sVarName);

                try
                {
                    DOMElement* xmlComplex = xmlBase::Dom::findFirstChildByName(xmlColVarVec[1], "Complex");
                    std::string test = xmlBase::Dom::getTextContent(xmlComplex);
                    int j = 0;
                }
                //catch(xmlBase::WrongNodeType& e)
                catch(...)
                {
                    std::string sValue = xmlBase::Dom::getAttribute(xmlColVarVec[1], "value");
                    int i = 0;
                }
            }

            break;
        }
    }

    return node;
}

ImActivityNode* ImSheetBuilder::parseFilterColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);
        
    // Get the list of arguments
    DOMElement* xmlArgumentList = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "ArgumentList");
    DOMElement* xmlXtProps      = xmlBase::Dom::findFirstChildByName(xmlArgumentList, "XTProps");

    // Now the vector of properties
    DOMEvector xmlPropertyVec;
    xmlBase::Dom::getChildrenByTagName(xmlXtProps, "Property", xmlPropertyVec);

    // Loop through the properties looking for new columns
    for(DOMEvector::iterator propIter = xmlPropertyVec.begin(); propIter != xmlPropertyVec.end(); propIter++)
    {
        DOMElement* xmlProperty = *propIter;

        std::string sName = xmlBase::Dom::getAttribute(xmlProperty, "name");

        if (sName == "excludeColumns")
        {
            DOMElement* xmlExpression = xmlBase::Dom::findFirstChildByName(xmlProperty,  "Property");

            std::string sExpression = xmlBase::Dom::getAttribute(xmlExpression, "name");

            break;
        }
    }

    return node;
}

ImActivityNode* ImSheetBuilder::parseFilterRowsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);
        
    // Get the list of arguments
    DOMElement* xmlArgumentList = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "ArgumentList");
    DOMElement* xmlXtProps      = xmlBase::Dom::findFirstChildByName(xmlArgumentList, "XTProps");

    // Now the vector of properties
    DOMEvector xmlPropertyVec;
    xmlBase::Dom::getChildrenByTagName(xmlXtProps, "Property", xmlPropertyVec);

    // Loop through the properties looking for new columns
    for(DOMEvector::iterator propIter = xmlPropertyVec.begin(); propIter != xmlPropertyVec.end(); propIter++)
    {
        DOMElement* xmlProperty = *propIter;

        std::string sName = xmlBase::Dom::getAttribute(xmlProperty, "name");

        if (sName == "testExpression")
        {
            try
            {
                DOMElement* xmlComplex = xmlBase::Dom::findFirstChildByName(xmlProperty, "Complex");
                std::string test = xmlBase::Dom::getTextContent(xmlComplex);
                int j = 0;
            }
            //catch(xmlBase::WrongNodeType& e)
            catch(...)
            {
                std::string sValue = xmlBase::Dom::getAttribute(xmlProperty, "value");
                int i = 0;
            }

            break;
        }
    }

    return node;
}

ImActivityNode* ImSheetBuilder::parseMissingValuesEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}

ImActivityNode* ImSheetBuilder::parseModifyColumnsEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}

ImActivityNode* ImSheetBuilder::parsePredictEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);
    
    // Build the DecisionTree
    DecisionTree* decision = m_builder->parseForest(xmlActivityNode);

    // Set node values
    node->setDecisionTree(decision);
    node->setInputVar(m_builder->getVarNames());
    node->addOutputVar(m_builder->getOutVarName());

    // Done
    return node;
}

ImActivityNode* ImSheetBuilder::parseReadTextFileEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}

ImActivityNode* ImSheetBuilder::parseShuffleEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}


ImActivityNode* ImSheetBuilder::parseSplitEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);
        
    // Get the list of arguments
    DOMElement* xmlArgumentList = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "ArgumentList");
    DOMElement* xmlXtProps      = xmlBase::Dom::findFirstChildByName(xmlArgumentList, "XTProps");

    // Now the vector of properties
    DOMEvector xmlPropertyVec;
    xmlBase::Dom::getChildrenByTagName(xmlXtProps, "Property", xmlPropertyVec);

    // Loop through the properties looking for new columns
    for(DOMEvector::iterator propIter = xmlPropertyVec.begin(); propIter != xmlPropertyVec.end(); propIter++)
    {
        DOMElement* xmlProperty = *propIter;

        std::string sName = xmlBase::Dom::getAttribute(xmlProperty, "name");

        if (sName == "testExpression")
        {
            try
            {
                DOMElement* xmlComplex = xmlBase::Dom::findFirstChildByName(xmlProperty, "Complex");
                std::string test = xmlBase::Dom::getTextContent(xmlComplex);
                int j = 0;
            }
            //catch(xmlBase::WrongNodeType& e)
            catch(...)
            {
                std::string sValue = xmlBase::Dom::getAttribute(xmlProperty, "value");
                int i = 0;
            }
        }
    }

    return node;
}

ImActivityNode* ImSheetBuilder::parseWriteTextFileEngineNode(const std::string& sType, const DOMElement* xmlActivityNode)
{
    // Retrieve name and node id
    DOMElement* displayInfo = xmlBase::Dom::findFirstChildByName(xmlActivityNode, "DisplayInfo");
    std::string sName       = xmlBase::Dom::getAttribute(displayInfo, "labelText");
    std::string sId         = xmlBase::Dom::getAttribute(xmlActivityNode, "id");

    // Create the node
    ImActivityNode* node = new ImActivityNode(sType, sName, sId);

    return node;
}



int ImSheetBuilder::linkActivityNodes(const DOMDocument* document)
{
    // Root...    
    DOMElement* domRoot = document->getDocumentElement();

    // This assumes that the node under the root node is Activity Node.
    // In IMW files, this is not the case.
    // IMML/Worksheet/ActivityNodeList
    std::vector<std::string> sLinkListPath;
    sLinkListPath.push_back("Worksheet");
    sLinkListPath.push_back("LinkList");

    DOMElement* xmlList = findXPath(domRoot, sLinkListPath);

    // If they are using the user library instead of IMW file, this will work.
    if(xmlList == 0) xmlList = domRoot;

    // We now need to ensure that we're getting 
    // ActivityNode[@engineClass=='com.insightful.miner.PredictEngineNode']
    std::vector<DOMElement *> xmlLinkVec;
    xmlBase::Dom::getChildrenByTagName(xmlList, "Link", xmlLinkVec);

    int numLinks = xmlLinkVec.size();

    // Make a copy of the node vector so we can find the "head" node
    std::vector<ImActivityNode*> headVec = m_nodeVec;

    for(DOMEvector::iterator linkIter = xmlLinkVec.begin();
        linkIter != xmlLinkVec.end(); linkIter++)
    {
        DOMElement* xmlLink = *linkIter;

        std::string sFromNode = xmlBase::Dom::getAttribute(xmlLink, "fromNode");  
        std::string sFromPort = xmlBase::Dom::getAttribute(xmlLink, "fromPort");  
        std::string sToNode   = xmlBase::Dom::getAttribute(xmlLink, "toNode");
        std::string sToPort   = xmlBase::Dom::getAttribute(xmlLink, "toPort");

        //if (sFromPort != "0" || sToPort != "0") continue;
        if (sFromPort == "1" && sToPort == "1") continue;

        ImActivityNode* fromNode = m_idToNodeMap[sFromNode];
        ImActivityNode* toNode   = m_idToNodeMap[sToNode];

        fromNode->setNodeLink(toNode);

        // Find the "to" node in what remains of the vector of node pointers
        std::vector<ImActivityNode*>::iterator toNodeIter = std::find(headVec.begin(), headVec.end(), toNode);

        // Can have the case that a node has already been removed
        if (toNodeIter != headVec.end())
        {
            headVec.erase(toNodeIter);
        }
    }

    // Head vector should be last man standing?
    int headVecSize = headVec.size();
    if (headVecSize > 0) 
    {
        m_headNode = headVec.front();
    }
    else
    {
        throw std::exception("ImSheetBuilder did not find a HEAD node!");
    }

    //done
    return m_nodeVec.size();
}

std::vector<ImActivityNode*> ImSheetBuilder::getActivityNodeVec(std::string& type)
{
    typeToNodeVecMap::iterator nodeIter = m_typeToNodeVecMap.find(type);

    if (nodeIter != m_typeToNodeVecMap.end()) return nodeIter->second;
    else                                      return std::vector<ImActivityNode*>(0);
}


// For output
void ImSheetBuilder::print(std::ostream& out) const
{
    // Start with output of details of the sheet
    out << "***********************" << std::endl;
    out << " Number of Activity nodes: " << m_nodeVec.size() << std::endl;
    out << " Number of Types found:    " << m_typeToNodeVecMap.size() << std::endl;

    for(typeToNodeVecMap::const_iterator typeIter = m_typeToNodeVecMap.begin(); 
        typeIter != m_typeToNodeVecMap.end(); typeIter++)
    {
        out << "   Type: " << (*typeIter).first << ", Number of instances: " << (*typeIter).second.size() << std::endl;
    }
    
    int depth = 0;

    // Begin with the head node and output the nodes in their order:
    if (m_headNode) m_headNode->print(out, depth);

    return;
}


  // Inputs a node (usually the root node)
  // and a list such as:
  //	char *straXMLPath[] = {"ActivityNode", "ModelProperties", "IMML", 
  //                           "TreeList", "TreeModel", 0};
  // And returns the node at the end, in this case: TreeModel.
  // The node given should be:
  // <xmlParent><ActivityNode><ModelProperties>
  //		<IMML><TreeList><TreeModel></TreeModel></TreeList></IMML>
  // </ModelProperties></ActivityNode></xmlParent>
  // When calling this function be sure to check Node==0 
  // or Node.isNull()==true;
  // Replace above function entirely because 
  //  - unnecessary calls to native Xerces routines
  //  - vector of strings is much nicer to deal with than array
  //  - all this requires is a simple loop, not recursion
DOMElement* ImSheetBuilder::findXPath(DOMElement* xmlParent, const std::vector<std::string>& nodeNames)
{
    unsigned nNames = nodeNames.size();
    if (nNames == 0) return xmlParent;

    DOMElement* parent = xmlParent;
    DOMElement* child;
    for (unsigned iName = 0; iName < nNames; iName++) 
    {
        child = xmlBase::Dom::findFirstChildByName(parent, nodeNames[iName]);
        if (child == 0) return child;  // path ended prematurely
        parent = child;
    }
    
    return child;
}

ImActivityNode::ImActivityNode(const std::string& type, const std::string& name, const std::string& id) : 
                               m_type(type), m_name(name), m_id(id), m_decisionTree(0) 
{
    m_nodeVec.clear();
    m_inputVar.clear();
    m_outputVar.clear();
}

// Does the "real" work... 
void ImActivityNode::print(std::ostream& out, int depth) const
{
    // Output our node ID, type and name
    out << indent(depth) << "ID: " << m_id << ", Type: " << m_type << ", Label: " << m_name << std::endl;

    // What do we set depth to?
    depth = m_nodeVec.size() > 1 ? depth + 1 : depth;

    // Now follow through with all the nodes we point to
    for(ImActivityNodeVec::const_iterator nodeIter = m_nodeVec.begin(); nodeIter != m_nodeVec.end(); nodeIter++)
    {
        (*nodeIter)->print(out, depth);
    }

    return;
}
