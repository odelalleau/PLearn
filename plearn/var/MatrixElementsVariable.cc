// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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


/* *******************************************************      
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "MatrixElementsVariable.h"

namespace PLearn {
using namespace std;



/** MatrixElementsVariable **/
/* fills the elements of a matrix using the given scalar Variable expression,
   that depends on index variables i and j, that loop from
   0 to ni-1 and 0 to nj-1 respectively. */

PLEARN_IMPLEMENT_OBJECT(MatrixElementsVariable, 
                        "Fills the elements of a matrix using the given scalar Variable "
                        "expression, that depends on index variables i and j, that loop from "
                        "0 to ni-1 and 0 to nj-1 respectively.",
                        "NO HELP");


MatrixElementsVariable::MatrixElementsVariable(Variable* the_expression, const Var& i_index,
                                               const Var& j_index, int number_of_i_values,
                                               int number_of_j_values, const VarArray& the_parameters)
    : inherited(the_parameters, number_of_i_values, number_of_j_values), i(i_index), 
      j(j_index), ni(number_of_i_values), nj(number_of_j_values), expression(the_expression), 
      parameters(the_parameters)
{
    build_();
}

void
MatrixElementsVariable::build()
{
    inherited::build();
}

void
MatrixElementsVariable::build_()
{
    if (i && j && (parameters.size() > 0) && expression) {
        if (!i->isScalar())
            PLERROR("MatrixElementsVariable expect a scalar index Var i_index");
        if (!j->isScalar())
            PLERROR("MatrixElementsVariable expect a scalar index Var j_index");

        full_fproppath = propagationPath(parameters&(VarArray)i&(VarArray)j, expression);
        fproppath = propagationPath(i&j, expression);
        bproppath = propagationPath(parameters, expression);
    }
}

void
MatrixElementsVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "i", &MatrixElementsVariable::i, OptionBase::buildoption, "");
    declareOption(ol, "j", &MatrixElementsVariable::j, OptionBase::buildoption, "");
    declareOption(ol, "ni", &MatrixElementsVariable::ni, OptionBase::buildoption, "");
    declareOption(ol, "nj", &MatrixElementsVariable::nj, OptionBase::buildoption, "");
    declareOption(ol, "expression", &MatrixElementsVariable::expression, OptionBase::buildoption, "");
    declareOption(ol, "parameters", &MatrixElementsVariable::parameters, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void MatrixElementsVariable::recomputeSize(int& l, int& w) const
{ l=ni; w=nj; }


#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void MatrixElementsVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    varDeepCopyField(i, copies);
    varDeepCopyField(j, copies);
    varDeepCopyField(expression, copies);
    deepCopyField(parameters, copies);
    deepCopyField(full_fproppath, copies);
    deepCopyField(fproppath, copies);
    deepCopyField(bproppath, copies);
}


void MatrixElementsVariable::fprop()
{
    int ii,jj;

    for (ii=0;ii<ni;ii++)
    {
        i->value[0]=ii;
        for (jj=0;jj<nj;jj++)
        {
            j->value[0]=jj;
            if (ii==0 && jj==0)
                // effect of parameters is computed only once
                full_fproppath.fprop();
            else
                fproppath.fprop();
            matValue(ii,jj) = expression->value[0];
        }
    }
}


void MatrixElementsVariable::bprop()
{
    int ii,jj;

    for (ii=0;ii<ni;ii++)
    {
        i->value[0]=ii;
        for (jj=0;jj<nj;jj++)
        {
            j->value[0]=jj;
            if (ii==0 && jj==0)
                // effect of parameters is computed only once
                full_fproppath.fprop();
            else
                fproppath.fprop();
            bproppath.clearGradient();
            // PASCAL: peut-etre qu'ici on devrait avoir += ?
            // (si l'expression est une Var utilisee ailleurs!)
            // REPONSE: Non! (expression ne devrait etre utilisee 
            // nulle part ailleurs. To do: utilisation de Func 
            // pour le garantir...)
            expression->gradient[0] = matGradient(ii,jj);
            bproppath.bprop();
        }
    }
}


void MatrixElementsVariable::fbprop()
{
    int ii,jj;

    for (ii=0;ii<ni;ii++)
    {
        i->value[0]=ii;
        for (jj=0;jj<nj;jj++)
        {
            j->value[0]=jj;
            if (ii==0 && jj==0)
                // effect of parameters is computed only once
                full_fproppath.fprop();
            else
                fproppath.fprop();
            matValue(ii,jj) = expression->value[0];
            bproppath.clearGradient();
            // PASCAL: peut-etre qu'ici on devrait avoir += ?
            // (si l'expression est une Var utilisee ailleurs!)
            // REPONSE: Non! (expression ne devrait etre utilisee 
            // nulle part ailleurs. To do: utilisation de Func 
            // pour le garantir...)
            expression->gradient[0] = matGradient(ii,jj);
            bproppath.bprop();
        }
    }
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
