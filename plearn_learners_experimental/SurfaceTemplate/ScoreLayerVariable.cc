// -*- C++ -*-

// ScoreLayerVariable.cc
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

/*! \file ScoreLayerVariable.cc */


#include "ScoreLayerVariable.h"
#include "ComputeScoreVariable.h"
#include "GlobalTemplateParameters.h"
#include "MoleculeTemplate.h"
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/SubMatVariable.h>

namespace PLearn {
using namespace std;

/** ScoreLayerVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ScoreLayerVariable,
    "First layer (alignment scores) of a SurfaceTemplateLearner.",
    "In addition to the alignment scores, this variable also replicates\n"
    "additional input features, if provided in the source input variable\n"
    "(as indicated by the 'templates_source' VMat option).\n"
);

////////////////////////
// ScoreLayerVariable //
////////////////////////
ScoreLayerVariable::ScoreLayerVariable():
    random_gen(new PRandom())
{
}

////////////////////
// declareOptions //
////////////////////
void ScoreLayerVariable::declareOptions(OptionList& ol)
{
    
    declareOption(ol, "n_active_templates",
                      &ScoreLayerVariable::n_active_templates,
                      OptionBase::buildoption,
        "Number of templates of active molecules.");

    declareOption(ol, "n_inactive_templates",
                      &ScoreLayerVariable::n_inactive_templates,
                      OptionBase::buildoption,
        "Number of templates of inactive molecules.");

    declareOption(ol, "seed", &ScoreLayerVariable::seed_,
                              OptionBase::buildoption,
        "Seed of the random number generator (similar to a PLearner's seed).");

    declareOption(ol, "templates_source",
                      &ScoreLayerVariable::templates_source,
                      OptionBase::buildoption,
        "The VMat templates are obtained from. This VMat's first column must\n"
        "be the name of a molecule, there may be other input features, and\n"
        "there must be a binary target indicating whether a molecule is\n"
        "active or inactive.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// bprop //
///////////
void ScoreLayerVariable::bprop()
{
    // The gradient is back-propagated only on the score variables, since the
    // other variables are inputs that do not need be updated.
    int n = n_active_templates + n_inactive_templates;
    assert( n <= length() );
    real* copy_grad_ptr = final_output->gradientdata;
    real* grad_ptr = gradientdata;
    for (int i = 0; i < n; i++, copy_grad_ptr++, grad_ptr++)
        *copy_grad_ptr += *grad_ptr;
}

///////////
// build //
///////////
void ScoreLayerVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ScoreLayerVariable::build_()
{
    if (seed_ != 0)
        random_gen->manual_seed(seed_);

    if (!templates_source)
        return;

    // Obtain mappings from the templates VMat.
    setMappingsSource(templates_source);
    assert( templates_source->targetsize() == 1 );

    // Verify that we have been given the input variable, and resize the input
    // array that is going to be constructed.
    assert( varray.length() >= 1 );
    varray.resize(1);

    // Randomly select active and inactive templates.
    TVec<int> list_of_active, list_of_inactive;
    int n = templates_source->length();
    Vec input, target;
    real weight;
    for (int i = 0; i < n; i++) {
        templates_source->getExample(i, input, target, weight);
        assert( fast_exact_is_equal(target[0], 0) ||
                fast_exact_is_equal(target[0], 1) );
        if (fast_exact_is_equal(target[0], 0))
            list_of_inactive.append(i);
        else
            list_of_active.append(i);
    }
    random_gen->shuffleElements(list_of_active);
    random_gen->shuffleElements(list_of_inactive);
    assert( list_of_active.length() >= n_active_templates );
    assert( list_of_inactive.length() >= n_inactive_templates );
    list_of_active.resize(n_active_templates);
    list_of_inactive.resize(n_inactive_templates);
    TVec<int>& templates = list_of_active; // Renaming to avoid confusion.
    templates.append(list_of_inactive);
    // Initialize the global parameters object.
    // TODO This has to be done.
    PP<GlobalTemplateParameters> params = new GlobalTemplateParameters();
    // Create the corresponding score variables.
    outputs.resize(0);
    for (int i = 0; i < templates.length(); i++) {
        templates_source->getExample(templates[i], input, target, weight);
        PPath molecule_path = mappings_source->getValString(0, input[0]);
        if (molecule_path.isEmpty())
            PLERROR("In ScoreLayerVariable::build_ - Could not find associated"
                    " mapping");
        string canonic_path = molecule_path.canonical();
        // Load the molecule if necessary.
        if (molecules.find(canonic_path) == molecules.end()) {
            Molecule* new_molecule =
                new MoleculeTemplate(molecule_path, int(target[0]));
            molecules[canonic_path] = new_molecule;
        }
        assert( molecules.find(canonic_path) != molecules.end() );
        Var score_var =
            new ComputeScoreVariable(params, molecules[canonic_path]);
        outputs.append(score_var);
    }
    // Append the additional input features if they are present.
    if (templates_source->inputsize() > 1) {
        Var input_var = varray[0];
        assert( input_var->width() == 1 );
        Var input_minus_molecule_id =
            PLearn::subMat(input_var, 1, 0, input_var->length() - 1, 1);
        outputs.append(input_minus_molecule_id);
    }
    final_output = vconcat(outputs);
    // The final 'varray' will contain, in this order:
    // - the input variable to this layer (already here)
    // - the parameters (means and standard deviations)
    // - the final output
    // The final output is not a parameter that will be updated during
    // initialization, but it needs to be in this Variable's parents so that
    // back-propagation is correctly performed.
    varray.append(params->mean_geom);
    varray.append(params->mean_feat);
    varray.append(params->stddev_geom);
    varray.append(params->stddev_feat);
    varray.append(final_output);
}

///////////
// fprop //
///////////
void ScoreLayerVariable::fprop()
{
    // Just need to copy the data from 'final_output'.
    int n = nelems();
    assert( n == length() );
    real* copy_ptr = final_output->valuedata;
    real* ptr = valuedata;
    for (int i = 0; i < n; i++, ptr++, copy_ptr++)
        *ptr = *copy_ptr;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ScoreLayerVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
    PLERROR("ScoreLayerVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// recomputeSizes //
////////////////////
void ScoreLayerVariable::recomputeSizes(int& l, int& w) const
{
    l = final_output.length();
    w = final_output.width();
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
