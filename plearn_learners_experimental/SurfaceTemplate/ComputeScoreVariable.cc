// -*- C++ -*-

// ComputeScoreVariable.cc
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

/*! \file ComputeScoreVariable.cc */


#include "ComputeScoreVariable.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ComputeScoreVariable,
    "Compute an ICP alignment score.",
    ""
);

//////////////////////////
// ComputeScoreVariable //
//////////////////////////
ComputeScoreVariable::ComputeScoreVariable() 
{
    PLERROR("Not implemented");
}

ComputeScoreVariable::ComputeScoreVariable(
    PP<GlobalTemplateParameters> global_params, PP<Molecule> template_mol,
    string the_weighting_method, PP<ChemicalICP> aligner_template = 0,
    PP<Molecule> aligned_mol = 0):

    // inherited(global_params->getAllVarParamters(), 1, 1),
    // TODO Properly set varray.
    template_molecule(template_mol),
    weighting_method(the_weighting_method),
    aligned_molecule(aligned_mol),
    params(global_params)
{
    if (icp_aligner)
        icp_aligner = aligner_template->deep_copy();
    else
        icp_aligner = new ChemicalICP();
    aligned_template_coordinates = new Var(template_molecule->n_points(), 3);
    build_();
}

/*
ComputeScoreVariable::ComputeScoreVariable(Var input_index,
                                           Var geom_mean,
                                           Var geom_dev,
                                           Var feat_mean,
                                           Var feat_dev,
                                           Var weighting_params,
                                           string the_weighting_method,
                                           const ChemICP& the_icp_aligner,
                                           const TVec<Molecule>& the_molecules
                                          )
    : inherited(input_index & geom_mean & geom_dev & feat_mean & feat_dev &
                weighting_params, 1, 1),
      weighting_method(the_weighting_method),
      icp_aligner(the_icp_aligner),
      p_molecules(&the_molecules)
{
    // ### You may (or not) want to call build_() to finish building the
    // ### object
    build_();
}
*/

////////////////////
// recomputeSizes //
////////////////////
void ComputeScoreVariable::recomputeSizes(int& l, int& w) const
{
    l = w = 1;
}

///////////
// fprop //
///////////
void ComputeScoreVariable::fprop()
{
    /*
    // Run the alignment process.
    icp_aligner->run();

    // Compute the new coordinates of the aligned template.
    productTranspose(aligned_template_coordinates->matValue,
                     molecule_template->coordinates, icp_aligner->rotation);
                     */
    this->value << final_score->value;
}

///////////
// bprop //
///////////
void ComputeScoreVariable::bprop()
{
    final_score->gradient << this->gradient;
}

// ### You can implement these methods:
// void ComputeScoreVariable::bbprop() {}
// void ComputeScoreVariable::symbolicBprop() {}
// void ComputeScoreVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void ComputeScoreVariable::build()
{
    inherited::build();
    build_();
}

void ComputeScoreVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ComputeScoreVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void ComputeScoreVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &ComputeScoreVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    /* Probably useless since set in ICP aligner.
    declareOption(ol, "weighting_method",
                  &ComputeScoreVariable::weighting_method,
                  OptionBase::buildoption,
        "The method used to weigh the geometric distances:\n"
        " - none   : no additional weight\n"
        " - sigmoid: the weight is set to sigmoid(w_b * (w_a - dist_feat))\n"
        "            where 'dist_feat' is the feature distance, and the\n"
        "            weights (w_a, w_b) are given in the underlying ICP\n"
        "            'weighting_params' option.");
        */

    /*
    declareOption(ol, "icp_aligner", &ComputeScoreVariable::icp_aligner,
                  OptionBase::buildoption,
                  "");
                  */

    // p_molecules is not an option, it has to be set in the constructor

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ComputeScoreVariable::build_()
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

    // icp_aligner->weighting_method = weighting_method;
    // set icp_aligner's Mats' and Vecs' data to the values of the vars
    setAlignedMolecule(aligned_molecule);

    // Build the graph of Vars.
    // TODO Assume we are given set_molecule_var.
    Var icp_var = new RunICPVariable(icp_aligner, set_molecule_var);
    Var molecule_coord = new VarRowsVariable(total_coords, derniere colonne de
            l'icp_var);
}

////////////////////////
// setAlignedMolecule //
////////////////////////
void ComputeScoreVariable::setAlignedMolecule(PP<Molecule> mol)
{
    aligned_molecule = mol;
    if (aligned_molecule)
        icp_aligner->setMolecule(aligned_molecule);
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
