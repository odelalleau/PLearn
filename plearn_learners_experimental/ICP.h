%// -*- C++ -*-

// ICP.h
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
   * $Id: ICP.h,v 1.9 2005/12/29 13:44:35 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file ICP.h */


#ifndef ICP_INC
#define ICP_INC

#include <plearn/base/Object.h>
#include <plearn/math/pl_math.h> // for MISSING_VALUE
#include <plearn_learners/nearest_neighbors/BallTreeNearestNeighbors.h>
//#include "../balltrees/BTreeLearner.h"
#include "SurfaceMesh.h"
#include "MeshMatch.h"
#include "geometry.h"

namespace PLearn {
using namespace std;

// And now, the ICP class

class ICP: public Object
{

private:
  
  typedef Object inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  real delta_trans_length;
  real delta_angle_length;
  real dynamic_dmax0;

public:

  // ************************
  // * public build options *
  // ************************

  // weighting parameters
//  enum Weight_method{ SIGMOID, LORENTZ, DYNAMIC, STATIC };
  string weight_method;
  real sigmoid_dmid;
  real sigmoid_k;
  real lorentz_sigma;
  real dynamic_d;
  real static_thresh;

  // stopping criteria
  real error_t;
  real dist_t;
  real angle_t;
  real trans_t;
  int max_iter;

  // other controls
  real normal_t;
  real normal_t_deg;
  bool area_weight;
  bool overlap_filter;
  int overlap_delay;
  bool smart_overlap;
  real smart_overlap_t;
  int n_per;
  bool fine_matching;

  // build parameters
  string model_file;
  string scene_file;
  string output_file;

  string initial_transform;
  bool read_trans_file;
  bool write_trans_file;
  string trans_file;

  // match
  MMatch match;
  Mat vtx_matching;
  bool write_vtx_match;
  bool write_all_vtx_match;
  string vtx_match_file;

/*
  real tx, ty, tz;
  real rx, ry, rz;
*/
  SurfMesh model;
  SurfMesh scene;
  GenericNN btl;
  Mat model_features;
  Mat scene_features;

  //! Level of verbosity. If 0 should not write anything on perr.
  //! If >0 may write some info on the steps performed along the way.
  //! The level of details written should depend on this value.
  int verbosity;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  ICP();

  //! Other constructor.
  ICP( SurfMesh a_model, SurfMesh a_scene, GenericNN btl_init );

  //! Virtual destructor.
  virtual ~ICP() {}


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();
  void buildMeshes();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

  void meshVertexAreas( const SurfMesh& mesh, Vec& areas );

  real dynamicDistanceThreshold( const Mat model_pts, const Mat scene_pts,
                                 const real d, const real d_max );

  real computeWeightedDistance( const Mat m1_pts, const Mat m2_pts,
                                const Vec weights );

  void computeWeights( const Mat model_pts, const Mat scene_pts,
                       const Vec areas, Vec weights,
                       real& min_wt, real& max_wt );

  bool iterativeReweight( Mat model_pts, const Mat scene_pts,
                          const Vec areas, Vec weights,
                          const Mat init_corners, const int max_iter,
                          Mat total_delta_rot, Vec total_delta_trans );

//  Mat computeVertexMatches();
  real weightOracle( int, Vec, int, Vec );

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(ICP);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  int iterate();

  void run();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ICP);
  
} // end of namespace PLearn

#endif
