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
   * $Id: ImputationVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
   ****************************************************************** */

/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef ImputationVMatrix_INC
#define ImputationVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/io/fileutils.h>                     //!<  For isfile()
#include <plearn/math/BottomNI.h>

namespace PLearn {
using namespace std;

class ImputationVMatrix: public VMatrix
{
  typedef VMatrix inherited;
  
public:

  //! The source VMatrix with missing values.
  VMat                  source;
  int                   test_level;

                        ImputationVMatrix();
  virtual               ~ImputationVMatrix();

  virtual void          build();
  virtual void          makeDeepCopyFromShallowCopy(CopiesMap& copies);
          void          testResultantVMatrix();
protected:
  //! Declares this class' options
  // (Please implement in .cc)
  static void           declareOptions(OptionList &ol);

private:
  
         void           build_();
  
  PLEARN_DECLARE_ABSTRACT_OBJECT(ImputationVMatrix);

};

DECLARE_OBJECT_PTR(ImputationVMatrix);

} // end of namespcae PLearn
#endif
