// -*- C++ -*-

// ComputeScoreVariable.h
//
// Copyright (C) 2006 Pascal Lamblin and Olivier Delalleau
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

// Authors: Pascal Lamblin and Olivier Delalleau

/*! \file ComputeScoreVariable.h */


#ifndef ComputeScoreVariable_INC
#define ComputeScoreVariable_INC

#include <plearn/var/NaryVariable.h>
#include "ChemicalICP.h"
#include "GlobalTemplateParameters.h"
#include "Molecule.h"

namespace PLearn {
using namespace std;

/*! * ComputeScoreVariable * */

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
class ComputeScoreVariable : public NaryVariable
{
    typedef NaryVariable inherited;

public:
    
    //! The molecule that is being aligned.
    PP<Molecule> aligned_molecule;
    
    //! The reference template.
    PP<Molecule> template_molecule;
    
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    // string weighting_method;

    
    /* its parameters should point to the matValue()'s of
                            parent variables from this */

    /*
    // pointer to the 'molecules' field of parent learner
    // to access element i, use (*p_molecule)[i]
    const TVec< Molecule >*  p_molecules;
    */

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor, usually does nothing
    ComputeScoreVariable();

    /*
    //! Constructor initializing from input variables
    // ### Make sure the implementation in the .cc calls inherited constructor
    // ### and initializes all fields with reasonable default values.
    ComputeScoreVariable(Var input_index,
                         Var geom_mean, Var geom_dev,
                         Var feat_mean, Var feat_dev,
                         Var weighting_params,
                         string the_weighting_method = "none",
                         const ChemICP& the_icp_aligner = new ChemicalICP(),
                         const TVec<Molecule>& the_molecules = TVec<Molecule>()
                         );
                         */

    ComputeScoreVariable(PP<GlobalTemplateParameters> global_params,
                         PP<Molecule> template_mol,
                         // string weighting_method = "none",
                         PP<ChemicalICP> aligner_template = 0,
                         PP<Molecule> aligned_mol = 0);

    // ### If your parent variables are a meaning and you want to be able to
    // ### access them easily, you can add functions like:
    // Var& First() { return varray[0]; }
    // Var& Second() { return varray[1]; }
    // ...

    /*
    Var& input_index() { return varray[0]; }
    Var& geom_mean() { return varray[1]; }
    Var& feat_mean() { return varray[2]; }
    Var& geom_sigma() { return varray[3]; }
    Var& feat_sigma() { return varray[4]; }
    Var& weighting_params() { return varray[5]; }
    */

    // Your other public member functions go here

    //! Set the current aligned molecule.
    void setAlignedMolecule(PP<Molecule> mol);

    //#####  PLearn::Variable methods #########################################
    virtual void recomputeSizes(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

    // ### These ones are not always implemented
    // virtual void bbprop();
    // virtual void symbolicBprop();
    // virtual void rfprop();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT 
    PLEARN_DECLARE_OBJECT(ComputeScoreVariable);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! The ICP aligner used to compute the alignment score.
    PP<ChemicalICP> icp_aligner;

    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:

    //! Contains the final score, computed in a variable.
    Var final_score;

    //! A (n x 3) Variable containing the aligned template coordinates after
    //! a chemical ICP run.
    Var aligned_template_coordinates;
    
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private: 
    //#####  Private Member Functions  ########################################

    //! This does the actual building. 
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ComputeScoreVariable);

/*
// ### Put here a convenient method for building your variable from other
// ### existing ones, or a VarArray.
inline Var computeScore(Var input_index, Var geom_mean, Var geom_dev,
                        Var feat_mean, Var feat_dev, Var weighting_params,
                        string the_weighting_method = "none",
                        const ChemICP& the_icp_aligner = new ChemicalICP(),
                        const TVec<Molecule>& the_molecules = TVec<Molecule>())
{ return new ComputeScoreVariable(input_index,
                                  geom_mean, geom_dev, feat_mean, feat_dev,
                                  weighting_params, the_weighting_method,
                                  the_icp_aligner, the_molecules); }
                                  */

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
