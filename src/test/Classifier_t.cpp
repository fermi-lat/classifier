/** @file Classifier_t.cpp
    @brief test program

    $Header: /nfs/slac/g/glast/ground/cvs/classifier/src/test/Classifier_t.cpp,v 1.3 2005/10/29 17:30:13 burnett Exp $

    */

#include "classifier/Classifier.h"
#include "classifier/BackgroundVsEfficiency.h"
#include "classifier/AdaBoost.h"
#include "classifier/DecisionTree.h"
#include "classifier/Filter.h"

#include "CLHEP/Random/RandGauss.h"

#include <stdexcept>
#include <vector>

//using Classifier::Table;
//using Classifier::Record;

/**
  Test the Classifier with signal and background normal distributions
 See the <a href="test_classifier_doc.html"> discussion and output </a>. 
*/

class TestClassifier {
public:
    TestClassifier()
        :m_gaussian( new RandGauss(HepRandom::getTheEngine()) )
    {
        defineEvent();

        createData();

       // create the tree from the data
       Classifier tree(m_data);
       tree.makeTree();

       // print the node list, and the variables used
       tree.printTree();
       tree.printVariables();

       // a table of the background for a given efficiency
       BackgroundVsEfficiency plot(tree);
       plot.print();
       std::cout << "Signal error: " << plot.sigma() << std::endl;
      // check function
       std::cout << "Efficiency function:\neff\tbackground\n";
       for( double x = 0; x<=1.01; x+= 0.05){
            std::cout << x << "\t" << plot(x) << std::endl;
       }

       // check probability calculation
       double p1 = tree.probability(event(1.0)),
           p2 = tree.probability(event(-1.0));

       if( p1 < 0.8  || p2 > 0.2){
           throw std::runtime_error("didn't statisfy probabiity");
       }
       // make an auxialliary simple tree and print it to a local file
       DecisionTree& dtree = *tree.createTree("test_classifier");
       dtree.print();
       std::ofstream printout("temptree.txt");
       dtree.print(printout);
       double q1 = dtree(event(1.0)),
           q2 = dtree(event(-1.0));
       if( q1!=p1 || q2!=p2 ) throw std::runtime_error(" DecisionTree evaluation did not match");
     
 

       // check alternative
       std::cout << " This is an efficiency plot from the data and decision tree"<<std::endl;

       BackgroundVsEfficiency plot2(dtree, m_data);
       plot2.print();
  

       // try the class
       class TestValue : public DecisionTree::Values {
       public:
           TestValue( const std::vector<float>& row) : m_row(row)   {}
           double operator[](int i)const{  return m_row[i];}
       private:
           const std::vector<float>& m_row;
       };
       double r1 = dtree(TestValue(event(1.0)));
       if( r1 != q1) throw std::runtime_error("second evalution did not match");


       delete &dtree;

       // read back from the local file and print.
       std::ifstream fileinput("temptree.txt"); // declare here due to gcc bug :-(
       DecisionTree fromfile(fileinput);
       std::cout << "read back the tree " << fromfile.title() << std::endl;
       fromfile.print();

       // test creating and using a filter

       testFilter(fromfile);
    }
    void defineEvent()
    {
        // two column names, unweighted
        m_names.push_back("x");
        m_names.push_back("y");
        Classifier::Record::setup( m_names, false);


    }
    void createData()
    {
        // create the data table
       for( int i = 0; i<1000; ++i){ 
         m_data.push_back(Classifier::Record(true, event(normal(1.0, 1.0))));
         m_data.push_back(Classifier::Record(false, event(normal(-1.0, 1.0))));
       }
       m_data.normalize(0.5,0.5);
    }

    void testFilter(const DecisionTree & oldtree)
    {
        std::string testfile("filter.txt");

        std::cout << "\nTesting a filter...\n";
        // set up a little test file
        std::ofstream ftext(testfile.c_str());
        ftext << "# a test filter\n";
        ftext << "y < 1 " << std::endl;
        ftext << "y >= -1" << std::endl;
        ftext.close();

        DecisionTree& ftree= *new DecisionTree("test_classifier"); // make a new tree
        
        Filter f(m_names, ftree);
        f.addCutsFrom(testfile);
        f.close();

        std::cout << "print of the test tree"<< std::endl;

        ftree.print();

        // does it work? These should be 0 or 1.
        double t[] = 
        { 
            ftree(event(0,0) ),
            ftree(event(0,0.99)),
            ftree(event(0,1.01)),
            ftree(event(0,-1.01))
        };
        double expect[]={1,1,0,0};

        for( int i = 0; i<4; ++i){
            if( t[i]!= expect[i]) {
                std::cout << "expected " << expect[i]<<", found "<<t[i]<<std::endl;
                throw("Fail filter test");
            }
        }
        // now append the classification tree
        ftree.addTree(&oldtree);

        // and test again.
        double t2[] ={
             ftree(event(0,0) ),
             ftree(event(0,0.99) ),
             ftree(event(0,1.01) ),
             ftree(event(0,-1.01) )
        };
        
        double v0 = oldtree(event(0,0)); // from the old tree at 0,0
        for( int i = 0; i<4; ++i){
            if( t2[i]!= expect[i]*v0) {
                std::cout << "expected " << (expect[i]*v0) 
                    <<", found "<<t2[i]<<std::endl;
                throw("Fail filter test");
            }
        }

        std::cout << "Filter OK!" << std::endl;

    }

