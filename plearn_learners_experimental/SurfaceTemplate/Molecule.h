// -*- C++ -*-

// Molecule.h
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

/*! \file PLearn/plearn_learners_experimental/SurfaceTemplate/Molecule.h */


#ifndef Molecule_INC
#define Molecule_INC

#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

class Molecule;
typedef PP<Molecule> Mol;

/**
 * A molecular surface, represented by a list of points and features on them.
 * The 3D coordinates and values of some chemical features are stored, as well
 * as the name of these features, and informations that allow to save it as a
 * VRML file.
 * This class is usually built from a pair of (.vrml, .amat) files, or by
 * deepCopy of an existing object.
 *
 */
class Molecule : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! Mat containing the 3D coordinates of the surface points.
    Mat coordinates;

    //! Mat containing the values of the chemical features at each point.
    Mat features;

    //! Name of the chemical features stored in 'features'.
    TVec<string> feature_names;

    //! List of point indices, used to define faces in VRML.
    TVec<int> vrml_face_set;

    //! List of point indices, used to define lines in VRML.
    TVec<int> vrml_line_set;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    Molecule();

    //! Constructor from files (call readFromFile). Usually used.
    Molecule( const PPath& filename );

    //! reads geometry from filename.vrml and features from filename.amat
    virtual void readFromFile( const PPath& filename );

    //! writes geometry in filename.vrml and features in filename.amat
    virtual void writeToFile( const PPath& filename );

    //! returns the number of points on the surface
    inline int n_points() { return coordinates.length(); }

    //! returns the dimension of the features vector
    virtual int n_features() { return features.width(); }


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(Molecule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

protected:
    //#####  Protected Member Functions  ######################################

    //! Reads geometric informations from file
    virtual void readFromVRMLFile( const PPath& filename );

    //! Reads features informations from file
    virtual void readFromAMATFile( const PPath& filename );

    //! Saves the geometric informations in file
    virtual void writeToVRMLFile( const PPath& filename );

    //! Saves the features informations in file
    virtual void writeToAMATFile( const PPath& filename );

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Molecule);

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
