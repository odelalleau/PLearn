// -*- C++ -*-

// geometry.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file PLearn/plearn_learners_experimental/geometry.h */


#ifndef geometry_INC
#define geometry_INC

// Put includes here
#include <plearn/math/TVec_decl.h>
#include <plearn/math/TMat_decl.h>

namespace PLearn {

// Put global function declarations here
Vec anglesFromRotationMatrix( const Mat& rot );
Mat rotationMatrixFromAngles( real rx, real ry, real rz );
Mat rotationMatrixFromAngles( const Vec& angles );
Mat rotationMatrixFromAxisAngle( const Vec& K, real th );

// points_in should be different from points_out
void applyGeomTransformation( const Mat& rot, const Vec& trans,
                              const Mat& points_in, Mat& points_out );

void transformationFromWeightedMatchedPoints( const Mat& template_points,
                                              const Mat& mol_points,
                                              const Vec& weights,
                                              const Mat& rot, const Vec& trans,
                                              real& error );

Vec weightedCentroid( const Mat& points, const Vec& weights );

Mat rotationFromWeightedMatchedPoints( const Mat& template_points,
                                       const Mat& mol_points,
                                       const Vec& weights,
                                       real& error );



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
