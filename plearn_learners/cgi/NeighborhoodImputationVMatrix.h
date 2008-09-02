// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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


/* ******************************************************************      
   * $Id: NeighborhoodImputationVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
   ****************************************************************** */

/*! \file NeighborhoodImputationVMatrix.h */

#ifndef NeighborhoodImputationVMatrix_INC
#define NeighborhoodImputationVMatrix_INC

#include <plearn/vmat/ImputationVMatrix.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/io/fileutils.h>                     //!<  For isfile()
#include <plearn/math/BottomNI.h>

namespace PLearn {
using namespace std;

class NeighborhoodImputationVMatrix: public ImputationVMatrix
{
  typedef ImputationVMatrix inherited;
  
public:

  //! The set of pre-computed neighbors index.
  //! This can be done with BallTreeNearestNeighbors.
  VMat                          reference_index;
  
  //! The reference set corresponding to the pre-computed index with missing values.
  VMat                          reference_with_missing;
  
  //! The reference set corresponding to the pre-computed index with the initial imputations.
  VMat                          reference_with_covariance_preserved;
   
  //! This is usually called K, the number of neighbors to consider.
  //! It must be less or equal than the with of the reference index.
  int                           number_of_neighbors;

  //! A vector that give for each variable the number of neighbors to use
  //! If a variable is not in the spec, it will use number_of_neighbors
  TVec< pair<string, int> >  imputation_spec;

  //!0: (default) We do not count a neighbour with a missing value in the number of neighbors
  //!1: We count a neighbour with a missing value in the number of neighbors
  int                           count_missing_neighbors;

                        NeighborhoodImputationVMatrix();
  virtual               ~NeighborhoodImputationVMatrix();

  static void           declareOptions(OptionList &ol);

  virtual void          build();
  virtual void          makeDeepCopyFromShallowCopy(CopiesMap& copies);

  virtual void         getExample(int i, Vec& input, Vec& target, real& weight);
  virtual real         get(int i, int j) const;
  virtual void         put(int i, int j, real value);
  virtual void         getSubRow(int i, int j, Vec v) const;
  virtual void         putSubRow(int i, int j, Vec v);
  virtual void         appendRow(Vec v);
  virtual void         insertRow(int i, Vec v);  
  virtual void         getRow(int i, Vec v) const;
  virtual void         putRow(int i, Vec v);
  virtual void         getColumn(int i, Vec v) const;

private:
  
          int          src_length;
          int          src_width;
          int          ref_length;
          Mat          ref_idx;
          Mat          ref_mis;
          Mat          ref_cov;
          TVec<int>    nb_neighbors;

          void         build_();
          real         impute(int i, int j) const;
  
  PLEARN_DECLARE_OBJECT(NeighborhoodImputationVMatrix);

};

DECLARE_OBJECT_PTR(NeighborhoodImputationVMatrix);

} // end of namespcae PLearn
#endif
