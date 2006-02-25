// -*- C++ -*-

// ChemicalICP.h
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

/*! \file ChemicalICP.h */


#ifndef ChemicalICP_INC
#define ChemicalICP_INC

#include <plearn/base/Object.h>
#include <plearn/var/Var.h>
#include <plearn/var/VarArray.h>
#include "Molecule.h"
#include "MoleculeTemplate.h"

namespace PLearn {
using namespace std;

class ChemicalICP;
typedef PP<ChemicalICP> ChemICP;

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class ChemicalICP : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    MolTemplate mol_template;
    Mol molecule;

    //! empty TVec means "use all available features"
    //! use "none" if you don't want to use any feature.
    TVec<string> feature_names;
    string weighting_method;
    Var weighting_params;

    // "exhaustive"
    string matching_method;

    real initial_angles_step;
    Mat initial_angles_list;

    // stopping conditions
    int max_iter;
    real error_t;
    real angle_t;
    real trans_t;

    // not options
    VarArray used_properties;
    VarArray other_base_properties;

    // Learned options
    Mat rotation;
    Vec translation;
    TVec<int> matching;
    Vec weights;
    real error;

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    TVec<string> used_feat_names;

    // for caching
    Mat feat_distances2;

    /* Var ? */
    Vec mol_feat_indices;
    Vec template_feat_indices;

    // variables that will be used in SurfaceTemplateLearner,
    // they form used_properties
    Var mol_coordinates;
    Var used_mol_features;
    Var template_coordinates;
    Var template_geom_dev;
    Var used_template_features;
    Var used_template_feat_dev;

    // variables that need to be added to the global parameter array
    // they form other_base_properties
    Var all_mol_features;
    Var all_template_features;
    Var all_template_feat_dev;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    ChemicalICP();

    ChemicalICP( const MolTemplate& the_template,
                 const Mol& the_molecule,
                 const TVec<string>& the_feature_names = TVec<string>(),
                 string the_weighting_method = "sigmoid",
                 const Var& the_weighting_params = Var( Vec(2,1) ) );

    // Your other public member functions go here

    //! Use this function to set the molecule and update every parameter that
    //! depends on it without having to call build().
    virtual void setMolecule( const Mol& the_molecule );

    //! Performs the alignment
    virtual void run();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ChemicalICP);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! computes the intersection of feature_names, molecule->feature_names and
    //! mol_template->feature_names, stores it in used_feat_names, and computes
    //! correspondance index for mol and mol_template
    virtual void computeUsedFeatures();

    //! updates the 'used_...' variables' fields
    virtual void computeVariables();

    virtual void cacheFeatureDistances();

    virtual void matchNearestNeighbors( const Mat& tr_template_coords,
                                        const Mat& matched_mol_coords );

    virtual void minimizeWeightedDistance( const Mat& tr_template_coords,
                                           const Mat& matched_mol_coords,
                                           real& delta_rot_length,
                                           real& delta_trans_length );

    virtual real computeWeightedDistance( const Mat& tr_template_coords,
                                          const Mat& matched_mol_coords );

    virtual void computeWeights( const Mat& tr_template_coords,
                                 const Mat& matched_mol_coords );

    //! Declares the class options.
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
DECLARE_OBJECT_PTR(ChemicalICP);
  
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