    /// return an event
    std::vector<float> event(double x, double y=0){
        std::vector<float> t;
        t.push_back(x);
        t.push_back(y);
        return t;
    }
private:
    double normal(double mean, double sigma=1.0){return m_gaussian->shoot(mean, sigma);}

    Classifier::Table m_data;
    std::vector<std::string> m_names;
    RandGauss * m_gaussian;

};


int main(int , char **)
{
    int rc=0;
    try {
//        Classifier::splitCriterion = Classifier::Entropy();
        TestClassifier();

    }catch (const std::exception & error)    {
        std::cerr << "Caught exception \"" << error.what() << "\"" <<std::endl;
        rc= 1;
    }
    return rc;
}
/** @page test_classifier_doc test program notes

Output from the test progam, test_classifier.cxx

 @verbatim
-------------------------------
--------- tree summary ----------
id      entries weight  Gini    purity  left branch
1       2000    1       0.5     0.5     x < 0.1064
3       947     0.4735  0.11647 0.86    x < 0.8242
7       589     0.2945  0.030261        0.95    x < 1.331
15      374     0.187   0.0097326       0.97    x < 1.63
31      254     0.127   0.0029646       0.99    (leaf)
30      120     0.06    0.0065917       0.94    (leaf)
14      215     0.1075  0.019749        0.9     (leaf)
6       358     0.179   0.073788        0.71    x < 0.4304
13      200     0.1     0.03318 0.79    (leaf)
12      158     0.079   0.037671        0.61    (leaf)
2       1053    0.5265  0.15508 0.18    x < -0.6
5       316     0.158   0.075759        0.4     (leaf)
4       737     0.3685  0.057615        0.085   x < -1.098
9       256     0.128   0.038371        0.18    (leaf)
8       481     0.2405  0.015468        0.033   x < -1.496
17      176     0.088   0.011182        0.068   (leaf)
16      305     0.1525  0.0039475       0.013   x < -2.047
33      161     0.0805  0.0019752       0.012   (leaf)
32      144     0.072   0.0019722       0.014   (leaf)
-------------------------------

Variable summary
Name    improvement
x       0.2706
y       0
Purity map
purity  weight  eff     cum_bkg
0       0       1       1
0.015   0.1525  0.996   0.699
0.065   0.088   0.984   0.535
0.185   0.128   0.937   0.326
0.395   0.158   0.811   0.136
0.605   0.079   0.715   0.074
0.795   0.1     0.557   0.032
0.895   0.1075  0.364   0.01
0.945   0.06    0.251   0.003
0.985   0.127   0       3.469e-018
Signal error: 0.1986
Efficiency function:
eff     background
0       3.469e-018
0.05    0.003
0.1     0.003
0.15    0.003
0.2     0.003
0.25    0.003
0.3     0.01
0.35    0.01
0.4     0.032
0.45    0.032
0.5     0.032
0.55    0.032
0.6     0.074
0.65    0.074
0.7     0.074
0.75    0.136
0.8     0.136
0.85    0.326
0.9     0.326
0.95    0.535
1       1
test_classifier
        0       -10     1
        1       0       0.1064
        2       0       -0.6
        4       0       -1.098
        8       0       -1.496
        16      0       -2.047
        32      -1      0.01389
        33      -1      0.01242
        17      -1      0.06818
        9       -1      0.1836
        5       -1      0.3987
        3       0       0.8242
        6       0       0.4304
        12      -1      0.6076
        13      -1      0.79
        7       0       1.331
        14      -1      0.8977
        15      0       1.63
        30      -1      0.9417
        31      -1      0.9882
 This is an efficiency plot from the data and decision tree
Purity map
purity  weight  eff     cum_bkg
0       0       1       1
0.015   0.1525  0.996   0.699
0.065   0.088   0.984   0.535
0.185   0.128   0.937   0.326
0.395   0.158   0.811   0.136
0.605   0.079   0.715   0.074
0.795   0.1     0.557   0.032
0.895   0.1075  0.364   0.01
0.945   0.06    0.251   0.003
0.985   0.127   0       0
read back the tree test_classifier
test_classifier
        0       -10     1
        1       0       0.1064
        2       0       -0.6
        4       0       -1.098
        8       0       -1.496
        16      0       -2.047
        32      -1      0.01389
        33      -1      0.01242
        17      -1      0.06818
        9       -1      0.1836
        5       -1      0.3987
        3       0       0.8242
        6       0       0.4304
        12      -1      0.6076
        13      -1      0.79
        7       0       1.331
        14      -1      0.8977
        15      0       1.63
        30      -1      0.9417
        31      -1      0.9882
@endverbatim

*/


