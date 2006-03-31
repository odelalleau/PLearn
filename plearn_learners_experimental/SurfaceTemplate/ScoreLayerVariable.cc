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
#include "MoleculeTemplate.h"
#include "RunICPVariable.h"
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/RowSumSquareVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SquareRootVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/Var_operators.h>

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
    // weighting_method("none")
{
}

////////////////////
// declareOptions //
////////////////////
void ScoreLayerVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "icp_aligner_template",
                  &ScoreLayerVariable::icp_aligner_template,
                  OptionBase::buildoption,
        "The model of ICP aligner we want to use (will be replicated for\n"
        "each underlying score variable.");
    
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

    /*
    declareOption(ol, "weighting_method",
                  &ScoreLayerVariable::weighting_method,
                  OptionBase::buildoption,
        "See help of ComputeScoreVariable for this option.");
        */

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
    /* TODO Remove?
    // Initialize the global parameters object.
    // TODO This has to be done.
    PP<GlobalTemplateParameters> params = new GlobalTemplateParameters();
    */

    // Create the Var that will run all ICPs.
    PP<RunICPVariable> run_icp_var = new RunICPVariable(varray[0]);

    // This VarArray will list additional parameters that must be optimized.
    VarArray optimized_params;
    
    // Create the corresponding score variables.
    outputs.resize(0);
    int index_in_run_icp_var = 0; // Current index.
    for (int i = 0; i < templates.length(); i++) {
        templates_source->getExample(templates[i], input, target, weight);
        PP<Molecule> mol_template = getMolecule(input[0], target[0]);
        /*
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
        // Create storage variable for the coordinates of the nearest neighbors in
        // the molecule.
        PP<Molecule> mol_template = molecules[canonic_path];
        */
        Var molecule_coordinates(mol_template->n_points(), 3);
        // Create the ICP aligner that will be used for this template.
        CopiesMap copies;
        PP<ChemicalICP> icp_aligner = icp_aligner_template->deepCopy(copies);
        // Declare this new template (with associated molecule coordinates) to
        // the RunICPVariable.
        run_icp_var->addTemplate(icp_aligner, mol_template, molecule_coordinates);
        // Declare the RunICPVariable as parent of the feature indices, in
        // order to ensure these variables are used after ICP has been run.
        PP<UnaryVariable> mol_feat_indices =
            (UnaryVariable*) ((Variable*) icp_aligner->mol_feat_indices);
        mol_feat_indices->setInput((RunICPVariable*) run_icp_var);
        PP<UnaryVariable> template_feat_indices =
            (UnaryVariable*) ((Variable*) icp_aligner->template_feat_indices);
        template_feat_indices->setInput((RunICPVariable*) run_icp_var);
        // Build graph of Variables.
        // (1) Compute the distance in chemical features.
        Var template_features = icp_aligner->used_template_features;
        optimized_params.append(icp_aligner->all_template_features);
        Var molecule_features = icp_aligner->used_mol_features;
        Var diff_features = template_features - molecule_features;
        Var template_features_stddev = icp_aligner->used_template_feat_dev;
        optimized_params.append(icp_aligner->all_template_feat_dev);
        Var feature_distance_at_each_point =
            rowSumSquare(diff_features / template_features_stddev);
        Var total_feature_distance = columnSum(feature_distance_at_each_point);
        // (2) Compute the associated weights for the geometric distance.
        string wm = icp_aligner->weighting_method;
        Var weights;
        if (wm == "none") {
            weights = Var(mol_template->n_points(), 1);
            weights->value.fill(1);
        } else if (wm == "features_sigmoid") {
            Var shift = icp_aligner->weighting_params[0];
            Var slope = icp_aligner->weighting_params[1];
            weights = sigmoid(
                slope * (shift - squareroot(feature_distance_at_each_point)));
            optimized_params.append(shift);
            optimized_params.append(slope);
        } else {
            PLERROR("In ScoreLayerVariable::build_ - Unsupported value for "
                    "'weighting_method'");
        }
        // (3) Compute the distance in geometric coordinates.
        Var template_coordinates =
            PLearn::subMat((RunICPVariable*) run_icp_var,
                           index_in_run_icp_var, 0,
                           mol_template->n_points(), 3);
        index_in_run_icp_var += mol_template->n_points();
        Var diff_coordinates = template_coordinates - molecule_coordinates;
        Var template_coordinates_stddev = icp_aligner->template_geom_dev;
        optimized_params.append(template_coordinates_stddev);
        Var distance_at_each_point =
            rowSumSquare(diff_coordinates / template_coordinates_stddev);
        Var weighted_total_geometric_distance =
            columnSum(distance_at_each_point * weights);
        // (4) Sum to obtain the final score.
        Var total_cost =
            total_feature_distance + weighted_total_geometric_distance;
        // TODO Add the regularization terms...

        /*
        Var score_var =
            new ComputeScoreVariable(params, molecules[canonic_path],
                                     //weighting_method,
                                     icp_aligner_template);
                                     */
        outputs.append(total_cost);
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
    // - the parameters (means, standard deviations, ...)
    // - the final output
    // The final output is not a parameter that will be updated during
    // initialization, but it needs to be in this Variable's parents so that
    // back-propagation is correctly performed.
    varray.append(optimized_params);
    varray.append(final_output);
    // We have changed a parent's option, we should re-build.
    inherited::build();
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

/////////////////
// getMolecule //
/////////////////
PP<Molecule> ScoreLayerVariable::getMolecule(real molecule_id, real activity)
{
    assert( fast_exact_is_equal(activity, 0) ||
            fast_exact_is_equal(activity, 1) ||
            fast_exact_is_equal(activity, -1) );
    assert( mappings_source );
    PPath molecule_path = mappings_source->getValString(0, molecule_id);
    if (molecule_path.isEmpty())
        PLERROR("In ScoreLayerVariable::getMolecule - Could not find "
                "associated mapping");
    string canonic_path = molecule_path.canonical();
    // Load the molecule if necessary.
    Molecule* molecule = 0;
    if (molecules.find(canonic_path) == molecules.end()) {
        molecule =
            new MoleculeTemplate(molecule_path, int(activity));
        molecules[canonic_path] = molecule;
    } else
        molecule = molecules[canonic_path];
    assert( molecule );
    return molecule;
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
