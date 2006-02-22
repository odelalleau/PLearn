// -*- C++ -*-

// MoleculeTemplate.cc
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

/*! \file MoleculeTemplate.cc */


#include "MoleculeTemplate.h"
#include <plearn/math/TMat.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MoleculeTemplate,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

MoleculeTemplate::MoleculeTemplate()
   : class_label( -1 )
    /* ### Initialize all fields to their default value */
{
    // ...
}

MoleculeTemplate::MoleculeTemplate( const Molecule& molecule,
                                    const Vec& the_geom_dev,
                                    const Mat& the_feat_dev,
                                    int the_class_label )
    : inherited( molecule ),
      geom_dev( the_geom_dev ),
      feat_dev( the_feat_dev ),
      class_label( the_class_label )
{
}

void MoleculeTemplate::writeToFile( const PPath& filename )
{
    writeVRMLToFile( filename );

    VMat features_ = new MemoryVMatrix( features );
    features_->declareFieldNames( feature_names );

    VMat geom_dev_ = new MemoryVMatrix( geom_dev.toMat(geom_dev.length(), 1) );
    TVec<string> geom_dev_names( 1, "geom_dev" );
    geom_dev_->declareFieldNames( geom_dev_names );

    VMat feat_dev_ = new MemoryVMatrix( feat_dev );
    TVec<string> feat_dev_names = feature_names.copy();
    int n_features = feature_names.length();
    for( int i=0 ; i<n_features ; i++ )
        feat_dev_names[i] += "_dev";
    feat_dev_->declareFieldNames( feat_dev_names );

    VMat all = hconcat( features_, hconcat( geom_dev_, feat_dev_ ) );
    all->defineSizes( all->width(), 0, 0 );
    all->saveAMAT( filename + ".amat", false );
}

void MoleculeTemplate::build()
{
    inherited::build();
    build_();
}

void MoleculeTemplate::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);

    deepCopyField(geom_dev, copies);
    deepCopyField(feat_dev, copies);
}

void MoleculeTemplate::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &MoleculeTemplate::myoption, OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "geom_dev", &MoleculeTemplate::geom_dev,
                  OptionBase::buildoption,
                  "Standard deviations of the geometrical distance");

    declareOption(ol, "feat_dev", &MoleculeTemplate::feat_dev,
                  OptionBase::buildoption,
                  "Standard deviations of each chemical property");

    declareOption(ol, "class_label", &MoleculeTemplate::class_label,
                  OptionBase::buildoption,
                  "Class label (0 for inactive, 1 for active, -1 for"
                  " uninitialized");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MoleculeTemplate::build_()
{
    // Size check
    int n_points = coordinates.length();
    int geom_dev_length = geom_dev.length();
    int feat_size = features.width();
    int feat_dev_length = feat_dev.length();
    int feat_dev_width = feat_dev.width();

    // TODO: resize if empty?
    if( geom_dev_length == 0 )
    {
        PLWARNING( "MoleculeTemplate::build_ - geom_dev.length() == 0,\n"
                   "resizing to coordinates.length() (%d), and filling with"
                   " 1's.\n", n_points );
        geom_dev = Vec( n_points, 1 );
    }
    else if( geom_dev_length != n_points )
        PLERROR( "MoleculeTemplate::build_ - geom_dev.length() should be equal"
                 " to\n"
                 "coordinates.length() (%d != %d).\n",
                 geom_dev_length, n_points );

    if( feat_dev_length == 0 && feat_dev_width == 0 )
    {
        PLWARNING( "MoleculeTemplate::build_ - feat_dev.length() == 0 and\n"
                   "feat_dev.width() == 0. Resizing to coordinates sizes\n"
                   "(%d × %d), and filling with 1's.\n", n_points, feat_size );
        feat_dev = Mat( n_points, feat_size, 1 );
    }
    else if( feat_dev_length != n_points )
        PLERROR( "MoleculeTemplate::build_ - feat_dev.length() should be equal"
                 " to\n"
                 "coordinates.length() (%d != %d).\n",
                 feat_dev_length, n_points );
    else if( feat_dev_width != feat_size )
        PLERROR( "MoleculeTemplate::build_ - feat_dev.width() should be equal"
                 " to\n"
                 "features.width() (%d != %d).\n", feat_dev_width, feat_size );

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
