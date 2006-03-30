// -*- C++ -*-

// MoleculeTemplate.h
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

/*! \file MoleculeTemplate.h */


#ifndef MoleculeTemplate_INC
#define MoleculeTemplate_INC

#include "Molecule.h"

namespace PLearn {
using namespace std;

class MoleculeTemplate;
typedef PP<MoleculeTemplate> MolTemplate;

/**
 * Subclass of Molecule, plus standard devs of points' positions and features.
 * There is only one geometric standard deviation per point (since space
 * dimenstions are equivalent), there is one chemical standard deviation per
 * chemical feature on every point (stored in the same order as the
 * corresponding feature value).
 *
 */
class MoleculeTemplate : public Molecule
{
    typedef Molecule inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here

    //! standard deviations of the geometrical distance
    Vec geom_dev;

    //! standard deviations of each chemical property
    Mat feat_dev;

    //! class label, 0 for inactive, 1 for active, -1 for uninitialized
    int class_label;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    MoleculeTemplate( int the_class_label = -1 );

    //! Constructor from files
    MoleculeTemplate( const PPath& filename, int the_class_label = -1 );

    //! Constructor from parent
    MoleculeTemplate( const Molecule& molecule,
                      const Vec& the_geom_dev = Vec(),
                      const Mat& the_feat_dev = Mat(),
                      int the_class_label = -1 );

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT 
    PLEARN_DECLARE_OBJECT(MoleculeTemplate);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################
    //! Override inherited function to read also deviation info, if able
    virtual void readFromAMATFile( const PPath& filename );

    //! Override inherited function to write also deviation info
    virtual void writeToAMATFile( const PPath& filename );

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private: 
    //#####  Private Member Functions  ########################################

    //! This does the actual building. 
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MoleculeTemplate);
  
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
