/* 
 * File:   seqsolver.cpp
 * Author: posypkin
 *
 * Created on September 18, 2012, 11:54 AM
 */
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <kernel/dmmemory/seqsched.hpp>
#include <kernel/dmmemory/bnbseqsolver.hpp>
#include <kernel/dmmemory/simptracer.hpp>
#include <problems/onedknap/common/knaputils.hpp>
#include "hsutils.hpp"
#include "u0bounder.hpp"
#include "hsresolver.hpp"
#include "lpbounder.hpp"
#include "mtbounder.hpp"


int main(int argc, char** argv) {
    KnapData<int> kd;
    if (argc != 3)
        BNB_ERROR_REPORT("usage: seqsolver.exe <data_file> <bounding_method>\n");
    KnapUtils<int>::readJSONfromFile(argv[1], kd);
    int bm = atoi(argv[2]);

    KnapUtils<int>::sort(kd);
//    printf("%s\n", KnapUtils<int>::writeJson(kd).c_str());
    HSResolver<int> hsr(&kd);
    U0Bounder<int> bnd;
    hsr.addBounder(&bnd);
    LPBounder<int> lpbnd;
    hsr.addBounder(&lpbnd);
    MTBounder<int> mtbnd;
    hsr.addBounder(&mtbnd);

    HSSub<int> sub;
    sub = HSUtils<int>::initialSubproblem(kd);
    hsr.fetchSub(sub);
    SeqSched ss;
    ss.setControlInterval(10000);
    ss.setSearchStrategyParams(SearchStrategies::DFS, 0, 1000);
    ss.setHeuristic(0);
    ss.setBoundingMethod(bm);
    BNBSeqSolver solver(&ss, &hsr);
    //SchedTracer st(1);
    SimpTracer st;
    solver.setTracer(&st);
    solver.solve();
    HSSol<int> rsol = HSUtils<int>::reorderSolution(kd, hsr.getRecord());
    printf("optimal solution: %s\n", HSUtils<int>::HSSol2string(rsol).c_str());
    printf("%d steps performed\n", st.getInfo().mNSteps);
}

