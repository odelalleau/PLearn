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
   * $Id: DisplayUtils.h,v 1.1 2002/07/30 09:01:27 plearner Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnUtil/DisplayUtils.h */

#ifndef DISPLAYUTILS_INC
#define DISPLAYUTILS_INC

#include "PLearn.h"
#include "GhostScript.h"
#include "Mat.h"
#include "Gnuplot.h"
#include "Learner.h"
#include "Func.h"

namespace PLearn <%
using namespace std;


/*!   Display a histogram of the density of the data column.
  By default set n_bins according to the number of data
  points: n_bins = min(5+dataColumn.length()/10,1000), but user can override this.
  By default (n_bins=0) the bins are equally spaced such that each
  bin receives approximately the same number of points. The
  user can override that by providing a vector of bin boundaries
  (min to bins[0], bins[0] to bins[1], ... bins[n_bins-2] to max)
  then n_bins will be bins.length()+1, or the user can specify
  regularly spaced bins with the bool argument. If normalized
  the the relative frequencies rather than actual frequencies are plotted.
*/
void displayHistogram(Gnuplot& gs, Mat dataColumn,
		      int n_bins=0, Vec* bins=0, 
		      bool regular_bins=false,
		      bool normalized=false, string extra_args="");


/*! * VarGraph * */

  void displayVarGraph(const VarArray& outputs, bool display_values=false, real boxwidth=100, const char* the_filename=0, bool must_wait=true, VarArray display_only_these=VarArray());
  void displayFunction(Func f, bool display_values=false, bool display_differentiation=false, real boxwidth=100, const char* the_filename=0, bool must_wait=true);



/*!     This will return a length*width matrix containing the computed outputs
    for a learner which has 2 dimensional input, where the inputs are
    taken on a regular grid ranging [min_x,max_x]x[min_y,max_y].  The
    mapping to the matrix m is m(i,j) =
    f(min_x+i*(max_x-min_x)/(length-1), min_y+j*(max_y-min_y)/(width-1))
    If the output is of length 1: (class depends on which side of the threshold we are)
      the result put in m is output[0] - singleoutput_threshold
    If the output is of length 2: (score for each class)
      the result put in m is output[0]-output[1]
*/
Mat compute2dGridOutputs(Learner& learner, real min_x=-1, real max_x=+1, real min_y=-1, real max_y=+1, 
                         int length=200, int width=200, real singleoutput_threshold=0.);

  //!  this draws x and + with the given radius for all the points in data (supposed to have width 3: [x, y, classnum]
void displayPoints(GhostScript& gs, Mat data, real radius, bool color=false);

/*!     This will display a rectangle (0,0,nx,ny)
    containing a 2D image of the decision surface for the given learner
    with the points of each class displayed with + and x, and optionally,
    the points in svindexes circled in black and the points in outlierindexes circled in gray.
*/
  void displayDecisionSurface(GhostScript& gs, real destx, real desty, real destwidth, real destheight, 
                              Learner& learner, Mat trainset,
                              Vec svindexes=Vec(), Vec outlierindexes=Vec(), int nextsvindex=-1,
                              real min_x=-1, real max_x=+1, real min_y=-1, real max_y=+1,
                              real radius=0.05, 
                              int nx=200, int ny=200);

%> // end of namespace PLearn

#endif

