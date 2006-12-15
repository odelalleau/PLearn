// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
 * This file is part of the PLearn library.
 ******************************************************* */

#include "Kernel.h"
#include <plearn/base/lexical_cast.h>
#include <plearn/base/tostring.h>
#include <plearn/base/ProgressBar.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

using namespace std;
  
PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Kernel,
                                 "A Kernel is a real-valued function K(x,y).",
                                 ""
    );
Kernel::~Kernel() {}

////////////
// Kernel //
////////////
Kernel::Kernel(bool is__symmetric)
    : lock_xi(false),
      lock_xj(false),
      lock_k_xi_x(false),
      data_inputsize(-1),
      gram_matrix_is_cached(false),
      sparse_gram_matrix_is_cached(false),
      n_examples(-1),
      cache_gram_matrix(false),
      is_symmetric(is__symmetric),
      report_progress(0)
{}

////////////////////
// declareOptions //
////////////////////
void Kernel::declareOptions(OptionList &ol)
{

    // Build options.

    declareOption(ol, "is_symmetric", &Kernel::is_symmetric, OptionBase::buildoption,
                  "Whether this kernel is symmetric or not.");
  
    declareOption(ol, "report_progress", &Kernel::report_progress, OptionBase::buildoption,
                  "If set to 1, a progress bar will be displayed when computing the Gram matrix,\n"
                  "or for other possibly costly operations.");
  
    declareOption(ol, "specify_dataset", &Kernel::specify_dataset, OptionBase::buildoption,
                  "If set, then setDataForKernelMatrix will be called with this dataset at build time");

    declareOption(ol, "cache_gram_matrix", &Kernel::cache_gram_matrix, OptionBase::buildoption,
                  "If set to 1, the Gram matrix will be cached in memory to avoid multiple computations.");

    // Learnt options.
  
    declareOption(ol, "data_inputsize", &Kernel::data_inputsize, OptionBase::learntoption,
                  "The inputsize of 'data' (if -1, is set to data.width()).");
  
    declareOption(ol, "n_examples", &Kernel::n_examples, OptionBase::learntoption,
                  "The number of examples in 'data'.");
  
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void Kernel::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void Kernel::build_() {
    if (specify_dataset) {
        this->setDataForKernelMatrix(specify_dataset);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void Kernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(evaluate_xi, copies);
    deepCopyField(evaluate_xj, copies);
    deepCopyField(k_xi_x, copies);
    deepCopyField(data, copies);
    deepCopyField(gram_matrix, copies);
    deepCopyField(sparse_gram_matrix, copies);
    deepCopyField(specify_dataset, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void Kernel::setDataForKernelMatrix(VMat the_data)
{ 
    data = the_data; 
    if (data) {
        data_inputsize = data->inputsize();
        if (data_inputsize == -1) {
            // Default value when no inputsize is specified.
            data_inputsize = data->width();
        }
        n_examples = data->length();
    } else {
        data_inputsize = 0;
        n_examples = 0;
    }
    gram_matrix_is_cached = false;
    sparse_gram_matrix_is_cached = false;
}

////////////////////////////
// addDataForKernelMatrix //
////////////////////////////
void Kernel::addDataForKernelMatrix(const Vec& newRow)
{
    try{
        data->appendRow(newRow);
    }
    catch(const PLearnError&){
        PLERROR("Kernel::addDataForKernelMatrix: if one intends to use this method,\n"
                "he must provide a data matrix for which the appendRow method is\n"
                "implemented.");
    }
}

//////////////////
// evaluate_i_j //
//////////////////
real Kernel::evaluate_i_j(int i, int j) const {
    static real result;
    if (lock_xi || lock_xj) {
        // This should not happen often, but you never know...
        Vec xi(data_inputsize);
        Vec xj(data_inputsize);
        data->getSubRow(i, 0, xi);
        data->getSubRow(j, 0, xj);
        return evaluate(xi, xj);
    } else {
        lock_xi = true;
        lock_xj = true;
        evaluate_xi.resize(data_inputsize);
        evaluate_xj.resize(data_inputsize);
        data->getSubRow(i, 0, evaluate_xi);
        data->getSubRow(j, 0, evaluate_xj);
        result = evaluate(evaluate_xi, evaluate_xj);
        lock_xi = false;
        lock_xj = false;
        return result;
    }
}


//////////////////
// evaluate_i_x //
//////////////////
real Kernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const  {
    static real result;
    if (lock_xi) {
        Vec xi(data_inputsize);
        data->getSubRow(i, 0, xi);
        return evaluate(xi, x);
    } else {
        lock_xi = true;
        evaluate_xi.resize(data_inputsize);
        data->getSubRow(i, 0, evaluate_xi);
        result = evaluate(evaluate_xi, x);
        lock_xi = false;
        return result;
    }
}

//////////////////
// evaluate_x_i //
//////////////////
real Kernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
    static real result;
    if(is_symmetric)
        return evaluate_i_x(i,x,squared_norm_of_x);
    else {
        if (lock_xi) {
            Vec xi(data_inputsize);
            data->getSubRow(i, 0, xi);
            return evaluate(x, xi);
        } else {
            lock_xi = true;
            evaluate_xi.resize(data_inputsize);
            data->getSubRow(i, 0, evaluate_xi);
            result = evaluate(x, evaluate_xi);
            lock_xi = false;
            return result;
        }
    }
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real Kernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
    return evaluate_i_x(i, x, squared_norm_of_x);
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real Kernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) const {
    return evaluate_x_i(x, i, squared_norm_of_x);
}

//////////////////////
// evaluate_all_i_x //
//////////////////////
void Kernel::evaluate_all_i_x(const Vec& x, const Vec& k_xi_x, real squared_norm_of_x, int istart) const {
    k_xi_x[0] = evaluate_i_x_again(istart, x, squared_norm_of_x, true);
    int i_max = istart + k_xi_x.length();
    for (int i = istart + 1; i < i_max; i++) {
        k_xi_x[i] = evaluate_i_x_again(i, x, squared_norm_of_x);
    }
}

//////////////////////
// evaluate_all_x_i //
//////////////////////
void Kernel::evaluate_all_x_i(const Vec& x, const Vec& k_x_xi, real squared_norm_of_x, int istart) const {
    k_x_xi[0] = evaluate_x_i_again(x, istart, squared_norm_of_x, true);
    int i_max = istart + k_x_xi.length();
    for (int i = istart + 1; i < i_max; i++) {
        k_x_xi[i] = evaluate_x_i_again(x, i, squared_norm_of_x);
    }
}

//////////////
// isInData //
//////////////
bool Kernel::isInData(const Vec& x, int* i) const {
    return data->find(x, 1e-8, i);
}

/////////////////////////////
// computeNearestNeighbors //
/////////////////////////////
void Kernel::computeNearestNeighbors(const Vec& x, Mat& k_xi_x_sorted, int knn) const {
    Vec k_val;
    bool unlock = true;
    if (lock_k_xi_x) {
        k_val.resize(n_examples);
        unlock = false;
    }
    else {
        lock_k_xi_x = true;
        k_xi_x.resize(n_examples);
        k_val = k_xi_x;
    }
    k_xi_x_sorted.resize(n_examples, 2);
    // Compute the distance from x to all training points.
    evaluate_all_i_x(x, k_val);
    // Find the knn nearest neighbors.
    for (int i = 0; i < n_examples; i++) {
        k_xi_x_sorted(i,0) = k_val[i];
        k_xi_x_sorted(i,1) = real(i);
    }
    partialSortRows(k_xi_x_sorted, knn);
    if (unlock)
        lock_k_xi_x = false;
}

///////////////////////
// computeGramMatrix //
///////////////////////
void Kernel::computeGramMatrix(Mat K) const
{
    if (!data)
        PLERROR("Kernel::computeGramMatrix should be called only after setDataForKernelMatrix");
    if (!is_symmetric)
        PLERROR("In Kernel::computeGramMatrix - Currently not implemented for non-symmetric kernels");
    if (cache_gram_matrix && gram_matrix_is_cached) {
        K << gram_matrix;
        return;
    }
    int l=data->length();
    int m=K.mod();
    PP<ProgressBar> pb;
    int count = 0;
    if (report_progress)
        pb = new ProgressBar("Computing Gram matrix for " + classname(), (l * (l + 1)) / 2);
    real Kij;
    real* Ki;
    real* Kji_;
    for (int i=0;i<l;i++)
    {
        Ki = K[i];
        Kji_ = &K[0][i];
        for (int j=0; j<=i; j++,Kji_+=m)
        {
            Kij = evaluate_i_j(i,j);
            *Ki++ = Kij;
            if (j<i)
                *Kji_ = Kij;
        }
        if (report_progress) {
            count += i + 1;
            PLASSERT( pb.isNotNull() );
            pb->update(count);
        }
    }
    if (cache_gram_matrix) {
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}

/////////////////////////////
// computeSparseGramMatrix //
/////////////////////////////
void Kernel::computeSparseGramMatrix(TVec<Mat> K) const
{
    if (!data) PLERROR("Kernel::computeSparseGramMatrix should be called only after setDataForKernelMatrix");
    if (!is_symmetric)
        PLERROR("In Kernel::computeGramMatrix - Currently not implemented for non-symmetric kernels");
    if (cache_gram_matrix && sparse_gram_matrix_is_cached) {
        for (int i = 0; i < sparse_gram_matrix.length(); i++) {
            K[i].resize(sparse_gram_matrix[i].length(), 2);
            K[i] << sparse_gram_matrix[i];
        }
        return;
    }
    if (cache_gram_matrix && gram_matrix_is_cached) {
        // We can obtain the sparse Gram matrix from the full one.
        int n = K.length();
        Vec row(2);
        for (int i = 0; i < n; i++) {
            Mat& K_i = K[i];
            K_i.resize(0,2);
            real* gram_ij = gram_matrix[i];
            for (int j = 0; j < n; j++, gram_ij++)
                if (!fast_exact_is_equal(*gram_ij, 0)) {
                    row[0] = j;
                    row[1] = *gram_ij;
                    K_i.appendRow(row);
                }
        }
        // TODO Use a method to avoid code duplication below.
        sparse_gram_matrix.resize(n);
        for (int i = 0; i < n; i++) {
            sparse_gram_matrix[i].resize(K[i].length(), 2);
            sparse_gram_matrix[i] << K[i];
        }
        sparse_gram_matrix_is_cached = true;
        return;
    }
    int l=data->length();
    PP<ProgressBar> pb;
    int count = 0;
    if (report_progress) {
        pb = new ProgressBar("Computing sparse Gram matrix for " + classname(), (l * (l + 1)) / 2);
    }
    Vec j_and_Kij(2);
    for (int i = 0; i < l; i++)
        K[i].resize(0,2);
    for (int i=0;i<l;i++)
    {
        for (int j=0; j<=i; j++)
        {
            j_and_Kij[1] = evaluate_i_j(i,j);
            if (!fast_exact_is_equal(j_and_Kij[1], 0)) {
                j_and_Kij[0] = j;
                K[i].appendRow(j_and_Kij);
                if (j < i) {
                    j_and_Kij[0] = i;
                    K[j].appendRow(j_and_Kij);
                }
            }
        }
        if (pb) {
            count += i + 1;
            pb->update(count);
        }
    }
    if (cache_gram_matrix) {
        sparse_gram_matrix.resize(l);
        for (int i = 0; i < l; i++) {
            sparse_gram_matrix[i].resize(K[i].length(), 2);
            sparse_gram_matrix[i] << K[i];
        }
        sparse_gram_matrix_is_cached = true;
    }
}


/////////////////////////////////
// computeGramMatrixDerivative //
/////////////////////////////////
void Kernel::computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                         real epsilon) const
{
    // This function is conceptually const, but the evaluation by finite
    // differences in a generic way requires some change-options, which
    // formally require a const-away cast.
    Kernel* This = const_cast<Kernel*>(this);
    bool old_cache = cache_gram_matrix;
    This->cache_gram_matrix = false;

    if (!data)
        PLERROR("Kernel::computeGramMatrixDerivative should be called only after "
                "setDataForKernelMatrix");

    int W = dataInputsize();
    KD.resize(W,W);
    Mat KDminus(W,W);

    string cur_param_str = getOption(kernel_param);
    real cur_param = lexical_cast<real>(cur_param_str);

    // Compute the positive part of the finite difference
    This->changeOption(kernel_param, tostring(cur_param+epsilon));
    computeGramMatrix(KD);

    // Compute the negative part of the finite difference
    This->changeOption(kernel_param, tostring(cur_param-epsilon));
    computeGramMatrix(KDminus);

    // Finalize computation
    KD -= KDminus;
    KD /= 2.*epsilon;

    This->changeOption(kernel_param, cur_param_str);
    This->cache_gram_matrix = old_cache;
}


///////////////////
// setParameters //
///////////////////
void Kernel::setParameters(Vec paramvec)
{ PLERROR("setParameters(Vec paramvec) not implemented for this kernel"); }

///////////////////
// getParameters //
///////////////////
Vec Kernel::getParameters() const
{ return Vec(); }

/////////////
// hasData //
/////////////
bool Kernel::hasData() {
    return data.isNotNull();
}

///////////
// apply //
///////////
void Kernel::apply(VMat m1, VMat m2, Mat& result) const
{
    result.resize(m1->length(), m2->length());
    int m1w = m1->inputsize();
    if (m1w == -1) { // No inputsize specified: using width instead.
        m1w = m1->width();
    }
    int m2w = m2->inputsize();
    if (m2w == -1) {
        m2w = m2->width();
    }
    Vec m1_i(m1w);
    Vec m2_j(m2w);
    PP<ProgressBar> pb;
    bool easy_case = (is_symmetric && m1 == m2);
    int l1 = m1->length();
    int l2 = m2->length();
    if (report_progress) {
        int nb_steps;
        if (easy_case) {
            nb_steps = (l1 * (l1 + 1)) / 2;
        } else {
            nb_steps = l1 * l2;
        }
        pb = new ProgressBar("Applying " + classname() + " to two matrices", nb_steps);
    }
    int count = 0;
    if(easy_case)
    {
        for(int i=0; i<m1->length(); i++)
        {
            m1->getSubRow(i,0,m1_i);
            for(int j=0; j<=i; j++)
            {
                m2->getSubRow(j,0,m2_j);
                real val = evaluate(m1_i,m2_j);
                result(i,j) = val;
                result(j,i) = val;
            }
            if (pb) {
                count += i + 1;
                pb->update(count);
            }
        }
    }
    else
    {
        for(int i=0; i<m1->length(); i++)
        {
            m1->getSubRow(i,0,m1_i);
            for(int j=0; j<m2->length(); j++)
            {
                m2->getSubRow(j,0,m2_j);
                result(i,j) = evaluate(m1_i,m2_j);
            }
            if (pb) {
                count += l2;
                pb->update(count);
            }
        }
    }
}


void Kernel::apply(VMat m, const Vec& x, Vec& result) const
{
    result.resize(m->length());
    int mw = m->inputsize();
    if (mw == -1) { // No inputsize specified: using width instead.
        mw = m->width();
    }
    Vec m_i(mw);
    for(int i=0; i<m->length(); i++)
    {
        m->getSubRow(i,0,m_i);
        result[i] = evaluate(m_i,x);
    }
}


void Kernel::apply(Vec x, VMat m, Vec& result) const
{
    result.resize(m->length());
    int mw = m->inputsize();
    if (mw == -1) { // No inputsize specified: using width instead.
        mw = m->width();
    }
    Vec m_i(mw);
    for(int i=0; i<m->length(); i++)
    {
        m->getSubRow(i,0,m_i);
        result[i] = evaluate(x,m_i);
    }
}


Mat Kernel::apply(VMat m1, VMat m2) const
{
    Mat result;
    apply(m1,m2,result);
    return result;
}

//////////
// test //
//////////
real Kernel::test(VMat d, real threshold, real sameness_below_threshold, real sameness_above_threshold) const
{
    int nerrors = 0;
    int inputsize = (d->width()-1)/2;
    for(int i=0; i<d->length(); i++)
    {
        Vec inputs = d(i);
        Vec input1 = inputs.subVec(0,inputsize);
        Vec input2 = inputs.subVec(inputsize,inputsize);
        real sameness = inputs[inputs.length()-1];
        real kernelvalue = evaluate(input1,input2);
        cerr << "[" << kernelvalue << " " << sameness << "]\n";
        if(kernelvalue<threshold)
        {
            if(fast_exact_is_equal(sameness, sameness_above_threshold))
                nerrors++;
        }
        else // kernelvalue>=threshold
        {
            if(fast_exact_is_equal(sameness, sameness_below_threshold))
                nerrors++;
        }
    }
    return real(nerrors)/d->length();
}


//////////////////////////////////////////////
// computeKNNeighbourMatrixFromDistanceMatrix //
//////////////////////////////////////////////
TMat<int> Kernel::computeKNNeighbourMatrixFromDistanceMatrix(const Mat& D, int knn, bool insure_self_first_neighbour, bool report_progress)
{
    int npoints = D.length();
    TMat<int> neighbours(npoints, knn);  
    Mat tmpsort(npoints,2);

    PP<ProgressBar> pb;
    if (report_progress) {
        pb = new ProgressBar("Computing neighbour matrix", npoints);
    }
  
    Mat indices;
    for(int i=0; i<npoints; i++)
    {
        for(int j=0; j<npoints; j++)
        {
            tmpsort(j,0) = D(i,j);
            tmpsort(j,1) = j;
        }
        if(insure_self_first_neighbour)
            tmpsort(i,0) = -FLT_MAX;

        partialSortRows(tmpsort, knn);
        indices = tmpsort.column(1).subMatRows(0,knn);
        for (int j = 0; j < knn; j++) {
            neighbours(i,j) = int(indices(j,0));
        }
        if (pb)
            pb->update(i);
    }
    return neighbours;
}

//////////////////////////////////////////////
// computeNeighbourMatrixFromDistanceMatrix //
//////////////////////////////////////////////
//  You should use computeKNNeighbourMatrixFromDistanceMatrix instead.
Mat Kernel::computeNeighbourMatrixFromDistanceMatrix(const Mat& D, bool insure_self_first_neighbour, bool report_progress)
{
    int npoints = D.length();
    Mat neighbours(npoints, npoints);  
    Mat tmpsort(npoints,2);

    PP<ProgressBar> pb;
    if (report_progress) {
        pb = new ProgressBar("Computing neighbour matrix", npoints);
    }
  
    //for(int i=0; i<2; i++)
    for(int i=0; i<npoints; i++)
    {
        for(int j=0; j<npoints; j++)
        {
            tmpsort(j,0) = D(i,j);
            tmpsort(j,1) = j;
        }
        if(insure_self_first_neighbour)
            tmpsort(i,0) = -FLT_MAX;

        sortRows(tmpsort);
        neighbours(i) << tmpsort.column(1);
        if (pb)
            pb->update(i);
    }
    return neighbours;
}

////////////////////////
// estimateHistograms //
////////////////////////
Mat Kernel::estimateHistograms(VMat d, real sameness_threshold, real minval, real maxval, int nbins) const
{
    real binwidth = (maxval-minval)/nbins;
    int inputsize = (d->width()-1)/2;
    Mat histo(2,nbins);
    Vec histo_below = histo(0);
    Vec histo_above = histo(1);
    int nbelow=0;
    int nabove=0;
    for(int i=0; i<d->length(); i++)
    {
        Vec inputs = d(i);
        Vec input1 = inputs.subVec(0,inputsize);
        Vec input2 = inputs.subVec(inputsize,inputsize);
        real sameness = inputs[inputs.length()-1];
        real kernelvalue = evaluate(input1,input2);
        if(kernelvalue>=minval && kernelvalue<maxval)
        {
            int binindex = int((kernelvalue-minval)/binwidth);
            if(sameness<sameness_threshold)
            {
                histo_below[binindex]++;
                nbelow++;
            }
            else
            {
                histo_above[binindex]++;
                nabove++;
            }
        }
    }
    histo_below /= real(nbelow);
    histo_above /= real(nabove);
    return histo;
}


Mat Kernel::estimateHistograms(Mat input_and_class, real minval, real maxval, int nbins) const
{
    real binwidth = (maxval-minval)/nbins;
    int inputsize = input_and_class.width()-1;
    Mat inputs = input_and_class.subMatColumns(0,inputsize);
    Mat classes = input_and_class.column(inputsize);
    Mat histo(4,nbins);
    Vec histo_mean_same = histo(0);
    Vec histo_mean_other = histo(1);
    Vec histo_min_same = histo(2);
    Vec histo_min_other = histo(3);

    for(int i=0; i<inputs.length(); i++)
    {
        Vec input = inputs(i);
        real input_class = classes(i,0);
        real sameclass_meandist = 0.0;
        real otherclass_meandist = 0.0;
        real sameclass_mindist = FLT_MAX;
        real otherclass_mindist = FLT_MAX;
        for(int j=0; j<inputs.length(); j++)
            if(j!=i)
            {
                real dist = evaluate(input, inputs(j));
                if(fast_exact_is_equal(classes(j,0), input_class))
                {
                    sameclass_meandist += dist;
                    if(dist<sameclass_mindist)
                        sameclass_mindist = dist;
                }
                else
                {
                    otherclass_meandist += dist;
                    if(dist<otherclass_mindist)
                        otherclass_mindist = dist;
                }
            }
        sameclass_meandist /= (inputs.length()-1);
        otherclass_meandist /= (inputs.length()-1);      
        if(sameclass_meandist>=minval && sameclass_meandist<maxval)
            histo_mean_same[int((sameclass_meandist-minval)/binwidth)]++;
        if(sameclass_mindist>=minval && sameclass_mindist<maxval)
            histo_min_same[int((sameclass_mindist-minval)/binwidth)]++;
        if(otherclass_meandist>=minval && otherclass_meandist<maxval)
            histo_mean_other[int((otherclass_meandist-minval)/binwidth)]++;
        if(otherclass_mindist>=minval && otherclass_mindist<maxval)
            histo_min_other[int((otherclass_mindist-minval)/binwidth)]++;
    }
    histo_mean_same /= sum(histo_mean_same, false);
    histo_min_same /= sum(histo_min_same, false);
    histo_mean_other /= sum(histo_mean_other, false);
    histo_min_other /= sum(histo_min_other, false);
    return histo;
}

/*
  void
  Kernel::oldwrite(ostream& out) const
  {
  writeHeader(out,"Kernel");
  writeField(out,"is_symmetric",is_symmetric);
  writeFooter(out,"Kernel");
  }


  void
  Kernel::oldread(istream& in)
  {
  readHeader(in,"Kernel");
  readField(in,"is_symmetric",is_symmetric);
  readFooter(in,"Kernel");
  }
*/

//////////////////////////////////////
// findClosestPairsOfDifferentClass //
//////////////////////////////////////
// last column of data is supposed to be a classnum
// returns a matrix of (index1, index2, distance)
Mat findClosestPairsOfDifferentClass(int k, VMat data, Ker dist)
{
    Mat result(k,3);
    real maxdistinlist = -FLT_MAX;
    int posofmaxdistinlist = -1;
    int kk=0; // number of pairs already in list
    Vec rowi(data.width());
    Vec inputi = rowi.subVec(0,rowi.length()-1);
    real& targeti = rowi[rowi.length()-1];
    Vec rowj(data.width());
    Vec inputj = rowj.subVec(0,rowj.length()-1);
    real& targetj = rowj[rowj.length()-1];
    for(int i=0; i<data.length(); i++)
    {
        data->getRow(i,rowi);
        for(int j=0; j<data.length(); j++)
        {
            data->getRow(j,rowj);
            if(!fast_exact_is_equal(targeti, targetj))
            {
                real d = dist(inputi,inputj);
                if(kk<k)
                {
                    result(kk,0) = i;
                    result(kk,1) = j;
                    result(kk,2) = d;
                    if(d>maxdistinlist)
                    {
                        maxdistinlist = d;
                        posofmaxdistinlist = kk;
                    }
                    kk++;
                }
                else if(d<maxdistinlist)
                {
                    result(posofmaxdistinlist,0) = i;
                    result(posofmaxdistinlist,1) = j;
                    result(posofmaxdistinlist,2) = d;
                    posofmaxdistinlist = argmax(result.column(2));
                    maxdistinlist = result(posofmaxdistinlist,2);
                }
            }
        }
    }
    sortRows(result, 2);//use partialSortRows instead
    return result;
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
