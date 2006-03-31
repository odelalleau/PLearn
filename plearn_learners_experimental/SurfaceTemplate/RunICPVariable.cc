// -*- C++ -*-

// RunICPVariable.cc
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

/*! \file RunICPVariable.cc */


#include "RunICPVariable.h"
#include "ScoreLayerVariable.h"

namespace PLearn {
using namespace std;

/** RunICPVariable **/

PLEARN_IMPLEMENT_OBJECT(
    RunICPVariable,
    "Runs multiple ICPs and outputs the transformed template coordinates.",
    ""
);

////////////////////
// RunICPVariable //
////////////////////
RunICPVariable::RunICPVariable()
{
}

RunICPVariable::RunICPVariable(Variable* input):
    inherited(input, 0, 3)
{}

/////////////////
// addTemplate //
/////////////////
void RunICPVariable::addTemplate(PP<ChemicalICP> icp_aligner,
                                 PP<Molecule> mol_template,
                                 Var mol_coordinates)
{
    icp_aligners.append(icp_aligner);
    templates.append(mol_template);
    molecule_coordinates.append(mol_coordinates);
    this->sizeprop(); // Resize this variable.
}

// constructor from input variable and parameters
// RunICPVariable::RunICPVariable(Variable* input, param_type the_parameter,...)
// ### replace with actual parameters
//  : inherited(input, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
//{
//    // ### You may (or not) want to call build_() to finish building the
//    // ### object
//}

////////////////////
// recomputeSizes //
////////////////////
void RunICPVariable::recomputeSizes(int& l, int& w) const
{
    l = 0;
    for (int i = 0; i < templates.length(); i++)
        l += templates[i]->n_points();
    w = 3;
}

///////////
// fprop //
///////////
void RunICPVariable::fprop()
{
    // We obtain the molecule from the input variable.
    PP<Molecule> molecule = score_layer->getMolecule(input->value[0]);
    // Launch all ICPs.
    int index_coord = 0;
    for (int i = 0; i < icp_aligners.length(); i++) {
        // Run ICP.
        PP<ChemicalICP> icp = icp_aligners[i];
        PP<Molecule> template_mol = templates[i];
        icp->setMolecule(molecule);
        icp->run();
        // Compute the aligned coordinates of the template and store them in
        // this variable.
        Mat template_coords =
            this->matValue.subMat(index_coord, 0, template_mol->n_points(), 3);
        index_coord += template_mol->n_points();
        productTranspose(template_coords, template_mol->coordinates,
                         icp->rotation);
        template_coords += icp->translation;
        // Obtain the coordinates of the nearest neighbors in the aligned
        // molecule.
        Mat molecule_coords = molecule_coordinates[i]->matValue;
        assert( molecule_coords.length() == template_mol->n_points() );
        for (int k = 0; k < template_mol->n_points(); k++) {
            int neighbor = icp->matching[k];
            molecule_coords(k) << molecule->coordinates(neighbor);
        }
    }
}

///////////
// bprop //
///////////
void RunICPVariable::bprop()
{
    // We do not backprop anything in this Variable.
}

// ### You can implement these methods:
// void RunICPVariable::bbprop() {}
// void RunICPVariable::symbolicBprop() {}
// void RunICPVariable::rfprop() {}


///////////
// build //
///////////
void RunICPVariable::build()
{
    inherited::build();
    build_();
}

void RunICPVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
    PLERROR("RunICPVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void RunICPVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &RunICPVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RunICPVariable::build_()
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
