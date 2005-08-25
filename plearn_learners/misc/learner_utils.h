
// -*- C++ -*-

// learner_utils.h
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file learner_utils.h */
#ifndef learner_utils_INC
#define learner_utils_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

Mat compute_learner_outputs(PP<PLearner> learner, VMat dataset);

// Finds appropriate x0, y0, deltax, deltay from the dataset range,
// extraspace of .10 means we'll look 10% beyond the data range on every side
void determine_grid_for_dataset(VMat dataset, int nx, int ny, 
                                real& x0, real& y0, real& deltax, real& deltay, 
                                real extraspace=.10);

//! Returns a nx*ny x learner->outputsize() matrix of outputs corresponding to the nx*ny grid points 
Mat compute_learner_outputs_on_grid(PP<PLearner> learner, int nx, int ny, 
                                    real x0, real y0, real deltax, real deltay);


double determine_density_integral_from_log_densities_on_grid(Vec log_densities, real deltax, real deltay);

//! considers data to have 2d input (first 2 columns of data)
void DX_write_2D_data(ostream& out, const string& basename, Mat data);

//! data must have nx*ny rows and must corresponds to values associated with the 2D
//! positions of the grid (typically learner outputs on that grid)
void DX_write_2D_data_for_grid(ostream& out, const string& basename, 
                               int nx, int ny, real x0, real y0, real deltax, real deltay,
                               Mat data);

//! considers data to have 2d input (first 2 columns of data)
void DX_save_2D_data(const string& filename, const string& basename, Mat data);

//! data must have nx*ny rows and must corresponds to values associated with the 2D
//! positions of the grid (typically learner outputs on that grid)
void DX_save_2D_data_for_grid(const string& filename, const string& basename, 
                              int nx, int ny, real x0, real y0, real deltax, real deltay,
                              Mat data);


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
