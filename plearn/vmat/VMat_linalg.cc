// -*- C++ -*-

// VMat_linalg.cc
//
// Copyright (C) 2004 Pascal Vincent
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************
 * $Id$
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_linalg.cc */


// From Boost
#include <boost/scoped_ptr.hpp>

// From PLearn
#include <plearn/base/ProgressBar.h>
#include "VMat_linalg.h"
#include <plearn/math/TMat_maths.h>
#include "VMat.h"
#include "ExtendedVMatrix.h"
#include <plearn/math/plapack.h>      //!< For solveLinearSystem.

namespace PLearn {
using namespace std;

Mat transposeProduct(VMat m)
{
    Mat result(m.width(),m.width());

    Vec v(m.width());
    Mat vrowmat = rowmatrix(v);

    for(int i=0; i<m.length(); i++)
    {
        m->getRow(i,v);
        transposeProductAcc(result, vrowmat,vrowmat);
    }
    return result;
}

Mat transposeProduct(VMat m1, VMat m2)
{
    if(m1.length()!=m2.length())
        PLERROR("in Mat transposeProduct(VMat m1, VMat m2) arguments have incompatible dimensions");

    Mat result(m1.width(),m2.width());

    Vec v1(m1.width());
    Vec v2(m2.width());
    Mat v1rowmat = rowmatrix(v1);
    Mat v2rowmat = rowmatrix(v2);

    for(int i=0; i<m1.length(); i++)
    {
        m1->getRow(i,v1);
        m2->getRow(i,v2);
        transposeProductAcc(result, v1rowmat,v2rowmat);
    }
    return result;
}

Vec transposeProduct(VMat m1, Vec v2)
{
    if(m1.length()!=v2.length())
        PLERROR("in Mat transposeProduct(VMat m1, Vec v2) arguments have incompatible dimensions");

    Vec result(m1.width(),1);
    result.clear();

    Vec v1(m1.width());
    for(int i=0; i<m1.length(); i++)
    {
        m1->getRow(i,v1);
        result += v1 * v2[i];
    }
    return result;
}

Mat productTranspose(VMat m1, VMat m2)
{
    if(m1.width()!=m2.width())
        PLERROR("in Mat productTranspose(VMat m1, VMat m2) arguments have incompatible dimensions");

    int m1l = (m1.length());
    int m2l = (m2.length());
    int w = (m1.width());
    Mat result(m1l,m2l);

    Vec v1(w);
    Vec v2(w);

    for(int i=0; i<m1l; i++)
    {
        m1->getRow(i,v1);
        for(int j=0; j<m2l; j++)
        {
            m2->getRow(j,v2);
            result(i,j) = dot(v1,v2);
        }
    }
    return result;
}

Mat product(Mat m1, VMat m2)
{
    if(m1.width()!=m2.length())
        PLERROR("in Mat product(VMat m1, VMat m2) arguments have incompatible dimensions");

    Mat result(m1.length(),m2.width());
    result.clear();

    Vec v2(m2.width());
    Mat v2rowmat = rowmatrix(v2);

    for(int i=0; i<m1.width(); i++)
    {
        m2->getRow(i,v2);
        productAcc(result, m1.column(i), v2rowmat);
    }
    return result;
}

VMat transpose(VMat m1)
{
    return VMat(transpose(m1.toMat()));
}

real linearRegression(
    VMat inputs, VMat outputs, real weight_decay, Mat theta_t,
    bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY,
    real& sum_squared_Y, Vec& outputwise_sum_squared_Y,
    bool return_squared_loss, int verbose_every, bool cholesky,
    int apply_decay_from)
{
    if (outputs.length()!=inputs.length())
        PLERROR("linearRegression: inputs.length()=%d while outputs.length()=%d",inputs.length(),outputs.length());
    if (theta_t.length()!=inputs.width() || theta_t.width()!=outputs.width())
        PLERROR("linearRegression: theta_t(%d,%d) should be (%dx%d)",
                theta_t.length(),theta_t.width(),inputs.width(),outputs.width());

    int inputsize = inputs.width();
    int targetsize = outputs.width();

    if(XtX.length()!=inputsize || XtX.width()!=inputsize)
        PLERROR("In linearRegression: XtX should have dimensions %dx%d (inputs.width())x(inputs.width())",
                inputsize,inputsize);
    if(XtY.length()!=inputsize || XtY.width()!=targetsize)
        PLERROR("In linearRegression: XtY should have dimensions %dx%d (inputs.width())x(outputs.width())",
                inputsize,targetsize);

    if(!use_precomputed_XtX_XtY) // then compute them
    {
        VMat X = inputs; // new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
        VMat Y = outputs;
        outputwise_sum_squared_Y.resize(targetsize);
        outputwise_sum_squared_Y.fill(0.0);

        // *************
        // Do efficiently the following:
        // XtX << transposeProduct(X); // '<<' to copy elements (as transposeProduct returns a new matrix)
        // XtY << transposeProduct(X,Y); // same thing (remember '=' for Mat never copies elements)
        XtX.clear();
        XtY.clear();
        sum_squared_Y=0;
        Vec x(X.width());
        Vec y(Y.width());
        int l=X.length();

        // Display progress bar iff we have some verbosity
        boost::scoped_ptr<ProgressBar> pb(
            verbose_every?
            new ProgressBar("Performing Unweighted Linear Regression", l) : 0);

        for(int i=0; i<l; i++)
        {
            if (pb)
                pb->update(i);
            
            X->getRow(i,x);
            Y->getRow(i,y);
            externalProductAcc(XtX, x,x);
            externalProductAcc(XtY, x,y);
            sum_squared_Y += dot(y,y);
            y *= y;                              //!< element-wise square
            outputwise_sum_squared_Y += y;
        }
        // *************
    }

    // add weight_decay on the diagonal of XX' (except for the bias)
    for (int i=apply_decay_from; i<XtX.length(); i++)
        XtX(i,i) += weight_decay;

    // VMat(XtX)->savePMAT("plXtX.pmat");
    // VMat(XtY)->savePMAT("plXtY.pmat");

    if (cholesky) {
        // now solve by Cholesky decomposition
        solveLinearSystemByCholesky(XtX,XtY,theta_t);
    } else {
        theta_t = solveLinearSystem(XtX, XtY);
    }

    real squared_loss=0;
    if (return_squared_loss)
    {
        // squared loss = sum_{ij} theta_{ij} (X'W X theta')_{ij} + sum_{t,i} Y_{ti}^2 - 2 sum_{ij} theta_{ij} (X'W Y)_{ij}
        Mat M(inputsize,targetsize);
        product(M,XtX,theta_t);
        squared_loss += dot(M,theta_t); //
        squared_loss += sum_squared_Y;
        squared_loss -= 2*dot(XtY,theta_t);
    }
    return squared_loss/inputs.length();
}

Mat linearRegression(VMat inputs, VMat outputs, real weight_decay, bool include_bias)
{
    int n = inputs.width()+(include_bias?1:0);
    int n_outputs = outputs.width();
    Mat XtX(n,n);
    Mat XtY(n,n_outputs);
    Mat theta_t(n,n_outputs);
    real sy=0;
    Vec outputwise_sum_squared_Y;
    if(include_bias)
        inputs = new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
    linearRegression(inputs, outputs, weight_decay, theta_t,
                     false, XtX, XtY, sy, outputwise_sum_squared_Y);
    return theta_t;
}


real weightedLinearRegression(
    VMat inputs, VMat outputs, VMat gammas, real weight_decay, Mat theta_t,
    bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY,
    real& sum_squared_Y, Vec& outputwise_sum_squared_Y,
    real& sum_gammas, bool return_squared_loss, int verbose_every,
    bool cholesky, int apply_decay_from)
{
    int inputsize = inputs.width();
    int targetsize = outputs.width();
    if (outputs.length()!=inputs.length())
        PLERROR("linearRegression: inputs.length()=%d while outputs.length()=%d",inputs.length(),outputs.length());
    if (theta_t.length()!=inputsize || theta_t.width()!=targetsize)
        PLERROR("linearRegression: theta_t(%d,%d) should be (%dx%d)",
                theta_t.length(),theta_t.width(),inputsize,targetsize);

    if(XtX.length()!=inputsize || XtX.width()!=inputsize)
        PLERROR("In linearRegression: XtX should have dimensions %dx%d (inputs.width())x(inputs.width())",
                inputsize,inputsize);
    if(XtY.length()!=inputsize || XtY.width()!=targetsize)
        PLERROR("In linearRegression: XtY should have dimensions %dx%d (inputs.width())x(outputs.width())",
                inputsize,targetsize);

    int l=inputs.length();
    if(!use_precomputed_XtX_XtY) // then compute them
    {
        XtX.clear();
        XtY.clear();
        // VMat X = new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
        VMat X = inputs;
        VMat Y = outputs;
        outputwise_sum_squared_Y.resize(targetsize);
        outputwise_sum_squared_Y.fill(0.0);

        sum_squared_Y= 0.0;
        sum_gammas= 0.0;

        // Prepare to comnpute weighted XtX and XtY
        Vec x(X.width());
        Vec y(Y.width());
        real gamma_i;

        // Display progress bar iff we have some verbosity
        boost::scoped_ptr<ProgressBar> pb(
            verbose_every?
            new ProgressBar("Performing Weighted Linear Regression", l) : 0);

        for(int i=0; i<l; i++)
        {
            if (pb)
                pb->update(i);

            X->getRow(i,x);
            Y->getRow(i,y);
            gamma_i = gammas(i,0);
            externalProductScaleAcc(XtX, x,x,gamma_i);
            externalProductScaleAcc(XtY, x,y,gamma_i);
            sum_squared_Y += gamma_i * dot(y,y);
            sum_gammas += gamma_i;
            y *= y;                                //!< element-wise square
            outputwise_sum_squared_Y += y;
        }
    }

    // add weight_decay on the diagonal of XX' (except for the bias)
    for (int i=apply_decay_from; i<XtX.length(); i++)
        XtX(i,i) += weight_decay;

    if (cholesky) {
        // now solve by Cholesky decomposition
        solveLinearSystemByCholesky(XtX,XtY,theta_t);
    } else {
        theta_t = solveLinearSystem(XtX, XtY);
    }

    real squared_loss=0;
    if (return_squared_loss)
    {
        // squared loss = sum_{ij} theta_{ij} (X'W X theta')_{ij} + sum_{t,i} gamma_t*Y_{ti}^2 - 2 sum_{ij} theta_{ij} (X'W Y)_{ij}
        Mat M(inputsize,targetsize);
        product(M,XtX,theta_t);
        squared_loss += dot(M,theta_t); //
        squared_loss += sum_squared_Y;
        squared_loss -= 2*dot(XtY,theta_t);
    }
    // return squared_loss/l;
    // perr << "linreg/l: " << squared_loss << "/" << l << "=" << squared_loss/l << endl;
    // perr << "linreg/sg: " << squared_loss << "/" << sum_gammas << "=" << squared_loss/sum_gammas << endl;
    return squared_loss/sum_gammas;
}

//! Version that does all the memory allocations of XtX, XtY and
//! theta_t. Returns theta_t
Mat weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas,
                             real weight_decay, bool include_bias)
{
    int n = inputs.width()+(include_bias?1:0);
    int n_outputs = outputs.width();
    Mat XtX(n,n);
    Mat XtY(n,n_outputs);
    Mat theta_t(n,n_outputs);
    real sy=0;
    real sg=0;
    Vec outputwise_sum_squared_Y;
    if(include_bias)
        inputs = new ExtendedVMatrix(inputs,0,0,1,0,1.0); // prepend a first column of ones
    weightedLinearRegression(inputs, outputs, gammas, weight_decay, theta_t,
                             false, XtX, XtY, sy, outputwise_sum_squared_Y,
                             sg);
    return theta_t;
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
