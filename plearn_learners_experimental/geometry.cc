// -*- C++ -*-

// geometry.cc
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
   * $Id: geometry.cc,v 1.15 2005/12/29 11:52:28 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file PLearn/plearn_learners_experimental/geometry.cc */


#include "geometry.h"

namespace PLearn {
using namespace std;

Vec fixedAnglesFromRotation( const Mat& m )
{
  Vec angle( 3 );

  angle[1] = atan2( -m(2,0), sqrt( m(0,0)*m(0,0) + m(1,0)*m(1,0) ) );
  angle[2] = atan2( m(1,0) / cos( angle[1] ), m(0,0) / cos( angle[1] ) );
  angle[0] = atan2( m(2,1) / cos( angle[1] ), m(2,2) / cos( angle[1] ) );

  if( angle[ 1 ] * 180 / Pi > 89.9 )
  {
    angle[ 1 ] = Pi / 2;
    angle[ 2 ] = 0;
    angle[ 0 ] = atan2( m( 0, 1 ), m( 1, 1 ) );
  }
  else if( angle[ 1 ] * 180.0 / Pi < -89.9 )
  {
    angle[ 1 ] = -Pi / 2;
    angle[ 2 ] = 0;
    angle[ 0 ] = -atan2( m( 0, 1 ), m( 1, 1 ) );
  }

  return( angle * ( real(180.0 / Pi) ) );
}

Mat rotationFromFixedAngles( real rx, real ry, real rz )
{
  Mat rot( 3, 3 );

  rx *= DEG2RAD;
  ry *= DEG2RAD;
  rz *= DEG2RAD;

  rot( 0, 0 ) = cos( rz ) * cos( ry );
  rot( 1, 0 ) = sin( rz ) * cos( ry );
  rot( 2, 0 ) = -sin( ry );

  rot( 0, 1 ) = cos( rz ) * sin( ry ) * sin( rx ) - sin( rz ) * cos( rx );
  rot( 1, 1 ) = sin( rz ) * sin( ry ) * sin( rx ) + cos( rz ) * cos( rx );
  rot( 2, 1 ) = cos( ry ) * sin( rx );

  rot( 0, 2 ) = cos( rz ) * sin( ry ) * cos( rx ) + sin( rz ) * sin( rx );
  rot( 1, 2 ) = sin( rz ) * sin( ry ) * cos( rx ) - cos( rz ) * sin( rx );
  rot( 2, 2 ) = cos( ry ) * cos( rx );

  return rot;
}


Mat rotationFromAxisAngle( Vec& K, real th )
{
  // j'ai pas cherché à comprendre
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


Mat boundingBoxToVertices( const Mat& bbox )
{
  /*
  Mat vertices( 8, 3 );

  ostringstream buf;
  buf << bbox(0,0) << " " << bbox(0,1) << " " << bbox(0,2) << " "
      << bbox(0,0) << " " << bbox(0,1) << " " << bbox(1,2) << " "
      << bbox(0,0) << " " << bbox(1,1) << " " << bbox(0,2) << " "
      << bbox(0,0) << " " << bbox(1,1) << " " << bbox(1,2) << " "
      << bbox(1,0) << " " << bbox(0,1) << " " << bbox(0,2) << " "
      << bbox(1,0) << " " << bbox(0,1) << " " << bbox(1,2) << " "
      << bbox(1,0) << " " << bbox(1,1) << " " << bbox(0,2) << " "
      << bbox(1,0) << " " << bbox(1,1) << " " << bbox(1,2) << " ";

  vertices << buf.str();
   */

  real buf_[24] = {
    bbox(0,0), bbox(0,1), bbox(0,2),
    bbox(0,0), bbox(0,1), bbox(1,2),
    bbox(0,0), bbox(1,1), bbox(0,2),
    bbox(0,0), bbox(1,1), bbox(1,2),
    bbox(1,0), bbox(0,1), bbox(0,2),
    bbox(1,0), bbox(0,1), bbox(1,2),
    bbox(1,0), bbox(1,1), bbox(0,2),
    bbox(1,0), bbox(1,1), bbox(1,2)
  };

  Mat vertices( 8, 3, buf_ );

  return vertices;
}



void transformPoints( const Mat& rot, const Vec& trans,
                      const Mat& points_in, Mat& points_out )
{
  int n = points_in.length();
  points_out.resize( n, 3 );
  Mat tmp( n, 3 );

  productTranspose( tmp, points_in, rot );
  tmp += trans;
  points_out << tmp;
}

void transformMesh( const Mat& rot, const Vec& trans, SurfMesh& sm )
{
  Mat input = sm->getVertexCoords();
  transformPoints( rot, trans, input, input );
  sm->setVertexCoords( input );

  input = sm->getVertexNorms();
  transformPoints( rot, Vec(3), input, input );
  sm->setVertexNorms( input );
}



void weightedTransformationFromMatchedPoints( const Mat& mp, const Mat& sp,
                                              const Vec& weights, Mat& rot,
                                              Vec& trans, real& error )
{
  Vec cs( 3 );
  cs << weightedCentroid( sp, weights );

  Vec cm( 3 );
  cm << weightedCentroid( mp, weights );

  int n = mp.length();
  Mat origin_mp( n, 3 );
  Mat origin_sp( n, 3 );

  origin_mp = mp - cm;
  origin_sp = sp - cs;

  rot << weightedRotationFromMatchedPoints( origin_mp, origin_sp, 
                                            weights, error );

  Vec res( 3 );
  product( res, rot, cm );

  trans << ( cs - res );
}

Vec weightedCentroid( const Mat& pts, const Vec& weights )
{
  Vec centroid( 3 );
  int n = pts.length();
  real w_tot = 0;

  for( int i=0 ; i<n ; i++ )
  {
    real w = weights[ i ];
    centroid += pts( i ) * w;
    w_tot += w;
  }

  if( w_tot == 0 )
  {
    centroid = Vec( 3 );
  }
  else
  {
    centroid /= w_tot;
  }

  return centroid;
}


Mat weightedRotationFromMatchedPoints( const Mat& mp, const Mat& sp, 
                                       const Vec& weights, real& error )
{
  // Ouais, c'est absolument pas optimisé, je sais

  Mat M( 4, 4 );
  Mat A( 4, 4 );
  Mat rot( 3, 3 );

  int n = mp.length();
  Vec vm( 3 );
  Vec vs( 3 );

  for( int i=0 ; i<n ; i++ )
  {
    vm << mp( i );
    vs << sp( i );

    real weight = weights[ i ];
    M(0,1) = vm[2]+vs[2];
    M(0,2) = -vm[1]-vs[1];
    M(0,3) = vm[0]-vs[0];

    M(1,0) = -vm[2]-vs[2];
    M(1,2) = vm[0]+vs[0];
    M(1,3) = vm[1]-vs[1];

    M(2,0) = vm[1]+vs[1];
    M(2,1) = -vm[0]-vs[0];
    M(2,3) = vm[2]-vs[2];

    M(3,0) = -vm[0]+vs[0];
    M(3,1) = -vm[1]+vs[1];
    M(3,2) = -vm[2]+vs[2];

    // A = A + transpose(M) * M * weight
    transposeProductAcc( A, M, M * weight );
  }

  Vec ev( 4 );
  Mat e( 4, 4 );
  int n_rot;

  if( jacobi( A, ev, e, n_rot ) )
  {
    eigsrt( ev, e, 4 );

    error = ev[ 3 ];
    real theta = 2.0 * acos( e( 3, 3 ) );

    if( theta !=0 )
    {
      Vec v( 3 );
      v << e.subMat( 0, 3, 3, 1 );
      v /= real(sin( theta/2.0 ));
      rot << rotationFromAxisAngle( v, theta );
      return rot;
    }
  }

  // rot = Id3
  rot << diagonalmatrix( Vec( 3, 1 ) );

  return rot;

}

/*
 ******************************************************************************
 + FUNCTION: jacobi
 + AUTHOR:   Numerical Recipes in C
 + MODIFIED: Pascal Lamblin
 + DATE:     16-Aug-04
 + PURPOSE:  Finds eigenvectors and eigenvalues of a matrix.
 ******************************************************************************
 */
int jacobi( Mat& a, Vec& d, Mat& v, int& nrot )
{
  int n = a.length();
  if( n != a.width() || n != v.length() || n != v.width() || n != d.length() )
  {
    PLERROR( "in jacobi( a, d, v, nrot ):\n a.length(), a.width(), v.length(), v.width() and d.length() must be equal" );
  }

  // v = identity
  v = diagonalmatrix( Vec( n, 1 ) );
  nrot = 0;

  Vec b( n );
  Vec z( n );

  for( int ip=0 ; ip<n ; ip++ )
  {
    real val = a( ip,  ip );
    b[ ip ] = val;
    d[ ip ] = val;
  }

  for( int i=0 ; i<50 ; i++ )
  {
    real sm = 0;
    real tresh;
    for( int ip=0 ; ip<n-1 ; ip++ )
    {
      for( int iq=ip+1 ; iq<n ; iq++ )
      { sm += fabs( a( ip, iq ) ); }
    }

    if( sm == 0 )
    { return 1; }

    if( i < 3 )
    { tresh = 0.2*sm/(n*n); }
    else
    { tresh = 0; }

    for( int ip=0 ; ip<n-1 ; ip++ )
    {
      for( int iq=ip+1 ; iq<n ; iq++ )
      {
        real g=100.0 * fabs( a( ip, iq ) );
        if( i>3 && ( fabs( d[ip] ) + g == fabs( d[ip] ) )
            && ( fabs( d[iq] ) + g == fabs( d[iq] ) ) )
        { a( ip, iq ) = 0; }
        else if( fabs( a( ip, iq ) ) > tresh )
        {
          real h = d[iq] - d[ip];
          real t;
          if( fabs( h ) + g == fabs( h ) )
          { t = a( ip, iq ) / h; }
          else
          {
            real theta = 0.5 * h/( a( ip, iq ) );
            t = 1.0 / ( fabs( theta ) + sqrt( 1.0 + theta*theta ) );
            if( theta < 0.0 )
            { t = -t; }
          }
          real c = 1.0 / sqrt( 1 + t*t );
          real s = t*c;
          real tau = s / ( 1.0 + c );
          h = t*a( ip, iq );
          z[ip] -= h;
          z[iq] += h;
          d[ip] -= h;
          d[iq] += h;
          a( ip, iq ) = 0.0;

          for( int j=0 ; j<=ip-1 ; j++ )
          {
            rotate( a, j, ip, j, iq, s, tau );
          }
          for( int j=ip+1 ; j<=iq-1 ; j++ )
          {
            rotate( a, ip, j, j, iq , s, tau );
          }
          for( int j=iq+1 ; j<n ; j++ )
          {
            rotate( a, ip, j, iq, j, s, tau );
          }
          for( int j=0 ; j<n ; j++ )
          {
            rotate( v, j, ip, j, iq, s, tau );
          }
          ++nrot;
        }
      }
    }
    b += z;
    d << b;
    z = 0;
  }
  PLERROR( "jacobi: too many iterations" );
  return 0;
}


// auxiliary function for jacobi
void rotate( Mat& a, int i, int j, int  k, int l,
             const real& s, const real& tau )
{
  real g = a( i, j );
  real h = a( k, l );
  a( i, j ) = g - s*( h + g*tau );
  a( k, l ) = h + s*( g - h*tau );
}


/*
 ******************************************************************************
 + FUNCTION: eigsrt
 + AUTHOR:   Numerical Recipes in C
 + MODIFIED: Andrew E. Johnson (aej@ri.cmu.edu) 
 + DATE:     3-Nov-94
 + PURPOSE:  Sorts eigenvectors from jacobi based on eigenvalues.
 ******************************************************************************
 */

void eigsrt( Vec& d, Mat& v, int n )
{
  for( int i=0 ; i<n-1 ; i++ )
  {
    real p = d[ i ];
    int k = i;

    for( int j=i+1 ; j<n ; j++ )
    {
      if( fabs( d[j] ) >= fabs( p ) )
      {
        p = d[ j ];
        k = j;
      }
    }

    if( k!=i )
    {
      d[ k ] = d[ i ];
      d[ i ] = p;
      for( int j=0 ; j<n ; j++ )
      {
        p = v( j, i );
        v( j, i ) = v( j, k );
        v( j, k ) = p;
      }
    }
  }
}


real maxPointMotion( const Mat& old_points, const Mat& new_points )
{
  real max_motion2 = 0;
  int n = old_points.length();

  if( new_points.length() != n )
  {
    PLERROR( "maxPointMotion: old_points and new_points Mat must have same length" );
  }

  for( int i=0 ; i<n ; i++)
  {
    real motion2 = powdistance( new_points( i ), old_points( i ), 2 );
    max_motion2 = max( max_motion2, motion2 );
  }

  return sqrt( max_motion2 );
}




/*
 ******************************************************************************
 + FUNCTION: CalcNormal
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     29-Jun-94
 + PURPOSE:  Given a node in the mesh it calculates the surface normal at that
 +           node using the nodes nearest neighbors.
 ******************************************************************************
 */

real calcNormal( graph& mesh, const vertex_descriptor& vtx, Vec& norm )
{
  set<vertex_descriptor> points;

  adjacency_iterator ai, ai_end;
  for( tie(ai,ai_end)=adjacent_vertices(vtx,mesh) ; ai!=ai_end ; ai++ )
  {
    adjacency_iterator bi, bi_end;
    for( tie(bi,bi_end)=adjacent_vertices(*ai,mesh) ; bi!=bi_end ; bi++ )
    {
      points.insert( *bi );
    }
  }

  /* determine the sums used to calculate the surface normal by fitting 
     a plane to the neighborhood of points */

  Vec sums( 10 );
  real d;
  real fit_error;
  findSumsFromPts( mesh, points, sums );
  calcPlaneParams( sums, norm, d, fit_error ); // fit the plane to get normal

  real n = sums[0];

  real error_sum = 0;
  for( tie(ai,ai_end)=adjacent_vertices(vtx,mesh) ; ai!=ai_end ; ai++ )
  {
    real dist = dot( norm, get(vertex_ppt,mesh,*ai)->coord ) + d;
    error_sum += dist*dist;
  }

  fit_error = sqrt( error_sum )/n;

  if( n <= 3 )
  { fit_error = INFINITY; }

  /* return the fit error on the plane */
  return fit_error;

}

Vec calcNormal( const Vec& v1, const Vec& v2, const Vec& v3,
                const Vec& n1, const Vec& n2, const Vec& n3,  
                const Vec& target)
{
  // just use avg of the 3 normals for now (later use barycentric coords)
  Vec normal = n1 + n2 + n3;
  normalize( normal, 2 );

  return normal;
}



/*
 ******************************************************************************
 + FUNCTION: FindSumsFromPts
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     14-Jul-94
 + PURPOSE:  Calculates the sums needed to calculate the planar and quadric
 +           parameters of a region. It takes a set of integers that
 +           correspond to the array location of all of the meshpoints in the
 +           region and calulates the sums.
 ******************************************************************************
 */

void findSumsFromPts( const graph& mesh, const set<vertex_descriptor>& points,
                      Vec& sums )
{
  sums.resize( 10 );

  set<vertex_descriptor>::const_iterator it;
  for( it=points.begin() ; it!=points.end() ; it++ )
  {
    Vec p = get( vertex_ppt, mesh, *it )->coord;
    real x = p[ 0 ];
    real y = p[ 1 ];
    real z = p[ 2 ];

    sums[0]++;
    sums[1]+=x;
    sums[2]+=y;
    sums[3]+=z;
    sums[4]+=x*x;
    sums[5]+=y*y;
    sums[6]+=z*z;
    sums[7]+=x*y;
    sums[8]+=x*z;
    sums[9]+=y*z;

  }
}

/*
 ******************************************************************************
 + FUNCTION: CalcPlaneParams
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     6-Jul-94
 + PURPOSE:  Calculates the new plane parameters for a plane and a new point.
 ******************************************************************************
 */

void calcPlaneParams(const Vec& sums, Vec& norm, real& d, real& err)
{
  if( sums[0] >= 3 )
  {
    real one_over_n = 1./sums[0];

    Mat inertia( 3, 3 );

    inertia(0,0) = ( sums[4] - sums[1]*sums[1]*one_over_n );
    inertia(1,1) = ( sums[5] - sums[2]*sums[2]*one_over_n );
    inertia(2,2) = ( sums[6] - sums[3]*sums[3]*one_over_n );
    inertia(1,0) = inertia(0,1) = (sums[7]-sums[1]*sums[2]*one_over_n );
    inertia(2,0) = inertia(0,2) = (sums[8]-sums[1]*sums[3]*one_over_n );
    inertia(2,1) = inertia(1,2) = (sums[9]-sums[2]*sums[3]*one_over_n );

    Mat e( 3, 3 );
    Vec ev( 3 );
    int nrot;

    if( jacobi( inertia, ev, e, nrot ) )
    {
      // The eigen vector corresponding to the smallest eigen value of
      // the inertia matrix is the normal of the plane.

      int sm_ev = getNormFromEigVecs( ev, e, norm );
      err = fabs( ev[sm_ev] );
      d = -( sums[1]*norm[0] + sums[2]*norm[1]+sums[3]*norm[2])/sums[0];
      return;
    }
  }

  norm.resize( 3 );
  norm << "0 0 1";
  d = 0;
  err = INFINITY;
  return;
}

/*
 ******************************************************************************
 + FUNCTION: GetNormFromEigVecs
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     29-Jun-94
 + PURPOSE:  Finds the minimum eigenvalue in a vector of eigenvalues and sets
 +           the normal equal to this eigenvector. Returns the index of the
 +           smallest eigen value
 ******************************************************************************
 */


int getNormFromEigVecs( const Vec& ev, const Mat& e, Vec& norm)
{
  int sm_ev;
  if( fabs(ev[0]) <= fabs(ev[1]) )
  {
    if( fabs(ev[0]) <= fabs(ev[2]) )
    { sm_ev = 0; }
    else
    { sm_ev = 2; }
  }
  else
  {
    if( fabs(ev[1]) <= fabs(ev[2]) )
    { sm_ev = 1; }
    else
    { sm_ev = 2; }
  }

  /* set normal */
  if( e(2, sm_ev) >= 0 )
  {
    norm[0] = e(0, sm_ev);
    norm[1] = e(1, sm_ev);
    norm[2] = e(2, sm_ev);
  }
  else
  {
    norm[0] = -e(0, sm_ev);
    norm[1] = -e(1, sm_ev);
    norm[2] = -e(2, sm_ev);
  }

  return sm_ev;
}

/*
 ******************************************************************************
 + FUNCTION: Cross
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     14-oct-94
 + PURPOSE:  returns the cross product of two 3 vectors
 ******************************************************************************
 */

Vec cross( const Vec& v1, const Vec& v2 )
{
  if( v1.size()!=3 || v2.size()!=3 )
  {
    PLERROR("cross-product of 2 Vec is only defined for Vec of size 3");
  }

  Vec res( 3 );

  res[0] = v1[1]*v2[2] - v2[1]*v1[2];
  res[1] = -( v1[0]*v2[2] - v2[0]*v1[2] );
  res[2] = v1[0]*v2[1] - v2[0]*v1[1];

  return res;
}

/*
 ******************************************************************************
 + FUNCTION: RandomTransformation
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     26-Jun-96
 + PURPOSE:  Creates a random rotation matrix with a maximun rotation angle of
 +           max_angle degrees.
 ******************************************************************************
 */

void randomTransformation( real max_angle, real max_dist,
                           Mat& rot, Vec& trans )
{
  rot = randomRotation( max_angle );

  trans[0] = bounded_uniform( -max_dist, max_dist );
  trans[1] = bounded_uniform( -max_dist, max_dist );
  trans[2] = bounded_uniform( -max_dist, max_dist );
}


/*
 ******************************************************************************
 + FUNCTION: RandomRotation
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     26-Jun-96
 + PURPOSE:  Creates a random rotation matrix with a maximun rotation angle of
 +           max_angle degrees.-
 ******************************************************************************
 */

Mat randomRotation( real max_angle )
{
  Mat rot( 3, 3 );

  real x1 = uniform_sample();
  real x2 = uniform_sample();
  real x3 = uniform_sample();

  /* scale x3 by max_angle */
  x3 *= (max_angle/180.0);

  real z = x1;
  real t = 2*Pi*x2;
  real r = sqrt( 1 - z*z );
  real w = Pi*x3;

  /* create quaternion */
  real a = cos(w);
  real b = sin(w) * cos(t) * r;
  real c = sin(w) * sin(t) * r;
  real d = sin(w) * z;

  /* create rotation matrix */
  rot(0,0) = 1-2*(c*c+d*d);
  rot(0,1) = 2*(b*c+a*d);
  rot(0,2) = 2*(b*d-a*c);
  rot(1,0) = 2*(b*c-a*d);
  rot(1,1) = 1-2*(b*b+d*d);
  rot(1,2) = 2*(c*d+a*b);
  rot(2,0) = 2*(b*d+a*c);
  rot(2,1) = 2*(c*d-a*b);
  rot(2,2) = 1-2*(b*b+c*c);

  return rot;
}

void getNearestVertex( const Vec& test_pt, const SurfMesh& mesh2,
                       const GenericNN& btl,
                       int& closest_vertex, Vec& closest_pt,
                       real& closest_dist )
{
  // find closest vertex on mesh2
  Vec dists;
  Vec outputs;
  btl-> computeOutputAndCosts( test_pt, Vec(), outputs, dists );

  int dimension = outputs.size()-1;
  closest_pt << outputs.subVec( 0, dimension );
  closest_vertex = (int) outputs[dimension];
  closest_dist = dists[0];
}



// lookup tables for classifying triangles
static TriType r1_table[3][3] = {
  {VERTEX1, VERTEX2, EDGE1},
  {VERTEX2, VERTEX3, EDGE2},
  {VERTEX3, VERTEX1, EDGE3} };

static TriType r2_table[3][5] = {
  {VERTEX3, VERTEX1, VERTEX2, EDGE3, EDGE1},
  {VERTEX1, VERTEX2, VERTEX3, EDGE1, EDGE2},
  {VERTEX2, VERTEX3, VERTEX1, EDGE2, EDGE3} };

// need to be sure face_cache is filled in first
/******************************************************************************
Description:
  Returns true if the test point overlaps mesh2.  If the function returns true,
  it also returns the closest point on the surface of mesh2.

Arguments:
  test_pt, test_normal - coords and normal of test point
  mesh2
  face_cache - foreach vertex on mesh 2, lists adjacent faces
  kdt - for finding closest vertices
  normalT - threshold on angle between normals of test_pt and closest_pt
  closest_pt - return - closest point on the surface of mesh2

Return:
  Returns true if the test point overlaps mesh2.  It also returns the closest
  point on the surface of mesh2.

******************************************************************************/

bool isOverlapping( Vec& test_pt,
                    Vec& test_normal,
                    const SurfMesh& mesh2,
                    const TVec< set<int> >& face_cache,
                    GenericNN& btl,
                    const real init_dist_t,
                    const real normal_t, //rads
                    int& closest_vertex,
                    Vec& closest_pt,
                    real& closest_dist )
{
  real dist_t = init_dist_t;

  getNearestVertex( test_pt, mesh2, btl,
                    closest_vertex, closest_pt, closest_dist );
/*
  // find closest vertex on mesh2
  Vec dists;
  Vec outputs;
//  Vec targets;
  btl->computeOutputAndCosts( test_pt, Vec(), outputs, dists );

  closest_vertex = (int) outputs[0];
  closest_dist = dists[0];
*/
  int closest_face;
  TriType closest_tri_type;

  // find closest face point on mesh2
  if( !closestFacePoint( test_pt, face_cache[closest_vertex], mesh2, dist_t,
                         closest_pt, closest_dist, closest_face,
                         closest_tri_type ) )
  {
    // should not happen
    closest_dist = dist_t;
    closest_pt = Vec( 3, MISSING_VALUE );
    PLWARNING( "no closest face point found for %i.\n", closest_vertex );
    return false;
  }

  if( !pointIsInterior( closest_tri_type, closest_face, mesh2 ) )
  {
    return false;
  }

  // check if normals agree within threshold
  // 1 compute normal for point on mesh2
  // 2 compare normals with dot product
  MFace mf = mesh2->getFace( closest_face );
  MVertex p1 = mesh2->getVertex( mf->pts[0] );
  MVertex p2 = mesh2->getVertex( mf->pts[1] );
  MVertex p3 = mesh2->getVertex( mf->pts[2] );

  Vec m2_normal = calcNormal( p1->coord, p2->coord, p3->coord,
                              p1->norm, p2->norm, p3->norm,
                              closest_pt );

  if( dot( test_normal, m2_normal ) < cos( normal_t ) )
  {
    return false;
  }

  return true;
}

// returns true if point of triangle type tri_type on face m2face of
// mesh mesh2 is is interior to the boundary
bool pointIsInterior( const TriType tri_type, const int m2face,
                      const SurfMesh& mesh2 )
{
  // depending on tri_type, check whether the mesh points are boundary points
  MFace mf = mesh2->getFace( m2face );
  int bf1 = mesh2->getVertex( mf->pts[0] )->bf;
  int bf2 = mesh2->getVertex( mf->pts[1] )->bf;
  int bf3 = mesh2->getVertex( mf->pts[2] )->bf;

  switch( tri_type )
  {
    case VERTEX1:
      if( bf1 ) return false;
      break;
    case VERTEX2:
      if( bf2 ) return false;
      break;
    case VERTEX3:
      if( bf3 ) return false;
      break;
    case EDGE1:
      if( bf1 && bf2 ) return false;
      break;
    case EDGE2:
      if( bf2 && bf3 ) return false;
      break;
    case EDGE3:
      if( bf3 && bf1 ) return false;
      break;
    default:
      break;
  }
  return true;
}



/******************************************************************************
Description:
  Find the closest face point with distance less than distT.

Arguments:
  m1pt - reference point
  m2faces - set of faces (face_ids) that could contain closest point
  mesh2 - for looking up tha actual m2faces values
  distT - distance threshold-
  closest_pt - return - closest point on m2faces
  closest_dist - return - dist from m1pt to closest_pt
  closest_face - return - index of closest face
  closest_tri_type - return - reln of point to closest face

Return:
  computes closest_pt and closest_dist
  returns true if a point was found with distance <= distT, false otherwise
  if function returns false, closest_pt is not set!
******************************************************************************/

bool closestFacePoint( const Vec& m1pt,
                       const set<int>& m2faces,
                       const SurfMesh& mesh2,
                       const real dist_t,
                       Vec& closest_pt,
                       real& closest_dist,
                       int& closest_face,
                       TriType& closest_tri_type )
{
  bool found_closer( false );
  closest_dist = dist_t;

  set<int>::const_iterator loop_iter;
  for( loop_iter = m2faces.begin() ; loop_iter != m2faces.end() ; loop_iter++ )
  {
    int i = *loop_iter;
    MFace mf = mesh2->getFace( i );
    Vec m2coord1 = mesh2->getVertex( mf->pts[0] )->coord;
    Vec m2coord2 = mesh2->getVertex( mf->pts[1] )->coord;
    Vec m2coord3 = mesh2->getVertex( mf->pts[2] )->coord;

    Vec face_pt(3);
    TriType tri_type;
    real dist;
    if( closestPointOnTriangle( m1pt, m2coord1, m2coord2, m2coord3,
                                closest_dist, face_pt, tri_type, dist ) )
    {
      if( dist < closest_dist + REAL_EPSILON )
      {
        found_closer = true;
        closest_pt << face_pt;
        closest_dist = dist;
        closest_face = i;
        closest_tri_type = tri_type;
      }
    }
  }
  return found_closer;
}

/******************************************************************************
Description:
  Find the closest point on a given triangle to the point p if it is less than-
  distT distance away from the triangle.

  Basic algorithm is to:
  1. project p onto the plane containing the triangle.
  2. determine which region the point projects into (see below)
  3. depending on region, perform test to see which vertex or edge is closest.

Arguments:
  p - target point
  v1, v2, v3 - vertices of the triangle
  distT - threshold distance-
  closest - return value of closest point
  tri_type - classification of closest point (interior, one of the edges, or
  one of the vertices.
  dist - distance to closest point on triangle

Return:
  closest, tri_type, and dist
  function returns true if a point was found <= distT away, false otherwise.
  If false, the values of closest, tri_type, and dist are not determined.
******************************************************************************/
/*
     .                       .
        .  Region 2       .
           .           .
              .     .
                 .
              .     .   Region 1
           .           .
        .                 .
     .       Region 3        .
  ............................................

note: there are three areas that are region 1 and three that are region 2.

*/

bool closestPointOnTriangle( const Vec& p,
                             const Vec& v1,
                             const Vec& v2,
                             const Vec& v3,
                             const real dist_t,
                             Vec& closest,
                             TriType& tri_type,
                             real& dist )
{
  bool stop = false;

  if( powdistance( p, v1, 2 ) < REAL_EPSILON )
  {
    tri_type = VERTEX1;
    stop = true;
  }
  else if( powdistance( p, v2, 2 ) < REAL_EPSILON )
  {
    tri_type = VERTEX2;
    stop = true;
  }
  else if( powdistance( p, v3, 2 ) < REAL_EPSILON )
  {
    tri_type = VERTEX3;
    stop = true;
  }

  if( stop )
  {
    closest << p;
    dist = 0;
  }

  // determine triangle plane equation nx+d=0 and make sure triangle is
  // well defined
  Vec normal = cross( v2-v1, v3-v2 );
  real norm_length = norm( normal );

  if( norm_length < REAL_EPSILON ) // 2 edges of triangle pll (singularity)
  {
    return false;
  }

  // normalize the normal
  normalize( normal, 2 );

  // determine distance to plane
  dist = dot( normal, p ) - dot( normal, v1 );

  // quick test -- no point can be less than dist_t if the distance to
  // the plane containing the triangle is greater than dist_t
  if( fabs(dist) > dist_t + REAL_EPSILON )
  {
    return false;
  }

  // determine point on plane (planep = p - dist*normal)
  Vec planep = p - (dist*normal);

  // determine the position of planep with respect to the 3 lines making
  // up the triangle
  // si > 0 => point is to the left of the edge
  // si = (ei cross planep - vi) dot n

  Vec e1 = v2-v1;
  Vec e2 = v3-v2;
  Vec e3 = v1-v3;

  real s1 = dot( cross( e1, planep ), normal );
  real s2 = dot( cross( e2, planep ), normal );
  real s3 = dot( cross( e3, planep ), normal );

  // region 3 - point projects inside triangle, so return planep
  if( (s1 >= 0) && (s2 >= 0) && (s3 >= 0 ) )
  {
    closest << planep;
    dist = fabs( dist );
    tri_type = FACE;

    if( dist > dist_t + REAL_EPSILON )
      return false;
    else
      return true;

  }

  // region 1 tests - point is inside the u-shaped region formed by one
  // edge and the extension of the adjacent edges
  if( (s1<0) && (s2 >= 0) && (s3 >= 0) )
  {
    int edge_type = region1ClosestPoint( planep, v1, v2, e1, closest );
    tri_type = r1_table[0][edge_type];
    stop = true;
  }
  else if( (s1 >= 0) && (s2 < 0) && (s3 >= 0) )
  {
    int edge_type = region1ClosestPoint( planep, v2, v3, e2, closest );
    tri_type = r1_table[1][edge_type];
    stop = true;
  }
  else if( (s1 >= 0) && (s2 >= 0) && (s3 < 0) )
  {
    int edge_type = region1ClosestPoint( planep, v3, v1, e3, closest );
    tri_type = r1_table[2][edge_type];
    stop = true;
  }

  if( stop )
  {
    dist = norm( p-closest );

    if( dist > dist_t + REAL_EPSILON )
      return false;
    else
      return true;

  }

  // region 2 tests - point is inside the v-shaped region formed by the
  // extension of two edges
  if( (s1 < 0) && (s3 < 0) )
  {
    int edge_type = region2ClosestPoint( planep, v3, v1, v2, e3, e1, closest );
    tri_type = r2_table[0][edge_type];
    stop = true;
  }
  else if( (s2 < 0) && (s1 < 0) )
  {
    int edge_type = region2ClosestPoint( planep, v1, v2, v3, e1, e2, closest );
    tri_type = r2_table[1][edge_type];
    stop = true;
  }
  else if( (s3 < 0) && (s2 < 0) )
  {
    int edge_type = region2ClosestPoint( planep, v2, v3, v1, e2, e3, closest );
    tri_type = r2_table[2][edge_type];
    stop = true;
  }

  if( stop )
  {
    dist = norm( p-closest );

    if( dist > dist_t + REAL_EPSILON )
      return false;
    else
      return true;
  }
  else
  {
    // should not occur, since all solutions should be covered by
    // regions 1, 2 and 3
    cout << "closestPointOnTriangle failed" << endl;
    return false;
  }
}



/******************************************************************************
Description:
  Compute the closest point for a target point in region 1.  Alg is:
  1. compute the scaled distances (ta) of the closest point on the edge.
  2. depending on the value, the closest point will be one of the adjacent
  vertices or somewhere on the edge.

Arguments:
   planep - target point in plane
   va, vb - vertices of the triangle (see diagram below.
   ea - edge (see below)
   closest - return value of closest point

Return:
   closest
   function returns edge_type (used to determine triangle class
   edge_type =
     0 -> point is closest to va
     1 -> closest to vb
     2 -> closest to ea

******************************************************************************/
/*
      .                       .
         .                 .
            .           .
               . va  .
                  .     Region 1
               .     .
            .           . ea
         .                 .
      .                       . vb
   ............................................
*/

inline int region1ClosestPoint( const Vec& planep,
                                const Vec& va, const  Vec& vb, const Vec& ea,
                                Vec& closest )
{
  real ta = dot( (planep - va), ea ) / dot( ea, ea );

  if( ta >= 1 ) // then vb
  {
    closest << vb;
    return( 1 );
  }
  else if( ta <= 0 ) // then va
  {
    closest << va;
    return( 0 );
  }
  else // then ea
  {
    closest << ( va + ta*ea );
    return( 2 );
  }
}

/******************************************************************************
Description:
  Compute the closest point for a target point in region 2.  Alg is:
  1. compute the scaled distances (ta, tb) of the closest point on each edge.
  2. depending on the value, the closest point will be one of the vertices
  or one of the two adjacent edges.

Arguments:
   planep - target point in plane
   va, vb, vc - vertices of the triangle (see diagram below.
   ea, eb - edges (see below)
   closest - return value of closest point

Return:
   closest
   function returns edge_type (used to determine triangle class
   edge_type =
     0 -> point is closest to va
     1 -> closest to vb
     2 -> closest to vc
     3 -> closest to ea
     4 -> closest to eb

******************************************************************************/
/*
           .                       .
              .  Region 2       .
                 .           .
                    . vb  .
                       .
                    .     .
              ea .           . eb
              .                 .
           .                       . vc
    va  ............................................
*/

inline int region2ClosestPoint( const Vec& planep,
                                const Vec& va, const Vec& vb, const Vec& vc,
                                const Vec& ea, const Vec& eb,
                                Vec closest )
{
  real ta = dot( planep - va, ea ) / dot( ea, ea );
  real tb = dot( planep - vb, eb ) / dot( eb, eb );

  if( ta <= 0 ) // then va
  {
    closest << va;
    return( 0 );
  }
  else if( tb >=1 ) // then vc
  {
    closest << vc;
    return( 2 );
  }
  else if( ta < 1 ) // then ea
  {
    closest << ( va + ta*ea );
    return( 3 );
  }
  else if( tb > 0 ) // then eb
  {
    closest << (vb + tb*eb );
    return( 4 );
  }
  else // then vb
  {
    closest << vb;
    return( 1 );
  }
}















                              
} // end of namespace PLearn
