// -*- C++ -*-

// GeodesicDistanceKernel.h
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: GeodesicDistanceKernel.h,v 1.5 2004/07/19 13:25:54 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file GeodesicDistanceKernel.h */


#ifndef GeodesicDistanceKernel_INC
#define GeodesicDistanceKernel_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;

class GeodesicDistanceKernel: public Kernel
{

private:

  typedef Kernel inherited;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  

public:

  // ************************
  // * Public build options *
  // ************************

  Ker distance_kernel;
  string geodesic_file;
  int knn;
  bool pow_distance;
  string shortest_algo;
  VMat geo_distances;
  
  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  GeodesicDistanceKernel();

  //! Convenient constructor.
  GeodesicDistanceKernel(Ker the_distance_kernel, int the_knn = 10,
      string the_geodesic_file = "", bool the_pow_distance = false);

  // ******************
  // * Kernel methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(GeodesicDistanceKernel);

  // **************************
  // **** Kernel methods ****
  // **************************

  //! Return the index j of the data point which satisfies:
  //!   1. j is among the knn nearest neighbors of the point x from which was
  //!      computed k_xi_x_sorted
  //!   2. it is such that dist(x,j) + geodesic_dist(j,x_i) is minimum
  int computeNearestGeodesicNeighbour(int i, const Mat& k_xi_x_sorted) const;

  //! Return the shortest distance to i from a point x whose distance
  //! to its knn nearest neighbors in the training set is given by
  //! the matrix 'k_xi_x_sorted'.
  real computeShortestDistance(int i, const Mat& k_xi_x_sorted) const;

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  virtual real evaluate_i_j(int i, int j) const;

  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;

  virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;

  //! Evaluate K(x_i,x) where x is not given explicitly, but only by the
  //! (sorted) distances to all training points.
  virtual real evaluate_i_x_from_distances(int i, const Mat& k_xi_x_sorted) const; 

  //! Overridden to precompute inter-points geodesic distance.
  virtual void setDataForKernelMatrix(VMat the_data);

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void computeGramMatrix(Mat K) const;
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;
  

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(GeodesicDistanceKernel);
  
} // end of namespace PLearn

#endif

