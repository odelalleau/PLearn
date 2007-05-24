// -*- C++ -*-

// VMat_basic_stats.h
//
// Copyright (C) 2004 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file VMat_basic_stats.h */


#ifndef VMat_basic_stats_INC
#define VMat_basic_stats_INC

// Put includes here
#include <plearn/math/TVec.h>
#include <plearn/math/TMat.h>
#include <plearn/base/Array.h>

namespace PLearn {
using namespace std;

class VMat;

#define MEAN_ROW 0
#define STDDEV_ROW 1
#define MIN_ROW 2
#define MAX_ROW 3
#define NMISSING_ROW 4
#define NZERO_ROW 5
#define NPOSITIVE_ROW 6
#define NNEGATIVE_ROW 7
#define MEANPOS_ROW 8
#define STDDEVPOS_ROW 9

/*!   The returned Mat is structured as follows:
  row 0: mean
  row 1: stddev
  row 2: min
  row 3: max
  row 4: nmissing
  row 5: nzero     (==0)
  row 6: npositive (>0)
  row 7: nnegative (<0)
  row 8: mean of positive
  row 9: stddev of positive
*/
Mat computeBasicStats(const VMat& m);

//! Compute mean of each row (the returned vector has length d->length()).
void computeRowMean             (const VMat& d, Vec& meanvec);

//! Compute basic statistics over all samples.
//! The first methods will compute statistics over *all* columns of a VMat,
//! including target and weight columns (an additional weight vector must be
//! supplied if one wants to compute weighted statistics).
//! The last methods (computeInput...) will compute statistics only on the
//! *input* part of a VMat, and weighted statistics can be obtained directly
//! using the weights in the VMat.
void computeMean                (const VMat& d, Vec& meanvec);
void computeMeanAndVariance     (const VMat& d, Vec& meanvec, Vec& variancevec, double epsilon=0.0);
void computeMeanAndStddev       (const VMat& d, Vec& meanvec, Vec& stddevvec,   double epsilon=0.0);
void computeMeanAndCovar        (const VMat& d, Vec& meanvec, Mat& covarmat,    double epsilon=0.0);
//! Computes covariance matrix given mean mu.
void computeCovar               (const VMat& d, const Vec& mu, Mat& covarmat,    double epsilon=0.0);
void computeWeightedMean        (const Vec& weights, const VMat& d, Vec& meanvec);
void computeWeightedMeanAndCovar(const Vec& weights, const VMat& d,
                                 Vec& meanvec, Mat& covarmat, double epsilon=0.0);
void computeInputMean           (const VMat& d, Vec& meanvec);
//! Computes covariance matrix given mean mu.
void computeInputCovar(const VMat& d, const Vec& mu, Mat& covarmat, double epsilon=0.0);
void computeInputMeanAndCovar   (const VMat& d, Vec& meanvec, Mat& covarmat, double epsilon=0.0);
void computeInputMeanAndVariance(const VMat& d, Vec& meanvec, Vec& var,      double epsilon=0.0);
void computeInputMeanAndStddev  (const VMat& d, Vec& meanvec, Vec& stddev,   double epsilon=0.0);

void autocorrelation_function(const VMat& data, Mat& acf);

void computeRange(const VMat& d, Vec& minvec, Vec& maxvec);

/*!   Computes conditional mean and variance of each target, conditoned on the
  values of categorical integer input feature.  The basic_stats matrix may
  be passed if previously computed (see computeBasicStats) or an empty
  matrix may be passed otherwise, that will compute the basic statistics.

  An input feature #i is considered a categorical integer input if its min
  and max (as found in basic_stats) are integers and are not too far
  apart.  For these, the correponding returned array[i] matrix will
  contain max-min+1 rows (one for each integer value between min and max
  inclusive), each row containing the corresponding input value, the
  number of times it occured, and mean and variance for each target.  The
  returned matrix array[i] for input features that are not considered
  categorical integers are empty.
*/
TVec<Mat> computeConditionalMeans(VMat trainset, int targetsize, Mat& basic_stats);

/*!   Subtracts mean and divide by stddev.
  meanvec and stddevvec can be shorter than d.width()
  (as we probably only want to 'normalize' the 'input' part of the sample,
  and not the 'target' that is typically present in the last columns).
*/
VMat normalize(const VMat& d, const Vec& meanvec, const Vec& stddevvec);

//! Here, mean and stddev are estimated on d.subMat(0,0,ntrain,inputsize).
VMat normalize(const VMat& d, int inputsize, int ntrain);
//! Here, mean and stddev are estimated on the whole dataset d.
VMat normalize(const VMat& d, int inputsize);

//! Compute the correlations between each of the columns of x and each of the
//! columns of y. The results are in the x.width() by y.width() matrix r.
//! The p-values of the corresponding test (no correlation) are stored
//! in the same-sized matrix pvalues.
//! If 'ignore_missing' is set to true, for each pair of columns, only rows
//! which have non-missing values will be taken into account.
//! This method should not be called with 'ignore_missing' set to false if there
//! are missing values in the VMats.
void correlations(const VMat& x, const VMat& y, Mat& r, Mat& pvalues, bool ignore_missing = false);

} // end of namespace PLearn
#endif


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
