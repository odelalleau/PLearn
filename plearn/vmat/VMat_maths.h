// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: VMat_maths.h,v 1.13 2004/02/20 21:14:44 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef VMat_maths_INC
#define VMat_maths_INC

#include <map>
#include "TMat.h"
#include "TMat_maths.h"
#include "StatsCollector.h"
#include "ConditionalStatsCollector.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class VecStatsCollector;

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
Mat computeBasicStats(VMat m);

//! Retirns the unconditional statistics of each field
TVec<StatsCollector> computeStats(VMat m, int maxnvalues);

void computeStats(VMat m, VecStatsCollector& st);


//! returns the cooccurence statistics conditioned on the given field
PP<ConditionalStatsCollector> computeConditionalStats(VMat m, int condfield, TVec<RealMapping> ranges);

void computeRowMean(VMat d, Vec& meanvec);
void computeMean(VMat d, Vec& meanvec);
void computeWeightedMean(Vec weights, VMat d, Vec& meanvec);
void computeMeanAndVariance(VMat d, Vec& meanvec, Vec& variancevec);
void computeMeanAndStddev(VMat d, Vec& meanvec, Vec& stddevvec);
void computeMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat, ostream& logstream=cerr);
void computeWeightedMeanAndCovar(Vec weights, VMat d, Vec& meanvec, Mat& covarmat);

void autocorrelation_function(const VMat& data, Mat& acf);

//! Computes the (possibly weighted) mean and covariance of the input part of the dataset.
//! This will only call d->getExamplev
void computeInputMean(VMat d, Vec& meanvec);
void computeInputMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat);
void computeInputMeanAndVariance(VMat d, Vec& meanvec, Vec& var);


void computeRange(VMat d, Vec& minvec, Vec& maxvec);

//! Last column of d is supposed to contain the weight for each sample
//! Samples with a weight less or equal to threshold will be ignored
//! (returns the sum of all the weights actually used)
real computeWeightedMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat, real threshold=0);

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
Array<Mat> computeConditionalMeans(VMat trainset, int targetsize, Mat& basic_stats);

/*!   subtracts mean and divide by stddev
  meanvec and stddevvec can be shorter than d.width() 
  (as we probably only want to 'normalize' the 'input' part of the sample, 
  and not the 'output' that is typically present in the last columns)
*/
VMat normalize(VMat d, Vec meanvec, Vec stddevvec);
VMat normalize(VMat d, int inputsize, int ntrain); //!<  Here, mean and stddev are estimated on d.subMat(0,0,ntrain,inputsize)
inline VMat normalize(VMat d, int inputsize) { return normalize(d,inputsize,d.length()); }

/*!   If exclude==false (the default) 
    returns a VMat containing only the rows
    whose column col has a value that belongs 
    to the given set of authorized values
  If exclude==true
    returns a VMat with all the other rows 
    (corresponds to grep -v)
  [MISSING_VALUE is a possible value and is handled correctly]
*/
VMat grep(VMat d, int col, Vec values, bool exclude=false);

//! returns a map mapping all different values appearing in column col to their number of occurences
map< real, int> countOccurencesInColumn(VMat m, int col);

//! returns a map mapping all different values appearing in column col to a vector of the corresponding row indices in the VMat
//! (this proceeds in 2 passes, first calling countOccurencesInColumn to allocate the exact memory)
map< real, TVec<int> > indicesOfOccurencesInColumn(VMat m, int col);

/*!   Same as above, except that the indexes of the rows are stored on disk
  rather than in memory
  (a SelectRowsFileIndexVMatrix is returned rather than a SelectRowsVMatrix)
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat grep(VMat d, int col, Vec values, const string& indexfile, bool exclude=false);

/*!   returns a VMat that contains only the lines that do not have any MISSING_VALUE
  The indexes of the rows of the original matrix are recorded in the indexfile
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat filter(VMat d, const string& indexfile);

//!  returns a SelectRowsVMatrix that has d's rows shuffled
VMat shuffle(VMat d);

//! returns a SelectRowsVMatrix that has d's rows bootstrapped (sample with replacement
//! and optionally re-ordered). Optionally the repeated rows are eliminated (this is actually
//! done by shuffling and taking the first 2/3 of the rows, so the length() will be always the
//! same).  Note that the default values are fine for "on-line"
//! learning algorithms but does not correspond to the usual "bootstrap".
//! 
VMat bootstrap(VMat d, bool reorder=true, bool norepeat=true);

//!  computes M1'.M2
Mat transposeProduct(VMat m1, VMat m2);

//!  computes M'.M
Mat transposeProduct(VMat m);

//!  computes M1'.V2
Vec transposeProduct(VMat m1, Vec v2);

//!  computes M1.M2'
Mat productTranspose(VMat m1, VMat m2);

//!  computes M1.M2
Mat product(Mat m1, VMat m2);

//!  returns M1'
VMat transpose(VMat m1);

/*!   computes the result of the linear regression into theta_t
  Parameters must have the following sizes:
  inputs(l,n)
  outputs(l,m)
  theta_t(n+1,m)
  XtX(n+1,n+1)
  XtY(n+1,m)
  The n+1 is due to the inclusion of the bias terms in the matrices (first row of theta_t)
  If use_precomputed_XtX_XtY is false, then they are computed. Otherwise
  they are used as they are (typically passed precomputed from a previous
  call made with a possibly different weight_decay).
  Returns average of squared loss.
*/
real linearRegression(VMat inputs, VMat outputs, real weight_decay, Mat theta_t, 
                      bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, real& sum_squared_Y,
                      bool return_squared_loss=false, int verbose_computation_every=0);
                      

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
Mat linearRegression(VMat inputs, VMat outputs, real weight_decay);

//! Linear regression where each input point is given a different importance weight (the gammas);
//! returns weighted average of squared loss
real weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas,
                              real weight_decay, Mat theta_t, bool use_precomputed_XtX_XtY, Mat XtX,
                              Mat XtY, real& sum_squared_Y, real& sum_gammas, bool return_squared_loss=false, 
                              int verbose_computation_every=0);

/*!   Rebalance the input VMatrix so that each class has a probability 1/nclasses.
  Also, the return VMat class alternates between all classes cyclicly.
  The resulting VMat is a SelectRowsFileIndexVMatrix which has its IntVecFile
  load if filename already exist, or computed if not. 
*/
VMat rebalanceNClasses(VMat inputs, int nclasses, const string& filename);

//!  Rebalance a 2-class VMat such as to keep all the examples of the
//!  dominant class.
void fullyRebalance2Classes(VMat inputs, const string& filename, bool save_indices=true);

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
Mat weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay);

/*!   This VMat is a SelectRowsVMatrix which, given a threshold date,
  keep only the rows earlier (or later) than this date.  The thresdold date
  is given as a YYYYMMDD date, and the date on the original VMatrix are kept
  on 1 column (YYYYMMDD) or 3 (YYYY, MM and DD).
*/
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
      int yyyymmdd_col);
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
      int yyyy_col, int mm_col, int dd_col);

//! Compute the correlations between each of the columns of x and each of the 
//! columns of y. The results are in the x.width() by y.width() matrix r.
void correlations(const VMat& x, const VMat& y, Mat& r, Mat& pvalues);

} // end of namespace PLearn

#endif
