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
 */
class ChemicalICP : public Object
{
    typedef Object inherited;

public:

    //! Indices of the used features, for the molecule and the template.
    //! They are UnaryVariable, so that their parent may be set to a Var that
    //! runs an ICP (in order to make sure they are not used before the ICP is
    //! run).
    Var mol_feat_indices, template_feat_indices;

    //! Variable that has the same content as the 'matching' vector after an
    //! ICP run.
    Var matching_neighbors;

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! The template we try to align on the molecule
    MolTemplate mol_template;

    //! The molecule
    Mol molecule;

    //! Names of features to use during alignment. Empty TVec means "use all
    //! available features" use "none" if you don't want to use any feature.
    TVec<string> feature_names;

    //! Method used to compute the weight of a pair of point. One of:
    //!   - "features_sigmoid": sigmoid of feature distance,
    //!   - "none": same weight for each pair.
    string weighting_method;

    //! Parameters used during weighting. Meaning depends on "weighting_method"
    Var weighting_params;

    //! Method used to find the nearest neighbors. For the moment, only one:
    //!   - "exhaustive": exhaustive search (caching feature distances).
    string matching_method;

    //! Tries initial rotations every "initial_angles_step" degrees
    real initial_angles_step;

    //! Explicit list of initial rotations angles
    Mat initial_angles_list;

    // Stopping conditions
    int max_iter; //!< Maximum number of iterations to perform during alignment
    real error_t; //!< Stop alignment if error falls below this threshold
    real angle_t; //!< Stop alignment if angles falls below this threshold
    real trans_t; //!< Stop alignment if translation falls below this threshold

    // Learned options
    Mat rotation; //!< Learned rotation matrix
    Vec translation; //!< Learned translation vector

    //! matching[i] is the index of the molecule point being
    //! the nearest neighbor of template point i
    TVec<int> matching;

    //! Weight of the pair of points (i, matching[i])
    Vec weights;

    //! Weigted error of the alignment (equal to the 'distance' part of
    //! 'score', as computed in ComputeScoreVariable)
    real error;

    // not options
    VarArray used_properties;
    VarArray other_base_properties;

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here

    //! feature names really used during alignment: intersection between
    //molecule->feature_names, mol_template->feature_names and feature_names
    TVec<string> used_feat_names;

    //! Cache of feature distances : feat_distances2(i,j) is feature distance
    //! between point i of the template and point j of the molecule
    Mat feat_distances2;


public: // for debug
    // variables that need to be added to the global parameter array
    // they form other_base_properties
    Var all_mol_features;
    Var all_template_features;
    Var all_template_feat_dev;

    // variables that will be used in SurfaceTemplateLearner,
    // they form used_properties
    Var mol_coordinates;
    Var used_mol_features;
    Var template_coordinates;
    Var template_geom_dev;
    Var used_template_features;
    Var used_template_feat_dev;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ChemicalICP();

    ChemicalICP( const MolTemplate& the_template,
                 const Mol& the_molecule,
                 const TVec<string>& the_feature_names = TVec<string>(),
                 string the_weighting_method = "features_sigmoid",
                 const Var& the_weighting_params = Var( Vec(2,1) ),
                 string the_matching_method = "exhaustive" );

    //! Use this function to set the molecule and update every parameter that
    //! depends on it without having to call build().
    virtual void setMolecule( const Mol& the_molecule );

    //! Performs the alignment
    virtual void run();

    //! Saves the alignment parameters: rotation, translation, matching...
    //! in a file (in plearn format)
    virtual void saveMatch( const PPath& filename );


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

    //! fills feat_distances2 matrix
    virtual void cacheFeatureDistances();

    //! fills matching, and compute matched_mol_coords as the list of
    //! coordinates of transformed template's nearest neighbors
    virtual void matchNearestNeighbors( const Mat& tr_template_coords,
                                        const Mat& matched_mol_coords );

    //! finds transformation minimizing weighted distance between matched points
    virtual void minimizeWeightedDistance( const Mat& tr_template_coords,
                                           const Mat& matched_mol_coords,
                                           real& delta_rot_length,
                                           real& delta_trans_length );

    //! computes the alignment error: sum over every matched pairs of
    //! (weighted geometrical distance) + feature distance
    virtual real computeWeightedDistance( const Mat& tr_template_coords,
                                          const Mat& matched_mol_coords );

    //! computes weights for every pair of matched points (fills weights)
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
