/* 
 * File:   cututils.hpp
 * Author: medved
 *
 * Created on November 2, 2014, 8:41 PM
 */

#ifndef CUTUTILS_HPP
#define	CUTUTILS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <functional>
#include <util/box/boxutils.hpp>
#include <problems/nlp/common/nlpproblem.hpp>

#include "cut.hpp"

template <class FT> class CutUtils {
public:

    /**
     * Produce a string representation of a cut
     * @param cut cut to print
     * @return the string representation
     */
    static std::string toString(const Cut<FT>& cut) {
        std::ostringstream os;
        os << "Cut type " << cut.mType << ", R = " << cut.mR;
        int n = cut.mC.size();
        os << ", centre = [";
        for (int i = 0; i < n; i++) {
            if (i != 0)
                os << ",";
            os << cut.mC[i];
        }
        os << "]";
        return os.str();
    }

#if 0

    /**
     * Appies the given cut to a box
     * @param cut to apply
     * @param box box (to which cuts are applied)
     * @param v resulting vector of boxes
     */
    static void ApplyCut(const Cut<FT>& cut, const Box<FT>& box, std::vector< Box<FT> >& v) {
        if (cut.mType == Cut<FT>::CutType::INNER_BALL) {
            ApplyInnerBallCutSimple(cut, box, v);
        } else if (cut.mType == Cut<FT>::CutType::OUTER_BALL) {
            ApplyOuterBallCut(cut, box, v);
        } else if (cut.mType == Cut<FT>::CutType::LINEAR) {
            ApplyLinearCut(cut, box, v);
        } else if (cut.mType == Cut<FT>::CutType::TOTAL) {
            /** DO NOTHING */
        }
    }
#endif

    /**
     * Checks if the box is inside the cut
     * @param cut cut
     * @param box box
     * @return true is a box in completely inside a cut, false otherwise
     */
    static bool isIn(const Cut<FT>& cut, const Box<FT>& box) {
        int n = box.mDim;
        bool rv;
        if (cut.mType == Cut<FT>::CutType::INNER_BALL) {
            FT qm = 0;
            for (int i = 0; i < n; i++) {
                qm += BNBSQR(BNBMAX(BNBABS(cut.mC[i] - box.mA[i]), BNBABS(cut.mC[i] - box.mB[i])));
            }
            if (qm <= cut.mR * cut.mR)
                rv = true;
            else
                rv = false;
        } else if (cut.mType == Cut<FT>::CutType::OUTER_BALL) {
            FT qm = 0;
            for (int i = 0; i < n; i++) {
                if (cut.mC[i] < box.mA[i]) {
                    qm += BNBSQR(box.mA[i] - cut.mC[i]);
                } else if (cut.mC[i] > box.mB[i]) {
                    qm += BNBSQR(cut.mC[i] - box.mB[i]);
                }
            }
            if (qm <= cut.mR * cut.mR)
                rv = false;
            else
                rv = true;
        } else if (cut.mType == Cut<FT>::CutType::LINEAR) {
            FT u = cut.mR;
            for (int i = 0; i < n; i++) {
                u += (cut.mC[i] > 0) ? (cut.mC[i] * box.mA[i]) : (cut.mC[i] * box.mB[i]);
            }
            if (u >= 0)
                rv = true;
            else
                rv = false;
        } else {
            rv = true;
        }
        return rv;
    }

    /**
     * Cuts the ball from a rectangle (simple version)
     * @param cut ball to cut from a box
     * @param box box to apply cut
     * @param vartypes types of variables (empty vector means all vars are generic)
     * @param v resulting vector
     * @return true if cut makes effect
     */
    static bool ApplyInnerBallCutSimple(const Cut<FT>& cut, const Box<FT>& box, const std::vector<unsigned int>& vartypes, std::vector< Box<FT> >& v) {
        int n = box.mDim;
        FT lmax = 0, amin = 0, bmax = 0;
        int imax;
        /* Search for best dimension to divide */
        FT rq = cut.mR * cut.mR;
        for (int i = 0; i < n; i++) {
            FT q = 0;
            for (int j = 0; j < n; j++) {
                if (j != i) {
                    FT p = BNBMAX(BNBABS(box.mA[j] - cut.mC[j]), BNBABS(box.mB[j] - cut.mC[j]));
                    q += p * p;
                }
            }
            if (q < rq) {
                FT h = sqrt(rq - q);
                /** Obsolete /
                FT a = BNBMAX(cut.mC[i] - h, box.mA[i]);
                FT b = BNBMIN(cut.mC[i] + h, box.mB[i]);
                 */
                FT a = cut.mC[i] - h;
                FT b = cut.mC[i] + h;
                if (!vartypes.empty()) {
                    if (vartypes[i] == NlpProblem<FT>::VariableTypes::INTEGRAL) {
                        a = floor(a);
                        b = ceil(b);
                    } else if (vartypes[i] == NlpProblem<FT>::VariableTypes::BOOLEAN) {
                        a = BNBBOOLFLOOR(a, FT);
                        b = BNBBOOLCEIL(b, FT);
                    }
                }
                /** Compute intersection **/
                FT an = BNBMAX(a, box.mA[i]);
                FT bn = BNBMIN(b, box.mB[i]);

                /** Check intersection **/
                if (an > bn)
                    continue;

                /** Check elimination **/
                if ((a < box.mA[i]) && (b > box.mB[i]))
                    return true;


                FT l = (bn - an) / (box.mB[i] - box.mA[i]);
                if (l > lmax) {
                    imax = i;
                    lmax = l;
                    amin = a;
                    bmax = b;
                }
            }
        }

        bool rv;
        if (lmax > 0) {
            if (amin >= box.mA[imax]) {
                Box<FT> b(n);
                BoxUtils::copy(box, b);
                b.mB[imax] = BNBMIN(amin, b.mB[imax]);
                v.push_back(b);
            }
            if (bmax <= box.mB[imax]) {
                Box<FT> b(n);
                BoxUtils::copy(box, b);
                b.mA[imax] = BNBMAX(bmax, b.mA[imax]);
                v.push_back(b);
            }
            rv = true;
        } else {
            rv = false;
            v.push_back(box);
        }
        return rv;
    }

    /**
     * Cuts the ball from a rectangle (Boxed based version)
     * @param cut ball to cut
     * @param box box to apply cut
     * @param vart variable types
     * @param v resulting vector
     */
    static void ApplyInnerBallCutBoxed(const Cut<FT>& cut, const Box<FT>& box, const std::vector<unsigned int>& vart, std::vector< Box<FT> >& v) {
        int n = box.mDim;
        Box<FT> cbox1(n);
        Box<FT> cbox2(n);

        FT vs, vp, vc;
        vs = BoxUtils::volume(box);
        vp = findPlainInnerBallIntersection(cut, box, vart, cbox1);
        if (vs != 0)
            vc = findMaxIntersection(cut, box, vart, cbox2);

        // TMP
#if 0
        std::cout << "\n======================\n";
        std::cout << "vs = " << vs << ", vp = " << vp << ", vc = " << vc << "\n";

        std::cout << "Cut: " << toString(cut) << "\n";
        std::cout << "Source box: " << BoxUtils::toString(box);
        std::cout << ", Plain box: " << BoxUtils::toString(cbox1);
        std::cout << ", Maximal box: " << BoxUtils::toString(cbox2);
        std::cout << "\n";
#endif
        // TMP

        std::vector< Box<FT> > nv;
        if (vs == 0) {
            /** Degenerate case **/
            BoxUtils::complement(box, cbox1, nv);
        } else {
            /** check whether assume plain cut instead of maximal cut **/
            auto muchbigger = [] (double a, double b) {
                return a >= 1.5 * b;
            };

            /** check whether it is worth to apply complement **/
            auto muchreduce = [] (double sourcev, double complementv) {
                return sourcev <= (8 * complementv);
            };

            Box<FT> cbox(n);
            if (muchbigger(vc, vp)) {
                if ((vc > 0) && muchreduce(vs, vc))
                    BoxUtils::complement(box, cbox2, nv);
                else
                    nv.push_back(box);
            } else {
                if (vp > 0)
                    BoxUtils::complement(box, cbox1, nv);
                else
                    nv.push_back(box);
            }
        }
        
        /** respect integral bounds **/
        auto integrify = [] (Box<FT>& box, const std::vector<unsigned int>& vart) {
            int n = box.mDim;
            for (int i = 0; i < n; i++) {
                if (vart[i] == NlpProblem<FT>::VariableTypes::INTEGRAL) {
                    box.mA[i] = ceil(box.mA[i]);
                    box.mB[i] = floor(box.mB[i]);
                    if(box.mA[i] > box.mB[i])
                        return false;
                }
            }
            return true;
        };

        if (!vart.empty()) {
            for (auto o : nv) {
                Box<FT> bx = o;
                if (integrify(bx, vart))
                    v.push_back(bx);
            }
        } else {
            v.insert(v.end(), nv.begin(), nv.end());
        }
    }

    /**
     * Finds basic intersection of an inner-ball cut and a box
     * @param cut inner ball cut
     * @param sbox source box
     * @param vart variable types
     * @param ibox resulting box
     * @return volume of the resulting box
     */
    static FT findPlainInnerBallIntersection(const Cut<FT>& cut, const Box<FT>& sbox, const std::vector<unsigned int>& vart, Box<FT>& ibox) {
        int n = sbox.mDim;
        FT lmax = 0, amin = 0, bmax = 0;
        int imax;
        /* Search for best dimension to divide */
        FT rq = cut.mR * cut.mR;
        for (int i = 0; i < n; i++) {
            FT q = 0;
            for (int j = 0; j < n; j++) {
                if (j != i) {
                    FT p = BNBMAX(BNBABS(sbox.mA[j] - cut.mC[j]), BNBABS(sbox.mB[j] - cut.mC[j]));
                    q += p * p;
                }
            }
            if (q < rq) {
                FT h = sqrt(rq - q);
                FT a = cut.mC[i] - h;
                FT b = cut.mC[i] + h;

                if (!vart.empty()) {
                    const FT alpha = 0.5;
                    if (vart[i] == NlpProblem<FT>::VariableTypes::INTEGRAL) {
                        FT fa = floor(a);
                        if (a != fa)
                            a = fa + alpha * (a - fa);
                        FT cb = ceil(b);
                        if (b != cb)
                            b = cb - alpha * (cb - b);
                    }
                }
                /** Compute intersection **/
                FT an = BNBMAX(a, sbox.mA[i]);
                FT bn = BNBMIN(b, sbox.mB[i]);

                /** Check intersection **/
                if (an > bn)
                    continue;

                FT l = (bn - an) / (sbox.mB[i] - sbox.mA[i]);
                if (l > lmax) {
                    imax = i;
                    lmax = l;
                    amin = an;
                    bmax = bn;
                }
            }
        }
        FT v;
        if (lmax > 0) {
            BoxUtils::copy(sbox, ibox);
            ibox.mA[imax] = amin;
            ibox.mB[imax] = bmax;
            v = BoxUtils::volume(ibox);
        } else {
            v = 0;
        }
        return v;
    }

    /**
     * Find a box inside cut maximizing intersection with the reduced box
     * @param cut spherical cut
     * @param sbox box to reduce
     * @param ibox resulting box
     * @param vart variable types
     * @return volume on intersection
     */
    static FT findMaxIntersection(const Cut<FT>& cut, const Box<FT>& sbox, const std::vector<unsigned int>& vart, Box<FT>& ibox) {
        FT v = 0;
        BNB_ASSERT(cut.mType == Cut<FT>::CutType::INNER_BALL);
        int n = sbox.mDim;
        BNB_ASSERT(ibox.mDim == n);
        BNB_ASSERT(cut.mC.size() == n);
        int maxit = 10 * 2 ^ n;
        Box<FT> tbox(n);
        FT lowb[n];

        /** Fill in lower bound **/
        for (int i = 0; i < n; i++) {
            lowb[i] = BNBMAX(0, BNBMAX(sbox.mA[i] - cut.mC[i], cut.mC[i] - sbox.mB[i]));
        }
        /** Compute reminder **/
        FT qref = cut.mR * cut.mR;
        for (int i = 0; i < n; i++)
            qref -= lowb[i] * lowb[i];

        if (qref <= 0)
            return 0;

        /** Iterate Monte-Carlo **/
        for (int I = 0; I < maxit; I++) {
            FT u = 1;
            FT q = qref;
            /** Compute random subbox and its volume **/
            for (int i = 0; i < n; i++) {
                FT s;

                q += lowb[i] * lowb[i];
                FT h = sqrt(q);
                FT z = h - lowb[i];
                // TMP
                if(z < 0) {
                    std::cout << "i = " << i << ", lowb[i] = " << lowb[i] << ", h = " << h << ", z = " << z << "\n";
                    std::cout << "cut = " << toString(cut) << "box = " << BoxUtils::toString(sbox) << "\n";
                }
                // TMP
                BNB_ASSERT(z >= 0);
                FT r;
                if (i == n - 1)
                    r = z;
                else
                    r = ((FT(rand()) / (FT) RAND_MAX)) * z;
                s = lowb[i] + r;
                q -= s * s;
                //BNB_ASSERT(q >= 0);
                FT a = cut.mC[i] - s;
                FT b = cut.mC[i] + s;
                if (!vart.empty()) {
                    const FT alpha = 0.5;
                    if (vart[i] == NlpProblem<FT>::VariableTypes::INTEGRAL) {
                        FT fa = floor(a);
                        if (a != fa)
                            a = fa + alpha * (a - fa);
                        FT cb = ceil(b);
                        if (b != cb)
                            b = cb - alpha * (cb - b);
                    }
                }
                FT ar = BNBMAX(a, sbox.mA[i]);
                FT br = BNBMIN(b, sbox.mB[i]);
                if (ar >= br) {
                    u = -1;
                    break;
                } else
                    u *= br - ar;
                tbox.mA[i] = ar;
                tbox.mB[i] = br;
                //tbox.mA[i] = a;
                //tbox.mB[i] = b;
            }
            if (u > v) {
                v = u;
                BoxUtils::copy(tbox, ibox);
            }
        }

        return v;

    }

    /**
     * Find a box inside cut maximizing intersection with the reduced box
     * @param cut spherical cut
     * @param sbox box to reduce
     * @param ibox resulting box
     * @param vart variable types
     * @return volume on intersection
     */
    static FT old_findMaxIntersection(const Cut<FT>& cut, const Box<FT>& sbox, const std::vector<unsigned int>& vart, Box<FT>& ibox) {
        FT v = 0;
        BNB_ASSERT(cut.mType == Cut<FT>::CutType::INNER_BALL);
        int n = sbox.mDim;
        BNB_ASSERT(ibox.mDim == n);
        BNB_ASSERT(cut.mC.size() == n);
        int maxit = 10 * 2 ^ n;
        Box<FT> tbox(n);

        /** Iterate Monte-Carlo **/
        for (int i = 0; i < maxit; i++) {
            FT u = 1;
            FT r = cut.mR;
            /** Compute random subbox and its volume **/
            for (int j = 0; j < n; j++) {
                FT s;
                if (j == n - 1)
                    s = r;
                else
                    s = ((FT(rand()) / (FT) RAND_MAX)) * r;
                FT a = cut.mC[j] - s;
                FT b = cut.mC[j] + s;
                if (!vart.empty()) {
                    if (vart[j] == NlpProblem<FT>::VariableTypes::INTEGRAL) {
                        a = floor(a);
                        b = ceil(b);
                        //std::cout << i << ": j = " << j << ", a = " << a << ", b = " << b << "\n";
                    }
                    if (vart[j] == NlpProblem<FT>::VariableTypes::BOOLEAN) {
                        a = BNBBOOLFLOOR(a, FT);
                        b = BNBBOOLCEIL(b, FT);
                    }
                }
                FT ar = BNBMAX(a, sbox.mA[j]);
                FT br = BNBMIN(b, sbox.mB[j]);
                if (ar >= br) {
                    u = -1;
                    break;
                } else
                    u *= br - ar;
                //tbox.mA[j] = ar;
                //tbox.mB[j] = br;
                tbox.mA[j] = a;
                tbox.mB[j] = b;
                r = sqrt(r * r - s * s);
            }
            if (u > v) {
                v = u;
                BoxUtils::copy(tbox, ibox);
            }
        }

        return v;

    }

    /**
     * Apply outer ball cut to a box
     * @param cut cut to apply
     * @param box box to apply cut
     * @param vartypes variable types
     * @param v resulting vector
     */
    static void ApplyOuterBallCut(const Cut<FT>& cut, const Box<FT>& box, const std::vector<unsigned int>& vartypes, std::vector< Box<FT> >& v) {
        int n = box.mDim;
        Box<FT> nbox(n);
        if (isIn(cut, box))
            return;
        for (int i = 0; i < n; i++) {
            FT a = cut.mC[i] - cut.mR;
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::INTEGRAL)) {
                a = ceil(a);
            }
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::BOOLEAN)) {
                a = BNBBOOLCEIL(a, FT);
            }
            nbox.mA[i] = BNBMAX(a, box.mA[i]);
            FT b = cut.mC[i] + cut.mR;
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::INTEGRAL)) {
                b = floor(b);
            }
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::BOOLEAN)) {
                b = BNBBOOLFLOOR(b, FT);
            }
            nbox.mB[i] = BNBMIN(b, box.mB[i]);
            if (nbox.mA[i] > nbox.mB[i]) {
                return;
            }
        }
        v.push_back(nbox);
    }

    /**
     * Applies linear cut to a box
     * @param cut cut to apply
     * @param box box to cut
     * @param v resulting vector
     */
    static void ApplyLinearCut(const Cut<FT>& cut, const Box<FT>& box, const std::vector<unsigned int>& vartypes, std::vector< Box<FT> >& v) {
        int n = box.mDim;
        Box<FT> nbox(n);
        FT u = cut.mR;
        /** Find minimum of the linear function in a box **/
        for (int i = 0; i < n; i++) {
            u += (cut.mC[i] > 0) ? (cut.mC[i] * box.mA[i]) : (cut.mC[i] * box.mB[i]);
        }
        /** if the minimum is >=0 the cut covers the whole box **/
        if (u >= 0)
            return;

        /** Find intersection **/
        for (int i = 0; i < n; i++) {
            FT p = cut.mC[i];
            FT a, b;
            if (p > 0) {
                a = box.mA[i];
                b = BNBMIN(box.mB[i], box.mA[i] - u / p);
            } else if (p < 0) {
                a = BNBMAX(box.mA[i], box.mB[i] - u / p);
                b = box.mB[i];
            } else {
                a = box.mA[i];
                b = box.mB[i];
            }
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::INTEGRAL)) {
                a = ceil(a);
                b = floor(b);
            }
            if ((!vartypes.empty()) && (vartypes[i] == NlpProblem<FT>::VariableTypes::BOOLEAN)) {
                a = BNBBOOLCEIL(a, FT);
                b = BNBBOOLFLOOR(b, FT);
            }
            if (a <= b) {
                nbox.mA[i] = a;
                nbox.mB[i] = b;
            } else
                return;
        }
        v.push_back(nbox);
    }



};

#endif	/* CUTUTILS_HPP */

