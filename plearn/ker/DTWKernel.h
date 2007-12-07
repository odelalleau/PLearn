// -*- C++ -*-

// DTWKernel.h
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file DTWKernel.h */


#ifndef DTWKernel_INC
#define DTWKernel_INC

#include <plearn/ker/Kernel.h>

namespace PLearn {

/**
 * Kernel for Dynamic Time Warping
 * see sect.4.7 of Rabiner, L. and Juang, B. 1993 'Fundamentals of Speech Recognition'. Prentice-Hall, Inc.
 *
 * WARNING: THIS CLASS IS *NOT* THREAD-SAFE (has mutable pre-allocated data members)
 * 
 * TODO: Add global path constraints and other goodies
 */

class DTWKernel : public Kernel
{
    typedef Kernel inherited;

public:

    //#####  Typedefs  ########################################################

    // ** Local path constraints:

    //! LocalStep specifies an (x,y) offset and associated cost (weight)
    typedef pair<pair<int, int>, real> LocalStep;

    //! LocalPath is a series of LocalSteps taken in one iteration
    typedef TVec<LocalStep> LocalPath;

    //#####  Public Build Options  ############################################

    //! length of each sub-vec within an example
    int subvec_length;

    //! allowed local paths (incl. associated costs)
    TVec<LocalPath> local_paths;

    //! name of the 'distance' function to use when comparing features (sub-vecs)
    string distance_type;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DTWKernel();

    //#####  Kernel Member Functions  #########################################

    //! Compute all distances and paths (to mutalbe vars)
    void dtw(const Vec& x1, const Vec& x2) const;
    tuple<Mat, Mat, TMat<pair<int, int> > > remote_dtw(const Vec& x1, const Vec& x2) const;

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //#####  PLearn::Object Protocol  #########################################

    PLEARN_DECLARE_OBJECT(DTWKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    //! point-to-point distances (pre-alloc'd)
    mutable Mat dpoint;

    //! path length from (0,0) to (i,j) (pre-alloc'd)
    mutable Mat dpath;

    //! back-pointers of optimal paths (pre-alloc'd)
    mutable TMat<pair<int,int> > bptrs;

    //! actual pointer to distance function
    real (*dist_fn)(const Vec&,const Vec&);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DTWKernel);

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
