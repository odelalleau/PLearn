// -*- C++ -*-

// ScoreLayerVariable.h
//
// Copyright (C) 2006 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file ScoreLayerVariable.h */


#ifndef ScoreLayerVariable_INC
#define ScoreLayerVariable_INC

#include "ChemicalICP.h"
#include "Molecule.h"
#include "RunICPVariable.h"
#include <plearn/math/PRandom.h>
#include <plearn/var/NaryVariable.h>

namespace PLearn {
using namespace std;

/*! * ScoreLayerVariable * */

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
class ScoreLayerVariable : public NaryVariable
{
    typedef NaryVariable inherited;

public:

    //! The variable that will run the ICP alignments.
    PP<RunICPVariable> run_icp_var;

    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    PP<ChemicalICP> icp_aligner_template;
    int n_active_templates;
    int n_inactive_templates;
    bool normalize_by_n_features;
    long seed_;
    bool simple_mixture;
    VMat templates_source;

public:

    //#####  Public Member Functions  #########################################

    //! Default constructor.
    ScoreLayerVariable();

    //! Return the molecule template associated with the given (real number)
    //! molecule id and activity.
    PP<MoleculeTemplate> getMoleculeTemplate(real molecule_id,
                                             real activity = -1);

    //! Return the molecule associated to the given (real number) molecule id.
    //! Note that this method is not called from this class: it may not be the
    //! cleanest way to do it, but we need to cache the molecules somewhere to
    //! ensure they do not need to be systematically reloaded.
    PP<Molecule> getMolecule(real molecule_id);

    //! Obtain the mappings of the input variable from a VMatrix.
    void setMappingsSource(const VMat& source_vmat);

    //! Modify scaling coefficient for i-th score to set it to 'coeff'.
    void setScalingCoefficient(int i, real coeff);

    //#####  PLearn::Variable methods #########################################
    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

    // ### These ones are not always implemented
    // virtual void bbprop();
    // virtual void symbolicBprop();
    // virtual void rfprop();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ScoreLayerVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! The VMatrix containing information about the input mappings.
    VMat mappings_source;

    //! Random number generator.
    PP<PRandom> random_gen;

    //! The array of output variables, comprised of two parts:
    //! - first, the scores (ComputeScoreVariable)
    //! - second, the (optional) additional input features
    VarArray outputs;

    //! The concatenation of the variables in 'outputs': this is what will be
    //! actually the value of this variable.
    Var final_output;

    //! All molecule templates currently loaded, identified by their id string.
    hash_map< string, PP<MoleculeTemplate> > molecule_templates;

    //! All molecules currently loaded, identified by their id string.
    hash_map< string, PP<Molecule> > molecules;

    //! Number of active molecules in the templates source.
    int n_active_in_source;

    //! Number of inactive molecules in the templates source.
    int n_inactive_in_source;

    //! The i-th element is a 1x1 Var that contains the normalization
    //! coefficient for the score of template number i.
    //! This normalization coefficient is equal to one over the number of
    //! points in the template, optionally also multiplied by one over the
    //! number of features common with the current aligned molecules (i.e.
    //! 3 + the number of common chemical features); this optional additional
    //! normalization depending on the 'normalize_by_n_features' option.
    VarArray scaling_coeffs;

    //#####  Protected Options  ###############################################

protected:
    //#####  Protected Member Functions  ######################################

    //! Return the actual number of active templates, i.e. n_active_templates
    //! if it is non-negative, and the total number of active molecules in the
    //! templates source otherwise.
    int getNActiveTemplates();

    //! Return the actual number of inactive templates, i.e. n_active_templates
    //! if it is non-negative, and the total number of inactive molecules in the
    //! templates source otherwise.
    int getNInactiveTemplates();

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
DECLARE_OBJECT_PTR(ScoreLayerVariable);

// ### Put here a convenient method for building your variable from other
// ### existing ones, or a VarArray.
// ### e.g., if your class is TotoVariable, with two parameters foo_type foo
// ### and bar_type bar, you could write, depending on your constructor:
// inline Var toto(Var v1, Var v2, Var v3,
//                 foo_type foo=default_foo, bar_type bar=default_bar)
// { return new TotoVariable(v1, v2, v3, foo, bar); }
// ### or:
// inline Var toto(Var v1, Var v2, v3
//                 foo_type foo=default_foo, bar_type bar=default_bar)
// { return new TotoVariable(v1 & v2 & v3, foo, bar); }
// ### or:
// inline Var toto(const VarArray& varray, foo_type foo=default_foo,
//                 bar_type bar=default_bar)
// { return new TotoVariable( varray, foo, bar); }

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
