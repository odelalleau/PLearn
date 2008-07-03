// -*- C++ -*-

// Molecule.cc
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

/*! \file Molecule.cc */


#include "Molecule.h"
#include <plearn/io/fileutils.h>
#include <plearn/io/openFile.h>
#include <plearn/db/getDataSet.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Molecule,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

Molecule::Molecule()
{
}

Molecule::Molecule( const PPath& filename )
{
    readFromFile( filename );
    build();
}

void Molecule::readFromFile( const PPath& filename )
{
    readFromVRMLFile( filename + ".vrml" );
    readFromAMATFile( filename + ".amat" );
}

void Molecule::writeToFile( const PPath& filename )
{
    writeToVRMLFile( filename + ".vrml" );
    writeToAMATFile( filename + ".amat" );
}


void Molecule::build()
{
    inherited::build();
    build_();
}

void Molecule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);

    deepCopyField( coordinates, copies );
    deepCopyField( features, copies );
    deepCopyField( feature_names, copies );
    deepCopyField( vrml_face_set, copies );
    deepCopyField( vrml_line_set, copies );
}

void Molecule::readFromVRMLFile( const PPath& filename )
{
    // Read the geometrical informations, contained in a VRML file
    string vrml = loadFileAsString( filename );

    // Read the coordinates, and store them in coordinates
    size_t begin;
    size_t end;
    begin = vrml.find( "Coordinate3" );
    if( begin != string::npos )
    {
        begin = vrml.find( "[", begin );
        end = vrml.find( "]", begin );

        string coordinate3 = vrml.substr( begin, end - begin + 1 );
        PStream coords = openString( coordinate3, PStream::plearn_ascii );
        Vec coordinates_;
        coords >> coordinates_;
        coordinates = coordinates_->toMat( coordinates_->length() / 3, 3 );
    }
    else
        PLERROR("Molecule::readFromFile - File %s.vrml should contain a"
                " 'Coordinate3' block.\n", filename.c_str());

    // Read the other geometrical informations (edges or faces)
    // Search for edges informations
    begin = vrml.find( "IndexedFaceSet" );
    if( begin != string::npos )
    {
        begin = vrml.find( "[", begin );
        end = vrml.find( "]", begin );

        // store them in vrml_face_set
        string indexedfaceset = vrml.substr( begin, end - begin + 1 );
        PStream face_indices = openString( indexedfaceset,
                                           PStream::plearn_ascii );
        face_indices >> vrml_face_set;
    }

    // Search for edges informations
    begin = vrml.find( "IndexedLineSet" );
    if( begin != string::npos )
    {
        begin = vrml.find( "[", begin );
        end = vrml.find( "]", begin );

        // store them in vrml_line_set
        string indexedlineset = vrml.substr( begin, end - begin + 1 );
        PStream line_indices = openString( indexedlineset,
                                           PStream::plearn_ascii );
        line_indices >> vrml_line_set;
    }
}

void Molecule::readFromAMATFile( const PPath& filename )
{
    // Read the chemical informations, contained in an AMAT file
    VMat features_ = getDataSet( filename );
    features = features_->toMat();
    features.compact(); // for use as Var storage

    feature_names = features_->fieldNames();
}

void Molecule::writeToVRMLFile( const PPath& filename )
{
    PStream vrml = openFile( filename, PStream::raw_ascii, "w" );

    // writes VRML header and beginning of file
    vrml<< "#VRML V1.0 ascii" << endl
        << endl
        << "Separator {" << endl
        << "    Material {" << endl
        << "        diffuseColor [ 1 1 1 ]" << endl
        << "    }" << endl
        << endl;

    // writes coordinates
    vrml<< "    Coordinate3 {" << endl
        << "        point [" << endl;

    for( int i=0 ; i<coordinates.length() ; i++ )
    {
        vrml<< "            ";
        for( int j=0 ; j<3 ; j++ )
            vrml<< coordinates(i,j) << " ";

        vrml<< "," << endl;
    }

    vrml<< "        ]" << endl
        << "    }" << endl
        << endl;

    // writes FaceSet (if any)
    int faceset_size = vrml_face_set.size();
    if( faceset_size > 0 )
    {
        vrml<< "    IndexedFaceSet {" << endl
            << "        coordIndex [" << endl
            << "            " ;
        for( int i=0 ; i<faceset_size ; i++ )
        {
            int index = vrml_face_set[i];
            if( index < 0 )
                vrml<< "-1," << endl
                    << "            ";
            else
                vrml<< index << ", ";
        }
        vrml<< "]" << endl
            << "    }" << endl
            << endl;
    }

    // writes LineSet (if any)
    int lineset_size = vrml_line_set.size();
    if( lineset_size > 0 )
    {
        vrml<< "    IndexedLineSet {" << endl
            << "        coordIndex [" << endl
            << "            " ;
        for( int i=0 ; i<lineset_size ; i++ )
        {
            int index = vrml_line_set[i];
            if( index < 0 )
                vrml<< "-1," << endl
                    << "            ";
            else
                vrml<< index << ", ";
        }
        vrml<< "]" << endl
            << "    }" << endl
            << endl;
    }

    // end of the file
    vrml<< "}" << endl;
}

void Molecule::writeToAMATFile( const PPath& filename )
{
    VMat features_ = new MemoryVMatrix( features );
    features_->declareFieldNames( feature_names );
    features_->saveAMAT( filename, false );
}

void Molecule::declareOptions(OptionList& ol)
{
    // ### ex:
    // declareOption(ol, "myoption", &Molecule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "coordinates", &Molecule::coordinates,
                  OptionBase::buildoption,
                  "Mat containing the 3D coordinates of the surface points.");

    declareOption(ol, "features", &Molecule::features,
                  OptionBase::buildoption,
                  "Mat containing the values of the chemical features at"
                  " each point.");

    declareOption(ol, "feature_names", &Molecule::feature_names,
                  OptionBase::buildoption,
                  "Name of the chemical features stored in 'features'.");

    declareOption(ol, "vrml_face_set", &Molecule::vrml_face_set,
                  OptionBase::learntoption,
                  "List of point indices, used to define faces in VRML.");

    declareOption(ol, "vrml_line_set", &Molecule::vrml_line_set,
                  OptionBase::learntoption,
                  "List of point indices, used to define lines in VRML.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void Molecule::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // check consistency of the sizes
    int features_length = features.length();

    if( n_features() > 0 && features_length != n_points() )
        PLERROR("In Molecule::build_ - features.length() should be equal to\n"
                "coordinates.length(), unless features is empty (%d != %d).\n",
                features_length, n_points() );

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
