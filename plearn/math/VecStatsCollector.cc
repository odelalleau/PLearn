// -*- C++ -*-
// VecStatsCollector.cc
// 
// Copyright (C) 2002 Pascal Vincent
// Copyright (C) 2005 University of Montreal
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

/*! \file VecStatsCollector.cc */
#include "VecStatsCollector.h"
#include "TMat_maths.h"
#include <assert.h>
#include <plearn/base/stringutils.h>    //!< For pl_isnumber.
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

VecStatsCollector::VecStatsCollector():
    maxnvalues(0), compute_covariance(false), epsilon(0.0),
    m_window(-1), no_removal_warnings(false), // Window mechanism
    sum_non_missing_weights(0),
    sum_non_missing_square_weights(0)
{}

PLEARN_IMPLEMENT_OBJECT(VecStatsCollector,
                        "Collects basic statistics on a vector",
                        "VecStatsCollector allows to collect statistics on a series of vectors.\n"
                        "Individual vectors x are presented by calling update(x), and this class will\n"
                        "collect both individual statistics for each element (as a Vec<StatsCollector>)\n"
                        "as well as (optionally) compute the covariance matrix."
    );

void VecStatsCollector::declareOptions(OptionList& ol)
{
    declareOption(ol, "maxnvalues", &VecStatsCollector::maxnvalues,
                                    OptionBase::buildoption,
        "Maximum number of different values to keep track of for each element.\n"
        "If -1, we will keep track of all different values.\n"
        "If 0, we will only keep track of global statistics.\n");

    declareOption(ol, "fieldnames", &VecStatsCollector::fieldnames, OptionBase::buildoption,
                  "Names of the fields of the vector");

    declareOption(ol, "compute_covariance", &VecStatsCollector::compute_covariance, OptionBase::buildoption,
                  "Should we compute and keep the covariance X'X ?");

    declareOption(ol, "epsilon", &VecStatsCollector::epsilon, OptionBase::buildoption,
                  "Optional regularization to *add* to the variance vector "
                  "(used *only* in getVariance, getCovariance and getStdDev).");

    declareOption(
        ol, "window", &VecStatsCollector::m_window,
        OptionBase::buildoption,
        "If positive, the window restricts the stats computed by this"
        "FinVecStatsCollector to the last 'window' observations. This uses the\n"
        "VecStatsCollector::remove_observation mechanism.\n"
        "\n"
        "Default: -1 (all observations are considered)." );

    declareOption(
        ol, "no_removal_warnings", &VecStatsCollector::no_removal_warnings,
        OptionBase::buildoption,
        "If the remove_observation mecanism is used and the removed\n"
        "value is equal to one of last_, min_ or max_, the default\n"
        "behavior is to warn the user.\n"
        "\n"
        "If one want to disable this feature, he may set\n"
        "no_removal_warnings to true.\n"
        "\n"
        "Default: false (0)." );
  
    declareOption(ol, "stats", &VecStatsCollector::stats, OptionBase::learntoption,
                  "the stats for each element");

    declareOption(ol, "cov", &VecStatsCollector::cov, OptionBase::learntoption,
                  "The uncentered and unnormalized covariance matrix (mean not subtracted): X'X");

    declareOption(ol, "sum_cross", &VecStatsCollector::sum_cross, OptionBase::learntoption,
                  "Element (i,j) is equal to the (weighted) sum of x_i when both x_i and x_j were observed");

    declareOption(ol, "sum_cross_weights", &VecStatsCollector::sum_cross_weights, OptionBase::learntoption,
                  "Element (i,j) is the sum of weights when both x_i and x_j were observed\n"
                  "(only used when 'compute_covariance' is set to 1)\n");

    declareOption(ol, "sum_cross_square_weights", &VecStatsCollector::sum_cross_square_weights, OptionBase::learntoption,
                  "Element (i,j) is the sum of square weights when both x_i and x_j were observed\n"
                  "(only used when 'compute_covariance' is set to 1)\n");

    declareOption(ol, "sum_non_missing_weights", &VecStatsCollector::sum_non_missing_weights, OptionBase::learntoption,
                  "Sum of weights for vectors with no missing value.");

    declareOption(ol, "sum_non_missing_square_weights", &VecStatsCollector::sum_non_missing_square_weights, OptionBase::learntoption,
                  "Sum of square weights for vectors with no missing value.");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

double VecStatsCollector::getStat(const string& statspec)
{
    PStream in = openString(statspec,PStream::plearn_ascii);
    string statname;
    in.smartReadUntilNext("[", statname);
    string fieldname;
    in.smartReadUntilNext("]", fieldname);
    int fieldnum = getFieldNum(fieldname);  
    if(fieldnum<0)
        PLERROR("In VecStatsCollector::getStat invalid fieldname: %s",fieldname.c_str());

    // It could be that nothing was accumulated into the stats collector,
    // which is different from accessing the "wrong" field.  In the first
    // case, return MISSING_VALUE
    if (stats.length() == 0)
        return MISSING_VALUE;
  
    return getStats(fieldnum).getStat(statname);
}

const Mat& VecStatsCollector::getObservations() const
{
    assert( m_window > 0 );
    return m_observation_window->m_observations;
}

const PP<ObservationWindow>
VecStatsCollector::getObservationWindow() const
{
    assert( m_window > 0 );
    return m_observation_window;
}

void VecStatsCollector::setFieldNames(TVec<string> the_fieldnames)
{
    fieldnames = the_fieldnames.copy();
    fieldnames_num.clear();
    for (int i=0, n=fieldnames.size() ; i<n ; ++i)
        fieldnames_num[fieldnames[i]] = i;
}

int VecStatsCollector::getFieldNum(const string& fieldname_or_num) const
{
    map<string,int>::const_iterator it = fieldnames_num.find(fieldname_or_num);
    if (it == fieldnames_num.end()) {          // not found
        if (pl_isnumber(fieldname_or_num))
            return toint(fieldname_or_num);
        else
            return -1;                             // unknown field
    }
    else
        return it->second;
}


////////////
// update //
////////////
void VecStatsCollector::update(const Vec& x, real weight)
{
    int n = x.size();
    if(stats.size()==0)
    {
        stats.resize(n);
        for(int k=0; k<n; k++)
        {
            // TODO It would be cool to have a simple (or automatic) mechanism
            // to be able to specify a different value of 'maxnvalues' for each
            // StatsCollector (e.g. when only one StatsCollector is meant to
            // compute a lift statistics).
            stats[k].maxnvalues          = maxnvalues;
            stats[k].no_removal_warnings = no_removal_warnings;
            stats[k].forget();
        }
        if(compute_covariance)
        {
            cov.resize(n,n);
            sum_cross.resize(n,n);
            sum_cross_weights.resize(n,n);
            sum_cross_square_weights.resize(n,n);
            cov.fill(0);
            sum_cross.fill(0);
            sum_cross_weights.fill(0);
            sum_cross_square_weights.fill(0);
        }      
    }

    if(stats.size()!=n)
        PLERROR("In VecStatsCollector::update -  Called update with vector of length "
                "%d, while size of stats (and most likely previously seen vector) is "
                "%d", n, stats.size());

    for(int k=0; k<n; k++)
        stats[k].update(x[k], weight);

    if(compute_covariance) {
        if (x.hasMissing()) {
            // Slower version to handle missing values.
            // TODO Could certainly be optimized.
            real val_i, val_j;
            for (int i = 0; i < n; i++) {
                val_i = x[i];
                if (!is_missing(val_i)) {
                    for (int j = 0; j < n; j++) {
                        val_j = x[j];
                        if (!is_missing(val_j)) {
                            cov(i,j)                      += weight * val_i * val_j;
                            sum_cross(i,j)                += weight * val_i;
                            sum_cross_weights(i,j)        += weight;
                            sum_cross_square_weights(i,j) += weight * weight;
                        }
                    }
                }
            }
        } else {
            externalProductScaleAcc(cov, x, x, weight);
            sum_non_missing_weights        += weight;
            sum_non_missing_square_weights += weight * weight;
            // TODO The two lines below could be optimized with an additional Vec
            // storing the sum of weighted x_i for non missing data.
            for (int i = 0; i < n; i++)
                sum_cross(i)                 += weight * x[i];
        }
    }
    
    // Window mechanism
    if (m_window > 0)
    {
        tuple<Vec, real> outdated = m_observation_window->update(x, weight);
        Vec& obs = get<0>(outdated);
        real weight = get<1>(outdated);
        if ( obs.isNotEmpty() )
            remove_observation(obs, weight);
    }
}

////////////////////////
// remove_observation //
////////////////////////
void VecStatsCollector::remove_observation(const Vec& x, real weight)
{
    assert( stats.size() > 0 );

    int n = x.size();

    if(stats.size()!=n)
        PLERROR( "In VecStatsCollector: problem, called remove_observation with vector of length %d, "
                 "while size of stats (and most likeley previously seen vector) is %d", 
                 n, stats.size() );

    for(int k=0; k<n; k++)
        stats[k].remove_observation(x[k], weight);
  
    // This removes the observation x contribution to the covariance matrix.
    if( compute_covariance ) {
        if (fast_exact_is_equal(stats[0].nnonmissing(), 0)) {
            // We removed the last observation. It may be safer to reset everything
            // so that numerical approximations do not lead to negative values for
            // statistics that should always be positive.
            forget();
        } else {
            if (x.hasMissing()) {
                // Slower version to handle missing values.
                // TODO Could certainly be optimized.
                real val_i, val_j;
                for (int i = 0; i < n; i++) {
                    val_i = x[i];
                    if (!is_missing(val_i)) {
                        for (int j = 0; j < n; j++) {
                            val_j = x[j];
                            if (!is_missing(val_j)) {
                                cov(i,j)                      -= weight * val_i * val_j;
                                sum_cross(i,j)                -= weight * val_i;
                                sum_cross_weights(i,j)        -= weight;
                                sum_cross_square_weights(i,j) -= weight * weight;
                            }
                        }
                    }
                }
            } else {
                externalProductScaleAcc(cov, x, x, -weight);
                sum_non_missing_weights        -= weight;
                sum_non_missing_square_weights -= weight * weight;
                // TODO The two lines below could be optimized with an additional Vec
                // storing the sum of weighted x_i for non missing data.
                for (int i = 0; i < n; i++)
                    sum_cross(i)               -= weight * x[i];
            }
        }
    }
}


//! calls update on all rows of m
void VecStatsCollector::update(const Mat& m)
{
    int l = m.length();
    for(int i=0; i<l; i++)
        update(m(i));
}

//! calls update on all rows of m, with given weight vector
void VecStatsCollector::update(const Mat& m, const Vec& weights)
{
    if (m.length() != weights.size())
        PLERROR("VecStatsCollector::update: matrix height (%d) "
                "is incompatible with weights length (%d)", m.length(),
                weights.size());
    int l = m.length();
    for(int i=0; i<l; i++)
        update(m(i), weights[i]);
}

void VecStatsCollector::build_()
{
    if (m_window > 0)
        m_observation_window = new ObservationWindow(m_window);
}

void VecStatsCollector::build()
{
    inherited::build();
    build_();
}

void VecStatsCollector::forget()
{
    stats.resize(0);
    cov.resize(0,0);
    sum_cross.resize(0,0);
    sum_cross_weights.resize(0,0);
    sum_cross_square_weights.resize(0,0);
    sum_non_missing_weights = 0;
    sum_non_missing_square_weights = 0;

    // Window mechanism
    if ( m_window > 0 )
        m_observation_window->forget();
}

void VecStatsCollector::finalize()
{
    int n = stats.size();
    for(int i=0; i<n; i++)
        stats[i].finalize();
}

/////////////
// getMean //
/////////////
void VecStatsCollector::getMean(Vec& res) const
{
    int n = stats.size();
    res.resize(n);
    for(int k=0; k<n; k++)
        res[k] = stats[k].mean();
}

/////////////////
// getVariance //
/////////////////
Vec VecStatsCollector::getVariance() const
{
    int n = stats.size();
    Vec res(n);
    for(int k=0; k<n; k++)
        res[k] = stats[k].variance() + epsilon;
    return res;
}

///////////////
// getStdDev //
///////////////
Vec VecStatsCollector::getStdDev() const
{
    int n = stats.size();
    Vec res(n);
    for(int k=0; k<n; k++)
        res[k] = sqrt(stats[k].variance() + epsilon);
    return res;
}

/////////////////
// getStdError //
/////////////////
Vec VecStatsCollector::getStdError() const
{
    int n = stats.size();
    Vec res(n);
    for(int k=0; k<n; k++)
        res[k] = stats[k].stderror();
    return res;
}

///////////////////
// getCovariance //
///////////////////
void VecStatsCollector::getCovariance(Mat& covar) const {
    // Formula used to compute an unbiased estimate of the covariance (note
    // that it may not yield a positive semi-definite matrix).
    // Notations:
    // x(k)_i                       = i-th coordinate of k-th sample 
    // sum^i_k   f(i,k)             = sum over k of f(i,k)   for k such that
    //                                x(k)_i is not missing
    // sum^i,j_k f(i,j,k)           = sum over k of f(i,j,k) for k such that
    //                                neither x(k)_i nor x(k)_j is missing
    // w(k)                         = weight of k-th sample
    // cov_i_j                      = sum^i,j_k w(k) x(k)_i * x(k)_j
    // mean_i                       = (sum^i_k w(k) x(k)_i) / sum^i_k w(k)
    // The estimator for element (i,j) of the covariance matrix is then:
    // covariance(i,j) = [ cov_i_j + mean_i * mean_j * sum^i,j_k w(k)
    //                             - mean_j * sum^i,j_k w(k) x(k)_i
    //                             - mean_i * sum^i,j_k w(k) x(k)_j
    //                   ] /
    //                   [    sum^i,j_k w(k)
    //                     + (sum^i,j_k w(k) - sum^i_k w(k) - sum^j_k w(k) )
    //                       * sum^i,j_k w(k)^2 / (sum^i_k w(k) * sum^j_k w(k))
    //                   ]
    //
    // If features i and j have never been observed simultaneously, or one of
    // the two features has been observed only once, then covariance(i,j) is
    // set to MISSING_VALUE.
    // The first  case occurs when sum^i,j_k w(k) == 0.
    // The second case occurs when sum^i,j_k w(k) == sqrt(sum^i,j_k w(k)^2)
    //                                            == sum^{i or j}_k w(k)
    static Vec meanvec;
    assert( compute_covariance && cov.length() == cov.width() );
    int d = cov.length();
    getMean(meanvec);
    covar.resize(d,d);
    for(int i=0; i<d; i++) {
        real sum_weights_i = stats[i].nnonmissing();
        for(int j=i; j<d; j++) {
            real sum_weights_j = stats[j].nnonmissing();
            real sum_cross_weights_i_j = sum_cross_weights(i,j) + sum_non_missing_weights;
            real sum_cross_square_weights_i_j = sum_cross_square_weights(i,j)
                + sum_non_missing_square_weights;
            real mean_i = meanvec[i];
            real mean_j = meanvec[j];
            if (fast_exact_is_equal(sum_cross_weights_i_j, 0) ||
                (fast_is_equal(sum_cross_weights_i_j,
                               sqrt(sum_cross_square_weights_i_j)) &&
                 (fast_is_equal(sum_cross_weights_i_j, sum_weights_i) ||
                  fast_is_equal(sum_cross_weights_i_j, sum_weights_j))))
                // One of the two cases described above.
                covar(i,j) = MISSING_VALUE;
            else
                covar(i,j) =
                    (cov(i,j) + mean_i * mean_j * sum_cross_weights_i_j
                              - mean_j * sum_cross(i,j)
                              - mean_i * sum_cross(j,i)
                    ) /
                    (  sum_cross_weights_i_j
                     + (sum_cross_weights_i_j - sum_weights_i - sum_weights_j)
                     * sum_cross_square_weights_i_j
                     / (sum_weights_i * sum_weights_j)
                    );
            if (j == i)
                covar(i,j) += epsilon;
            else
                covar(j,i) = covar(i,j);
        }
    }
}

Mat VecStatsCollector::getCovariance() const
{
    Mat covariance;
    getCovariance(covariance);
    return covariance;
}

////////////////////
// getCorrelation //
////////////////////
Mat VecStatsCollector::getCorrelation() const
{  
    Mat norm(cov.width(),cov.width());
    externalProduct(norm,getStdDev(),getStdDev());
    return getCovariance()/norm;
}

Vec VecStatsCollector::getAllStats(const string& statname) const
{
    const int n = stats.size();
    Vec r(n);
    getAllStats(statname, r);
    return r;
}

void VecStatsCollector::getAllStats(const string& statname,
                                    Vec& result) const
{
    const int n = stats.size();
    result.resize(n);
    for (int i=0; i<n; ++i)
        result[i] = getStats(i).getStat(statname);
}

void VecStatsCollector::append(const VecStatsCollector& vsc,
                               const string fieldname_prefix,
                               const TVec<string>& new_fieldnames)
{
    // To avoid problems with fieldnames, ensure we don't start out with too
    // many fieldnames, and pad nonexistent fieldnames in *this with ""
    fieldnames.resize(stats.size());
    for (int i=fieldnames.size(), n = stats.size() ; i<n ; ++i)
        fieldnames.append("");
  
    stats.append(vsc.stats);

    // Take care of field names
    if (new_fieldnames.size() > 0) {
        assert( new_fieldnames.size() == vsc.stats.size() );
        fieldnames.append(new_fieldnames);
    }
    else {
        const int n = vsc.stats.size();
        assert (vsc.fieldnames.size() == n );
        fieldnames.resize(fieldnames.size(), n);
        for (int i=0 ; i<n ; ++i)
            fieldnames.append(fieldname_prefix + vsc.fieldnames[i]);
    }
    setFieldNames(fieldnames);                 // update map

    // Take care of covariance matrix
    if (compute_covariance) {
        const int oldsize = cov.width();
        const int vscsize = vsc.cov.width();
        assert( oldsize == cov.length() && vscsize == vsc.cov.length() );
        int new_n = stats.size();
        Mat newcov(new_n, new_n, 0.0);
        Mat new_sum_cross(new_n, new_n, 0.0);
        Mat new_sum_cross_weights(new_n, new_n, 0.0);
        Mat new_sum_cross_square_weights(new_n, new_n, 0.0);
        newcov.subMat(0,0,oldsize,oldsize) << cov;
        Mat sub = new_sum_cross.subMat(0, 0, oldsize, oldsize);
        sub << sum_cross;
        sub = new_sum_cross_weights.subMat(0, 0, oldsize, oldsize);
        sub << sum_cross_weights;
        sub += sum_non_missing_weights;
        sum_non_missing_weights = 0;
        sub = new_sum_cross_square_weights.subMat(0, 0, oldsize, oldsize);
        sub << sum_cross_square_weights;
        sub += sum_non_missing_square_weights;
        sum_non_missing_square_weights = 0;
        if (vsc.compute_covariance) {
            newcov.subMat(oldsize,oldsize,vscsize,vscsize) << vsc.cov;
            sub = new_sum_cross.subMat(oldsize,oldsize,vscsize,vscsize);
            sub << vsc.sum_cross;
            sub = new_sum_cross_weights.subMat(oldsize,oldsize,vscsize,vscsize);
            sub << vsc.sum_cross_weights;
            sub += vsc.sum_non_missing_weights;
            sub = new_sum_cross_square_weights.subMat(oldsize,oldsize,vscsize,vscsize);
            sub << vsc.sum_cross_square_weights;
            sub += vsc.sum_non_missing_square_weights;
        }
        else {
            newcov.subMat(oldsize,oldsize,vscsize,vscsize).fill(MISSING_VALUE);
            new_sum_cross.subMat(oldsize, oldsize, vscsize, vscsize).fill(MISSING_VALUE);
            new_sum_cross_weights.subMat(oldsize,oldsize,vscsize,vscsize).fill(MISSING_VALUE);
            new_sum_cross_square_weights.subMat(oldsize,oldsize,vscsize,vscsize).fill(MISSING_VALUE);
        }
        cov                      = newcov;
        sum_cross                = new_sum_cross;
        sum_cross_weights        = new_sum_cross_weights;
        sum_cross_square_weights = new_sum_cross_square_weights;
    }
}

void VecStatsCollector::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Object::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(fieldnames,               copies);
    deepCopyField(stats,                    copies);
    deepCopyField(cov,                      copies);
    deepCopyField(sum_cross,                copies);
    deepCopyField(sum_cross_weights,        copies);
    deepCopyField(sum_cross_square_weights, copies);
    deepCopyField(m_observation_window,     copies);
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
