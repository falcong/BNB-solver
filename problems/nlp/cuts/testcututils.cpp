/* 
 * File:   testcututils.cpp
 * Author: medved
 *
 * Created on January 20, 2015, 8:20 PM
 */

#include <vector>
#include <iostream>
#include "cututils.hpp"

void initBox(Box<double> &box) {
    box.mA[0] = 0;
    box.mA[1] = 0;
    box.mB[0] = 12;
    box.mB[1] = 6;
}

/*
 * 
 */
int main(int argc, char** argv) {
    Box<double> box(2);
    initBox(box);
    Cut<double> cut;
    cut.mC.realloc(2);
    cut.mC[0] = 6;
    cut.mC[1] = 3;
    cut.mR = 10;
    cut.mType = Cut<double>::CutType::INNER_BALL;
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == true);
    cut.mR = 5;

    /** Tesing inner ball cut with generic variables **/
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == false);
    std::vector< Box<double> > boxv;
    CutUtils<double>::ApplyInnerBallCutSimple(cut, box, std::vector<unsigned int> (), boxv);
    std::cout << "Inner ball cut test\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 2);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 2) && (boxv[0].mB[1] == 6));
    BNB_ASSERT((boxv[1].mA[0] == 10) && (boxv[1].mA[1] == 0) && (boxv[1].mB[0] == 12) && (boxv[1].mB[1] == 6));

    /** Testing inner ball cut with integral variable **/
    cut.mR = 4;
    boxv.clear();
    CutUtils<double>::ApplyInnerBallCutSimple(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::INTEGRAL, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Inner ball cut with integral vars test \n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 2);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 3) && (boxv[0].mB[1] == 6));
    BNB_ASSERT((boxv[1].mA[0] == 9) && (boxv[1].mA[1] == 0) && (boxv[1].mB[0] == 12) && (boxv[1].mB[1] == 6));

    /** Testing inner ball cut with boolean variable **/
    boxv.clear();
    CutUtils<double>::ApplyInnerBallCutSimple(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::BOOLEAN, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Inner ball cut with boolean vars test I\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 1) && (boxv[0].mB[1] == 6));

    cut.mR = 6;
    boxv.clear();
    CutUtils<double>::ApplyInnerBallCutSimple(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::BOOLEAN, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Inner ball cut with boolean vars test II\n";
    
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 0) && (boxv[0].mB[1] == 6));


    /** Testing outer ball cuts **/
    cut.mC[0] = 20;
    cut.mC[1] = 40;
    cut.mType = Cut<double>::CutType::OUTER_BALL;
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == true);

    cut.mC[0] = 6;
    cut.mC[1] = 3;
    cut.mR = 4;
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == false);

    /** Testing outer ball cuts with generic variables **/
    boxv.clear();
    CutUtils<double>::ApplyOuterBallCut(cut, box, std::vector<unsigned int> (), boxv);
    std::cout << "Outer ball cut test\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 2) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 10) && (boxv[0].mB[1] == 6));

    /** Testing outer ball cuts with integral variables **/
    cut.mR = 4.5;
    boxv.clear();
    CutUtils<double>::ApplyOuterBallCut(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::INTEGRAL, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Outer ball cut test with integral variables\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 2) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 10) && (boxv[0].mB[1] == 6));

    /** Testing outer ball cuts with boolean variables **/
    boxv.clear();
    CutUtils<double>::ApplyOuterBallCut(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::BOOLEAN, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Outer ball cut test with boolean variables I\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.empty());

    cut.mR = 10;
    boxv.clear();
    CutUtils<double>::ApplyOuterBallCut(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::BOOLEAN, NlpProblem<double>::VariableTypes::GENERIC}, boxv);
    std::cout << "Outer ball cut test with boolean variables II\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 1) && (boxv[0].mB[1] == 6));

    /** Testing linear cuts **/
    cut.mR = 1;
    cut.mC[0] = 2;
    cut.mC[1] = 1;
    cut.mType = Cut<double>::CutType::LINEAR;
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == true);
    cut.mR = -8;
    BNB_ASSERT(CutUtils<double>::isIn(cut, box) == false);

    /** Testing linear cuts with generic variables **/
    boxv.clear();
    CutUtils<double>::ApplyLinearCut(cut, box, std::vector<unsigned int> (), boxv);
    std::cout << "Linear cut test\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 4) && (boxv[0].mB[1] == 6));

    /** Testing linear cuts with boolean variables **/
    boxv.clear();
    CutUtils<double>::ApplyLinearCut(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::BOOLEAN, NlpProblem<double>::VariableTypes::BOOLEAN}, boxv);
    std::cout << "Linear cut test with boolean variables\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 1) && (boxv[0].mB[1] == 1));

    /** Testing linear cuts with integral variables **/
    cut.mR = -5.5;
    boxv.clear();
    CutUtils<double>::ApplyLinearCut(cut, box, std::vector<unsigned int>{NlpProblem<double>::VariableTypes::INTEGRAL, NlpProblem<double>::VariableTypes::INTEGRAL}, boxv);
    std::cout << "Linear cut test with integral variables I\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
    BNB_ASSERT(boxv.size() == 1);
    BNB_ASSERT((boxv[0].mA[0] == 0) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 2) && (boxv[0].mB[1] == 5));


    /** Testing inner boxed cut **/
    cut.mR = sqrt(2);
    cut.mC[0] = 0;
    cut.mC[1] = 0;
    cut.mType = Cut<double>::CutType::INNER_BALL;

    boxv.clear();
    CutUtils<double>::ApplyInnerBallCutBoxed(cut, box, std::vector<unsigned int>{},boxv);
    std::cout << "Boxed inner ball cut test\n";
    for (auto b : boxv) {
        std::cout << BoxUtils::toString(b);
        std::cout << "\n";
    }
#if 0 
    BNB_ASSERT(boxv.size() == 2);
    BNB_ASSERT((boxv[0].mA[0] == 1) && (boxv[0].mA[1] == 0) && (boxv[0].mB[0] == 12) && (boxv[0].mB[1] == 6));
    BNB_ASSERT((boxv[1].mA[0] == 0) && (boxv[1].mA[1] == 1) && (boxv[1].mB[0] == 1) && (boxv[1].mB[1] == 6));
#endif

    cut.mR = 5;
    cut.mC[0] = 0;
    cut.mC[1] = -3;
    box.mA[0] = 1;
    box.mA[1] = 1;
    box.mB[0] = 5;
    box.mB[1] = 3;
    Box<double> ibox(2);
    double v = CutUtils<double>::findMaxIntersection(cut, box, std::vector<unsigned int>{}, ibox);
    std::cout << BoxUtils::toString(ibox) << " with volume " << v << "\n";
    return 0;
}

