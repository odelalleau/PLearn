// -*- C++ -*-

// geometry.h
//
// Copyright (C) 2004 Pascal Lamblin 
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
   * $Id: geometry.h,v 1.9 2005/03/04 21:02:57 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file PLearn/plearn_learners_experimental/geometry.h */


#ifndef geometry_INC
#define geometry_INC

#include <plearn/math/TMat_maths.h>
#include <plearn/math/random.h>
#include "SurfaceMesh.h"
#include <plearn_learners/nearest_neighbors/BallTreeNearestNeighbors.h>
//#include "../balltrees/BTreeLearner.h"

namespace PLearn {
using namespace std;

typedef enum {FACE, VERTEX1, VERTEX2, VERTEX3, EDGE1, EDGE2, EDGE3} TriType;

Vec fixedAnglesFromRotation( const Mat& m );

Mat rotationFromFixedAngles( real rx, real ry, real rz );

Mat rotationFromAxisAngle( Vec& K, real th );

void transformPoints( const Mat& rot, const Vec& trans,
                      const Mat& points_in, Mat& points_out );

void transformMesh( const Mat& rot, const Vec& trans, SurfMesh& sm );

void weightedTransformationFromMatchedPoints( const Mat& mp, const Mat& sp,
                                              const Vec& weights, Mat& rot,
                                              Vec& trans, real& error );

Vec weightedCentroid( const Mat& pts, const Vec& weights );

Mat weightedRotationFromMatchedPoints( const Mat& mp, const Mat& sp,
                                       const Vec& weights, real& error );

int jacobi( Mat& a, Vec& d, Mat& v, int& nrot );

void rotate( Mat& a, int i, int j, int  k, int l,
                          const real& s, const real& tau );

void eigsrt( Vec& d, Mat& v, int n );

real maxPointMotion( const Mat& old_points, const Mat& new_points );

real calcNormal( graph& mesh, const vertex_descriptor& vtx, Vec& norm );

Vec calcNormal( const Vec& v1, const Vec& v2, const Vec& v3,
                const Vec& n1, const Vec& n2, const Vec& n3,
                const Vec& target);


void findSumsFromPts( const graph& mesh, const set<vertex_descriptor>& points,
                      Vec& sums );

void calcPlaneParams(const Vec& sums, Vec& norm, real& d, real& err);

int getNormFromEigVecs( const Vec& ev, const Mat& e, Vec& norm);

Vec cross( const Vec& v1, const Vec& v2 );

Mat randomRotation( real max_angle );

void randomTransformation( real max_angle, real max_dist,
                           Mat& rot, Vec& trans );

Mat boundingBoxToVertices( const Mat& bbox );

void getNearestVertex( const Vec& test_pt, const SurfMesh& mesh2,
                       const GenericNN& btl,
                       int& closest_vertex, Vec& closest_pt,
                       real& closest_dist );

bool isOverlapping( Vec& test_pt,
                    Vec& test_normal,
                    const SurfMesh& mesh2,
                    const TVec< set<int> >& face_cache,
                    GenericNN& btl,
                    const real init_dist_t,
                    const real normal_t, //rads
                    int& closest_vertex,
                    Vec& closest_pt,
                    real& closest_dist );

bool pointIsInterior( const TriType tri_type, const int m2face,
                      const SurfMesh& mesh2 );

bool closestFacePoint( const Vec& m1pt,
                       const set<int>& m2faces,
                       const SurfMesh& mesh2,
                       const real dist_t,
                       Vec& closest_pt,
                       real& closest_dist,
                       int& closest_face,
                       TriType& closest_tri_type );

bool closestPointOnTriangle( const Vec& p,
                             const Vec& v1, const Vec& v2, const Vec& v3,
                             const real dist_t, Vec& closest,
                             TriType& tri_type, real& dist );

inline int region1ClosestPoint( const Vec& planep,
                                const Vec& va, const  Vec& vb, const Vec& ea,
                                Vec& closest );

inline int region2ClosestPoint( const Vec& planep,
                                const Vec& va, const Vec& vb, const Vec& vc,
                                const Vec& ea, const Vec& eb, Vec closest );


} // end of namespace PLearn

#endif
