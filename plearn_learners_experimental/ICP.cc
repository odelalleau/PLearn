// -*- C++ -*-

// ICP.cc
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
   * $Id: ICP.cc,v 1.23 2005/12/29 13:44:35 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file ICP.cc */


#include "ICP.h"

namespace PLearn {
using namespace std;

ICP::ICP() :
  weight_method( "dynamic" ),
  sigmoid_dmid( 500 ),
  sigmoid_k( 1 ),
  lorentz_sigma( 100 ),
  dynamic_d( 100 ),
  static_thresh( 1000 ),
  error_t( 0 ),
  dist_t( 10 ),
  angle_t( 0 ),
  trans_t( 0 ),
  max_iter( 20 ),
  normal_t( DEG2RAD * 90 ),
  area_weight( false ),
  overlap_filter( false ),
  overlap_delay( 0 ),
  smart_overlap( false ),
  smart_overlap_t( 0 ),
  n_per( 0 ),
  fine_matching( false ),
  read_trans_file( false ),
  write_trans_file( false ),
  write_vtx_match( false ),
  write_all_vtx_match( false ),
  verbosity( 1 )
{}

ICP::ICP( SurfMesh a_model, SurfMesh a_scene,
          GenericNN btl_init ) :
  weight_method( "dynamic" ),
  sigmoid_dmid( 500 ),
  sigmoid_k( 1 ),
  lorentz_sigma( 100 ),
  dynamic_d( 100 ),
  static_thresh( 1000 ),
  error_t( 0 ),
  dist_t( 10 ),
  angle_t( 0 ),
  trans_t( 0 ),
  max_iter( 20 ),
  normal_t_deg( 90 ),
  area_weight( false ),
  overlap_filter( false ),
  overlap_delay( 0 ),
  smart_overlap( false ),
  smart_overlap_t( 0 ),
  n_per( 0 ),
  fine_matching( false ),
  read_trans_file( false ),
  write_trans_file( false ),
  write_vtx_match( false ),
  write_all_vtx_match( false ),
  verbosity( 1 )
{
  model = a_model;
  scene = a_scene;

  if( !btl_init )
  {
    btl->setTrainingSet( scene->getVertexCoordsAndFeatures() );
    btl->setOption( "nstages", "-1" );
//    btl->rmin = 1;
    btl->build();
    btl->train();
    btl->num_neighbors = 1;
    btl->copy_input = true;
    btl->copy_target = false;
    btl->copy_weight = false;
    btl->copy_index = true;
  }
  else
  {
    btl = btl_init;
  }

}


PLEARN_IMPLEMENT_OBJECT(ICP,
    "Performs the alignement of two surface meshes by the ICP method",
    "ICP stands for \"Iterative Closest Point\""
);

void ICP::declareOptions(OptionList& ol)
{
  declareOption(ol, "weight_method", &ICP::weight_method, 
                OptionBase::buildoption,
                "Method used to weight the points:\n"
                " - \"sigmoid\": sigmoid function\n"
                " - \"lorentz\": Lorentzian function\n"
                " - \"dynamic\": dynamic threshold\n"
                " - \"static\": static threshold\n" );

  declareOption(ol, "sigmoid_dmid", &ICP::sigmoid_dmid, OptionBase::buildoption,
                "midpoint offset parameter in sigmoid function");

  declareOption(ol, "sigmoid_k", &ICP::sigmoid_k, OptionBase::buildoption,
                "slope parameter in sigmoid function");

  declareOption(ol, "lorentz_sigma", &ICP::lorentz_sigma,
                OptionBase::buildoption,
                "sigma parameter in Lorentzian function");

  declareOption(ol, "dynamic_d", &ICP::dynamic_d, OptionBase::buildoption,
                "D parameter in dynamic thresholding");

  declareOption(ol, "static_thresh", &ICP::static_thresh,
                OptionBase::buildoption,
                "threshold distance for static thresholding");

  declareOption(ol, "error_t", &ICP::error_t, OptionBase::buildoption,
                "stop when average point distance falls below this");

  declareOption(ol, "dist_t", &ICP::dist_t, OptionBase::buildoption,
                "stop when max points movement falls below this");

  declareOption(ol, "angle_t", &ICP::angle_t, OptionBase::buildoption,
                "stop when max rotation change falls below this (degrees)");

  declareOption(ol, "trans_t", &ICP::trans_t, OptionBase::buildoption,
                "stop when translation distance falls below this");

  declareOption(ol, "max_iter", &ICP::max_iter, OptionBase::buildoption,
                "stop when number of iterations reaches this");

  declareOption(ol, "normal_t_deg", &ICP::normal_t_deg, OptionBase::buildoption,
                "angle threshold (degrees) for compatible normal");

  declareOption(ol, "area_weight", &ICP::area_weight, OptionBase::buildoption,
                "enables weighting by area");

  declareOption(ol, "overlap_filter", &ICP::overlap_filter, 
                OptionBase::buildoption,
                "filter non-overlapping points");

  declareOption(ol, "overlap_delay", &ICP::overlap_delay, 
                OptionBase::buildoption,
                "number of iterations before overlap filtering is enabled");

  declareOption(ol, "smart_overlap_t", &ICP::smart_overlap_t,
                OptionBase::buildoption,
                "enable overlap filter when average point distance is below this");

  declareOption(ol, "n_per", &ICP::n_per, OptionBase::buildoption,
                "number of perturbations about minimum");

  declareOption(ol, "fine_matching", &ICP::fine_matching,
                OptionBase::buildoption,
                "if true, will match model vertices to any scene point,\n"
                "if false, will match vertices only to scene vertices.\n");

  declareOption(ol, "model_file", &ICP::model_file, OptionBase::buildoption,
                "file containing the model we're trying to align on the scene");

  declareOption(ol, "scene_file", &ICP::scene_file, OptionBase::buildoption,
                "file containing the scene we're trying to align the model on");

  declareOption(ol, "output_file", &ICP::output_file, OptionBase::buildoption,
                "transformed model");

  declareOption(ol, "initial_transform", &ICP::initial_transform,
                OptionBase::buildoption,
                "initial transformation to apply, \"tx ty tz rx ry rz\"\n"
                "this option is overriden by read_trans_file\n" );

  declareOption(ol, "read_trans_file", &ICP::read_trans_file,
                OptionBase::buildoption,
                "read initial transformation from \"trans_file\"");

  declareOption(ol, "write_trans_file", &ICP::write_trans_file,
                OptionBase::buildoption,
                "write final transformation into file \"trans_file\"");

  declareOption(ol, "trans_file", &ICP::trans_file, OptionBase::buildoption,
                "file containing a transformation. syntax of this file is:\n"
                "tx ty tz rx ry rz\\n");

  declareOption(ol, "write_vtx_match", &ICP::write_vtx_match,
                OptionBase::buildoption,
                "write final matching between model and scene vertices in \"vtx_match_file\"");

  declareOption(ol, "write_all_vtx_match", &ICP::write_all_vtx_match,
                OptionBase::buildoption,
                "force to match _each_ model vertex to a scene vertex \n"
                "(wether it was used for the alignment or not).\n" );

  declareOption(ol, "vtx_match_file", &ICP::vtx_match_file,
                OptionBase::buildoption,
                "File containing the final match between model and vertices point indices");

  declareOption(ol, "verbosity", &ICP::verbosity,
                OptionBase::buildoption,
                "Level of verbosity. If 0 should not write anything on perr.\n"
                "If >0 may write some info on the steps performed along"
                " the way.\n"
                "The level of details written should depend on this value.\n"
               );

/*
  declareOption(ol, "tx", &ICP::tx, OptionBase::buildoption,
                "translation wrt x");

  declareOption(ol, "ty", &ICP::ty, OptionBase::buildoption,
                "translation wrt y");

  declareOption(ol, "tz", &ICP::tz, OptionBase::buildoption,
                "translation wrt z");

  declareOption(ol, "rx", &ICP::rx, OptionBase::buildoption,
                "rotation wrt x");

  declareOption(ol, "ry", &ICP::ry, OptionBase::buildoption,
                "rotation wrt y");

  declareOption(ol, "rz", &ICP::rz, OptionBase::buildoption,
                "rotation wrt z");
*/
  declareOption(ol, "model", &ICP::model, OptionBase::buildoption,
                "the model we're trying to align on the scene");

  declareOption(ol, "scene", &ICP::scene, OptionBase::buildoption,
                "the scene we're trying to align the model on");

  declareOption(ol, "model_features", &ICP::model_features,
                OptionBase::buildoption,
                "Matrix containing the chemical features of each vertex of"
                " the model");

  declareOption(ol, "scene_features", &ICP::scene_features,
                OptionBase::buildoption,
                "Matrix containing the chemical features of each vertex of"
                " the scene");

  declareOption(ol, "btl", &ICP::btl, OptionBase::buildoption,
                "BTreeLearner, containing efficiently the scene points");

  declareOption(ol, "match", &ICP::match, OptionBase::learntoption,
                "matching of the model on the scene");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void ICP::build_()
{
  normal_t = DEG2RAD * normal_t_deg;

  if( fast_is_equal(smart_overlap_t, 0., REAL_MAX, 1e-5) )
    smart_overlap = false;
  else
    smart_overlap = true;

  string wm = lowerstring( weight_method );
  if( wm == "sigmoid" )
    weight_method = wm;
  else if( wm == "lorentz" )
    weight_method = wm;
  else if( wm == "dynamic" )
    weight_method = wm;
  else if( wm == "static" )
    weight_method = wm;
  else if( wm == "oracle" )
    weight_method = wm;
  else
    PLERROR( "weight_method \"%s\" not supported", weight_method.c_str() );

  buildMeshes();
}

void ICP::build()
{
  inherited::build();
  build_();
}

real ICP::dynamicDistanceThreshold( const Mat model_pts, const Mat scene_pts,
                                    const real d, const real d_max )
{
  /* calculate distance statistics */
  int n = model_pts.length();
  real sumd = 0;
  real sumd2 = 0;

  for( int i = 0 ; i < n ; i++ )
  {
    real d = powdistance( model_pts( i ), scene_pts( i ), 2 );
    sumd2 += d;
    sumd += sqrt( d );
  }

  real davg = sumd/n;
  real dsig = sqrt( sumd2/n - davg*davg );

  /* set dynamic threshold */
  if( davg < d )
  {
    return davg + 3*dsig;
  }
  else if( davg < 3*d )
  {
    return davg + 2*dsig;
  }
  else if( davg < 6*d )
  {
    return davg + dsig;
  }
  else
  {
    return d_max;
  }
}

real ICP::computeWeightedDistance( const Mat m1_pts, const Mat m2_pts,
                                   const Vec weights )
/* hey, on pourrait avoir une VMat avec des poids et tout... */
{
  int n = m1_pts.length();
  real weight_total = 0;
  real error_total = 0;

  for( int i=0 ; i<n ; i++ )
  {
    real distance = dist( m1_pts( i ), m2_pts( i ), 2 );
    weight_total += weights[ i ];
    error_total += weights[ i ] * distance;
  }

  if( weight_total > 0 )
  {
    return( error_total / weight_total );
  }
  else
  {
    return 0;
  }
}

void ICP::computeWeights( const Mat model_pts, const Mat scene_pts,
                          const Vec areas, Vec weights,
                          real& min_wt, real& max_wt )
{
  int n = model_pts.length();
  real total_weight = 0;
  max_wt = 0;
  min_wt = INFINITY;

  if( area_weight )
    PLERROR( "area weighting not implemented yet, please come back later" );


  if( ( weight_method == "static" ) || ( weight_method == "dynamic" ) )
  {
    /*if( area_weight )
      PLERROR( "area weighting not implemented yet, please come back later" );
    */
    weights = 1./n;
    max_wt = weights[0];
    min_wt = weights[0];
  }
  else if( weight_method == "sigmoid" )
  {
    for( int i=0 ; i<n ; i++ )
    {
      real distance = dist( model_pts( i ), scene_pts( i ), 2 );
      real exp_part = exp( sigmoid_k * ( sigmoid_dmid - distance ) );
      weights[ i ] = exp_part / ( 1.0 + exp_part );
      /*if( area_weight )
      {
        PLERROR( "area weighting not implemented yet, please come back later" );
      }*/
      total_weight += weights[ i ];
    }
    weights /= total_weight;
    min_wt = min( weights );
    max_wt = max( weights );
  }
  else if( weight_method == "lorentz" )
  {
    for( int i=0 ; i<n ; i++ )
    {
      real dist2 = powdistance( model_pts( i ), scene_pts( i ), 2 );
      dist2 /= ( lorentz_sigma * lorentz_sigma );
      weights[ i ] = 1.0 / ( 1.0 + 0.5 * dist2 );
      /*if( area_weight )
      {
        PLERROR( "area weighting not implemented yet, please come back later" );
      }*/
      total_weight += weights[ i ];
    }
    weights /= total_weight;
    min_wt = min( weights );
    max_wt = max( weights );
  }

  else if( weight_method == "oracle" )
  {
    for( int i=0 ; i<n ; i++ )
    {
      real w = weightOracle( (int) vtx_matching( i, 0 ), model_pts(i),
                             (int) vtx_matching( i, 1 ), scene_pts(i) );
      weights[i] = w;
      total_weight += w;
    }
    weights /= total_weight;
    min_wt = min( weights );
    max_wt = max( weights );
  }

}


bool ICP::iterativeReweight( Mat model_pts, const Mat scene_pts,
                             const Vec areas, Vec weights,
                             const Mat init_corners, const int max_iter,
                             Mat total_delta_rot, Vec total_delta_trans )
{
  Mat delta_rot( 3, 3 );
  Vec delta_trans( 3 );
  Vec angles( 3 );

  int n = model_pts.length();
  if( n < 3 )
  {
    PLERROR("not enough points to calculate transformation");
  }

  // initialize
  int n_iter = 0;

  real max_corner_motion = INFINITY;
  Mat cur_corners( 8, 3 );
  cur_corners << init_corners;
  Mat last_corners( 8, 3 );
  last_corners << init_corners;

  // loop until bounding box motion is small
  while( ( max_corner_motion > dist_t ) && ( n_iter < max_iter ) )
  {
    n_iter++;

    real min_wt, max_wt;
    computeWeights( model_pts, scene_pts, areas, weights, min_wt, max_wt );

    // calculate best transformation
    real error = INFINITY;
    weightedTransformationFromMatchedPoints( model_pts, scene_pts, weights,
                                             delta_rot, delta_trans, error );
    // compute max motion of bounding box and update corner points
    last_corners << cur_corners;
    transformPoints( delta_rot, delta_trans, cur_corners, cur_corners );
    max_corner_motion = maxPointMotion( last_corners, cur_corners );

    // accumulate transformation ensuring normalization
    if( n_iter == 1 )   // first time
    {
      total_delta_rot << delta_rot;
      total_delta_trans << delta_trans;
    }
    else
    {
      Mat tmp_mat( 3, 3 );
      product( tmp_mat, delta_rot, total_delta_rot );
      angles << fixedAnglesFromRotation( tmp_mat );

      total_delta_rot <<
        rotationFromFixedAngles( angles[0], angles[1], angles[2] );

      Vec tmp_vec( 3 );
      product( tmp_vec, delta_rot, total_delta_trans );
      total_delta_trans += tmp_vec;
    }

    // update model points using new transformation

    transformPoints( delta_rot, delta_trans, model_pts, model_pts );

    if( verbosity >= 3 )
    {
      pout << "  " << n_iter << " * delta trans " << delta_trans 
        << " * delta rot " << fixedAnglesFromRotation( delta_rot ) << endl
        << "  max weight " << max_wt << " * min weight " << min_wt
        << " * max motion " << max_corner_motion << endl;
    }
  }

  return true;
}

int ICP::iterate()
{
  real error = REAL_MAX;
  real max_corner_motion = REAL_MAX;
  real delta_trans_length = REAL_MAX; // ?
  real delta_angle_length = REAL_MAX; // ?

  int n_iter = 0;

  if( verbosity >=2 )
    pout << "Initial transform: " << *match << endl; // 

  Mat total_rot = match->rot;
  Vec total_trans = match->trans;

  if( area_weight )
    PLERROR( "area_weight not implemented yet. please come back later." );

  // cache scene faces by point
  TVec< set<int> > face_cache = scene->cacheNeighborFaces();

  // stores scene vertex coordinates
  Mat all_scene_coords = scene->getVertexCoords();

  // compute vertices of bounding box and apply initial transform
  Mat bbox = model->boundingBox();
  Mat orig_corners = boundingBoxToVertices( bbox );
  Mat cur_corners( 8, 3);
  cur_corners << orig_corners;

  transformPoints( total_rot, total_trans, cur_corners, cur_corners );

  real avg_pt_dist;
  int avg_pt_dist_cnt;

  // apply initial transform to
  // loop until convergence or other stopping criterion reached
  while( (error > error_t) &&
         (max_corner_motion > dist_t ) &&
         (delta_trans_length > trans_t) &&
         (delta_angle_length > angle_t) &&
         (n_iter < max_iter) )
  {
    n_iter++;

    // find scene points closest to model points
//    Vec model_vtx; vtx_matching[0,]
    Mat model_coords_feats;
//    Vec scene_vtx; vtx_matching[1,]
    Mat scene_coords_feats;

    real closest_dist;
    real dynamic_thresh = dynamic_dmax0 = 20 * dynamic_d;

    // record average distance between closest points for smart overlap enabler
    if( smart_overlap && !overlap_filter )
    {
      avg_pt_dist = 0;
      avg_pt_dist_cnt = 0;
    }

    int n_verts = model->numVertices();
    vtx_matching.resize( 0, 2 );

    vertex_iterator vi, vi_end;
    tie(vi, vi_end) = vertices( *model->p_mesh );
    int feat_size = model->getVertex( *vi )->features.size();

    for( int i=0 ; vi != vi_end ; i++, vi++ )
    {
      MVertex mv = model->getVertex( *vi );
      Vec model_coord(3);
      product( model_coord, total_rot, mv->coord );
      model_coord += total_trans;

      Vec model_coord_feat(3+feat_size);
      model_coord_feat.subVec(0,3) << model_coord;
      model_coord_feat.subVec(3,feat_size) << mv->features;

      Vec model_norm(3);
      product( model_norm, total_rot, mv->norm );

      Vec scene_coord_feat(3+feat_size);

      real init_dist_t = REAL_MAX;
//      vtx_matching[i] = false;

      int j;

      if( fine_matching )
      {
        Vec scene_coord(3);
        bool is_overlapping = isOverlapping( model_coord, model_norm,
                                             scene, face_cache, btl,
                                             init_dist_t, normal_t,
                                             j, scene_coord,
                                             closest_dist );
        scene_coord_feat.subVec(0,3) << scene_coord;

        if( overlap_filter && !is_overlapping && (n_iter > overlap_delay) )
          continue;

        if( smart_overlap && !overlap_filter )
        {
          avg_pt_dist += closest_dist;
          avg_pt_dist_cnt++;
        }
      }
      else
      {
        getNearestVertex( model_coord_feat, scene, btl,
                          j, scene_coord_feat, closest_dist );
      }

      // skip points beyond threshold
      if( (weight_method == "static") && (closest_dist > static_thresh) )
        continue;
      if( (weight_method == "dynamic") && (closest_dist > dynamic_thresh) )
        continue;

      // get vertex coordinates if we haven't them
      if( is_missing( scene_coord_feat[0] ) ||
          is_missing( scene_coord_feat[1] ) ||
          is_missing( scene_coord_feat[2] ) )
      {
        scene_coord_feat.subVec(0,3) << all_scene_coords(j);
        closest_dist = dist( scene_coord_feat.subVec(0,3), model_coord, 2 );
      }


      // add to list of pts to use in icp
      Vec vtx_matching_pair( 2 );
      vtx_matching_pair[0] = i;
      vtx_matching_pair[1] = j;
      vtx_matching.appendRow( vtx_matching_pair );
      model_coords_feats.appendRow( model_coord_feat );
      scene_coords_feats.appendRow( scene_coord_feat );
//      vtx_matching[i] = true;

    }

    //vtx_matching << model_vtx; // we keep track of the matching vertices

    int n_good = vtx_matching.length();
    if( n_good < 3 )
    {
      cerr << "Not enough points to compute transform" << endl;
      return( n_iter );
    }

    Mat delta_rot( 3, 3 );
    Vec delta_trans( 3 );

    int n_inner_iter;
    if( (weight_method == "static") || (weight_method == "dynamic")
        || (weight_method == "oracle") )
      n_inner_iter = 1;
    else
      n_inner_iter = 10;

    Vec weights( n_good );
    // iteratively reweight until convergence - updates model pts
    Mat model_coords = model_coords_feats.subMatColumns(0,3);
    Mat scene_coords = scene_coords_feats.subMatColumns(0,3);
    if( !iterativeReweight( model_coords, scene_coords,
                            Vec(), weights,
                            cur_corners, n_inner_iter,
                            delta_rot, delta_trans ) )
    {
      break;
    }
    // accumulate transformation ensuring normalization

    Mat tmp_mat( 3, 3 );
    product( tmp_mat, delta_rot, total_rot );
    Vec angles = fixedAnglesFromRotation( tmp_mat );

    total_rot = rotationFromFixedAngles( angles[0], angles[1], angles[2] );

    Vec tmp_vec( 3 );
    product( tmp_vec, delta_rot, total_trans );
    total_trans = tmp_vec + delta_trans;

    // compute error
    error = computeWeightedDistance( model_coords_feats, scene_coords_feats,
                                     weights );

    if( weight_method == "dynamic" ) // compute dynamic threshold
    {
      dynamic_thresh = dynamicDistanceThreshold( model_coords, scene_coords,
                                                 dynamic_d, dynamic_dmax0 );
    }

    // update corners and compute corner motion from start of this iteration
    Mat last_corners( 8, 3 );
    last_corners << cur_corners;
    transformPoints( total_rot, total_trans, orig_corners, cur_corners );
    max_corner_motion = maxPointMotion( last_corners, cur_corners );

    delta_trans_length = norm( delta_trans, 2 );
    delta_angle_length = norm( fixedAnglesFromRotation( delta_rot ), 2 );

    if( verbosity >= 3 )
    {
      pout << n_iter << " trans " << total_trans << " * rotate "
        << fixedAnglesFromRotation( total_rot ) << endl
        << " Wgt avg Err " << error << " * max motion "
        << max_corner_motion << " * n_pts " << n_good << " / " << n_verts
        << " * dist thresh " << dynamic_thresh
        << endl;
    }

    if( smart_overlap && !overlap_filter )
    {
      if( (error < smart_overlap_t) || (n_iter == overlap_delay) )
      {
        if( verbosity >= 2 )
        {
          pout << 
            "\n*********\n---- activating overlap filter now -----\n*********"
            << endl << endl;
        }
        overlap_filter = true;
        overlap_delay = n_iter;
      }
    }

  }

  /* store transformation in MMatch math */
  match->rot << total_rot;
  match->trans << total_trans;
  match->error = error;
  match->angles << fixedAnglesFromRotation( total_rot );

  return n_iter;
}

void ICP::buildMeshes()
{
  if( !model )
  {
    if( model_file.length() == 0 )
      PLERROR( "You need to provide either 'model' or 'model_file' option");

    model = new SurfaceMesh();

    // read in the model mesh
    if( model_features.isNull() )
    {
      if( !(model->readVRMLFile( model_file )) )
        PLERROR( "Problem reading %s\n", model_file.c_str() );
    }
    else
    {
      if( !(model->readVRMLFile( model_file, model_features )) )
        PLERROR( "Problem reading %s\n", model_file.c_str() );
    }
  }

  if( !scene )
  {
    if( scene_file.length() == 0 )
      PLERROR( "You need to provide either 'scene' or 'scene_file' option");

    scene = new SurfaceMesh();

    // red in the scene mesh
    if( scene_features.isNull() )
    {
      if( !(scene->readVRMLFile( scene_file )) )
        PLERROR( "Problem reading %s\n", scene_file.c_str() );
    }
    else
    {
      if( !(scene->readVRMLFile( scene_file, scene_features )) )
        PLERROR( "Problem reading %s\n", scene_file.c_str() );
    }
  }

  // check options compatible with information in meshes (mesh_type)
  if( scene->mesh_type == "PointSet" || model->mesh_type == "PointSet" )
    PLWARNING( "ICP without edge information is untested. Beware!" );
  if( scene->mesh_type != "FaceSet" && fine_matching )
  {
    PLWARNING( "Disabling 'fine_matching' because scene has no face information." );
    fine_matching = false;
  }

  // create BinBallTrees for closest point in scene computations
  BallTreeNN btnn = new BallTreeNearestNeighbors();
  btnn->setTrainingSet( scene->getVertexCoordsAndFeatures() );
  btnn->setOption( "nstages", "-1" );
  btnn->rmin = 1;
  btnn->build();
  btnn->train();
  btnn->num_neighbors = 1;
  btnn->copy_input = true;
  btnn->copy_target = false;
  btnn->copy_weight = false;
  btnn->copy_index = true;

  btl = btnn;

  real tx=0, ty=0, tz=0;
  real rx=0, ry=0, rz=0;

  if( initial_transform != "" )
  {
    istringstream in( initial_transform );
    if( !( in >> tx >> ty >> tz >> rx >> ry >> rz ) )
    {
      PLERROR( "initial_transform option in ICP should have syntax: \"tx ty tz rx ry rz\"" );
    }
  }

  if( read_trans_file )
  {
    ifstream in( trans_file.c_str() );
    if( !in )
      PLERROR( "Cannot open file: %s (trans_file option).\n",
               trans_file.c_str() );

    in >> tx >> ty >> tz >> rx >> ry >> rz;
  }

  // initial match
  match = new MeshMatch();
  match->rot = rotationFromFixedAngles( rx, ry, rz );
  match->trans[0] = tx;
  match->trans[1] = ty;
  match->trans[2] = tz;
  match->angles[0] = rx;
  match->angles[1] = ry;
  match->angles[2] = rz;

}

void ICP::run()
{
  if( verbosity >= 1 )
  {
    pout << "Weight method: " << weight_method << endl
      << "Area weight: " << area_weight << endl
      << "Overlap filter: " << overlap_filter << endl
      << "Overlap Threshold: " << smart_overlap_t << endl
      << endl;
  }

  // determine the first best transformation
  int total_iterations = iterate();
  MMatch  m_best = match;

  if( verbosity >= 2 )
    pout << "Best registration so far\n" << *m_best << endl; // ??

  // randomly perturb transformation about minimum to get global minimum
  for( int i=0 ; i<n_per ; i++ )
  {
    Mat random_r( 3, 3 );
    Vec random_t( 3 );
    randomTransformation( 5, model->getResolution(), random_r, random_t );

    // init transform
    product( match->rot, m_best->rot, random_r );
    match->angles = fixedAnglesFromRotation( match->rot );
    match->trans = m_best->trans + random_t;

    total_iterations += iterate();

    if( match->error < m_best->error )
    {
      m_best = match;
    }

    if( verbosity >=3 )
    {
      pout << "Best registration after " << i << " perturbations\n"
        << *m_best << endl;
    }
  }

  if( verbosity >=2 )
  {
    pout << "Best Registration\n" << *m_best << endl;
    pout << "Total iterations: " << total_iterations << endl;
  }

  if( output_file.length()!=0 ) // write out transformed points
  {
    transformMesh( m_best->rot, m_best->trans, model );
    model->writeVRMLFile( output_file );
  }

  if( write_trans_file )
  {
    ofstream out( trans_file.c_str(), ios::trunc );
    if( !out )
    {
      PLERROR( "Cannot open file: %s.\n", trans_file.c_str() );
    }

    out << m_best->trans[0] << " " << m_best->trans[1] << " " << m_best->trans[2] << " " << m_best->angles[0] << " " << m_best->angles[1] << " " << m_best->angles[2]
        << endl;
  }

  if( write_vtx_match )
  {
    Mat vtx_match;

    ofstream out( vtx_match_file.c_str(), ios::trunc );
    if( !out )
      PLERROR( "Cannot open file: %s.\n", vtx_match_file.c_str() );

//    vtx_match = computeVertexMatches();
    out << vtx_matching << endl;
  }

}

/*
Mat ICP::computeVertexMatches()
{
  Mat vtx_match( 0, 0 );

  vertex_iterator vi, vi_end;
  tie(vi, vi_end) = vertices( *model->p_mesh );
  for( int i=0 ; vi != vi_end ; vi++, i++ )
  {
    if( !write_all_vtx_match && !vtx_matching[i] )
      continue;

    MVertex mv = model->getVertex( *vi );

    Vec dists;
    Vec outputs;
    btl->computeOutputAndCosts( mv->coord, Vec(), outputs, dists );
    int closest_vertex = (int) outputs[0];

    // store match
    Vec vmatch(2);
    vmatch[0] = i;
    vmatch[1] = closest_vertex;
    vtx_match.appendRow( vmatch );
  }
  return vtx_match;
}
*/


// This should become a pointer to a function returning the weight of
// one pair of points, given their indices and coordinates
real ICP::weightOracle( int, Vec, int, Vec )
{
  return real(1);
}

void ICP::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);
  deepCopyField( model_features, copies );
  deepCopyField( scene_features, copies );


  // ### Remove this line when you have fully implemented this method.
  PLERROR("ICP::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn
