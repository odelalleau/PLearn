// -*- C++ -*-

// geometry.cc
//
// Copyright (C) 1996 Andrew E. Johnson (aej@ri.cmu.edu)
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

// These functions come from ....

/*! \file geometry.cc */


#include "geometry.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

// Put function implementations here.

Vec anglesFromRotationMatrix( const Mat& rot )
{
    Vec angle( 3 );
    angle[1] = atan2( -rot(2,0), sqrt( rot(0,0)*rot(0,0)+rot(1,0)*rot(1,0) ) );
    angle[2] = atan2( rot(1,0) / cos( angle[1] ), rot(0,0) / cos( angle[1] ) );
    angle[0] = atan2( rot(2,1) / cos( angle[1] ), rot(2,2) / cos( angle[1] ) );

    if( angle[1] * 180 / Pi > 89.9 )
    {
        angle[1] = Pi / 2;
        angle[2] = 0;
        angle[0] = atan2( rot(0,1), rot(1,1) );
    }
    else if( angle[1] * 180.0 / Pi < -89.9 )
    {
        angle[1] = -Pi / 2;
        angle[2] = 0;
        angle[0] = -atan2( rot(0,1), rot(1,1) );
    }

    return( angle * ( 180.0 / Pi ) );
}

Mat rotationMatrixFromAngles( real rx, real ry, real rz )
{
    Mat rot( 3, 3 );
    rx *= DEG2RAD;
    ry *= DEG2RAD;
    rz *= DEG2RAD;

    rot(0, 0) = cos( rz ) * cos( ry );
    rot(1, 0) = sin( rz ) * cos( ry );
    rot(2, 0) = -sin( ry );

    rot(0, 1) = cos( rz ) * sin( ry ) * sin( rx ) - sin( rz ) * cos( rx );
    rot(1, 1) = sin( rz ) * sin( ry ) * sin( rx ) + cos( rz ) * cos( rx );
    rot(2, 1) = cos( ry ) * sin( rx );

    rot(0, 2) = cos( rz ) * sin( ry ) * cos( rx ) + sin( rz ) * sin( rx );
    rot(1, 2) = sin( rz ) * sin( ry ) * cos( rx ) - cos( rz ) * sin( rx );
    rot(2, 2) = cos( ry ) * cos( rx );

    return rot;
}

Mat rotationMatrixFromAngles( const Vec& angles )
{
    if( angles.size() != 3 )
        PLERROR( "rotationMatrixFromAngles - angles size should be 3 (is %d)."
                 "\n", angles.size() );

    return rotationMatrixFromAngles( angles[0], angles[1], angles[0] );
}

Mat rotationFromAxisAngle( const Vec& K, real th )
{
    Mat R( 3, 3 );
    real c = cos( th );
    real s = sin( th );
    real v = 1 - c;

    R( 0, 0 ) = K[0]*K[0]*v + c;
    R( 0, 1 ) = K[0]*K[1]*v - K[2]*s;
    R( 0, 2 ) = K[0]*K[2]*v + K[1]*s;
    R( 1, 0 ) = K[0]*K[1]*v + K[2]*s;
    R( 1, 1 ) = K[1]*K[1]*v + c;
    R( 1, 2 ) = K[1]*K[2]*v - K[0]*s;
    R( 2, 0 ) = K[0]*K[2]*v - K[1]*s;
    R( 2, 1 ) = K[1]*K[2]*v + K[0]*s;
    R( 2, 2 ) = K[2]*K[2]*v + c;

    return R;
}


void applyGeomTransformation( const Mat& rot, const Vec& trans,
                              const Mat& points_in, Mat& points_out )
{
    points_out.resize( points_in.length(), 3 );
    productTranspose( points_out, points_in, rot );
    points_out += trans;
}

void transformationFromWeightedMatchedPoints( const Mat& template_points,
                                              const Mat& mol_points,
                                              const Vec& weights,
                                              const Mat& rot, const Vec& trans,
                                              real& error )
{
    Vec t_centroid = weightedCentroid( template_points, weights );
    Vec m_centroid = weightedCentroid( mol_points, weights );

    Mat origin_tp = template_points - t_centroid;
    Mat origin_mp = mol_points - m_centroid;

    rot << rotationFromWeightedMatchedPoints( origin_tp, origin_mp,
                                              weights, error );

    // trans = m_centroid - rot * t_centroid
    trans << m_centroid;
    productAcc( trans, rot, -t_centroid );
}

Mat rotationFromWeightedMatchedPoints( const Mat& template_points,
                                       const Mat& mol_points,
                                       const Vec& weights,
                                       real& error )
{
    Mat M( 4, 4 );
    Mat A( 4, 4 );
    Mat rot( 3, 3 );

    int n = template_points.length();
    Vec tp( 3 );
    Vec mp( 3 );

    for( int i=0 ; i<n ; i++ )
    {
        tp = template_points( i );
        mp = mol_points( i );

        real weight = weights[ i ];
        M(0,1) = tp[2]+mp[2];
        M(0,2) = -tp[1]-mp[1];
        M(0,3) = tp[0]-mp[0];

        M(1,0) = -tp[2]-mp[2];
        M(1,2) = tp[0]+mp[0];
        M(1,3) = tp[1]-mp[1];

        M(2,0) = tp[1]+mp[1];
        M(2,1) = -tp[0]-mp[0];
        M(2,3) = tp[2]-mp[2];

        M(3,0) = -tp[0]+mp[0];
        M(3,1) = -tp[1]+mp[1];
        M(3,2) = -tp[2]+mp[2];

        // A = A + transpose(M) * M * weight
        transposeProductAcc( A, M, M * weight );
    }

    Vec eigen_vals( 4 );
    Mat eigen_vecs( 4, 4 );

    eigenVecOfSymmMat( A, 4, eigen_vals, eigen_vecs, false );

    error = eigen_vals[ 3 ];
    real theta = 2.0 * acos( eigen_vecs( 3, 3 ) );

    if( theta !=0 )
    {
        Vec axis = eigen_vecs.subMat( 3, 0, 1, 3 ).toVecCopy();
        axis /= sin( theta/2.0 );
        rot << rotationFromAxisAngle( axis, theta );
    }
    else
        rot << diagonalmatrix( Vec( 3, 1 ) );

    return rot;
}

Vec weightedCentroid( const Mat& points, const Vec& weights )
{
    Vec centroid( 3 );
    int n = points.length();
    real w_tot = 0;

    for( int i=0 ; i<n ; i++ )
    {
        real w = weights[i];
        centroid += points(i) * w;
        w_tot += w;
    }

    if( w_tot == 0 )
        centroid.fill(0);
    else
        centroid /= w_tot;

    return centroid;
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
