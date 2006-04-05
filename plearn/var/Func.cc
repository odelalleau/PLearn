// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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


 

/* *******************************************************      
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "Func.h"
#include <plearn/math/random.h>
#include <plearn/math/TMat_maths.h>
#include "Var.h"
#include "Var_operators.h"
#include "TimesConstantVariable.h"
//#include <plearn/display/DisplayUtils.h> ////////// to remove

namespace PLearn {
using namespace std;

/** Func **/

Func::Func()
{}

Func::Func(Function* f) 
    :PP<Function>(f) 
{}

Func::Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
    :PP<Function>(new Function(the_inputs,parameters_to_optimize,the_outputs))
{}
Func::Func(const VarArray& the_inputs, const VarArray& the_outputs)
    :PP<Function>(new Function(the_inputs, the_outputs))
{}

/*void Func::bprop(VarArray& parameters_to_optimize)
  {
  ptr->bprop(parameters_to_optimize);
  }
*/

Vec Func::operator()(const Vec& input) const
{ return ptr->operator()(input); }

real Func::operator()(const Vec& input1, const Vec& input2) const
{ return ptr->operator()(input1, input2); }

VarArray Func::operator()(const VarArray& new_inputs) const
{ return ptr->operator()(new_inputs); }

Func operator/(Func f, real value)
{ 
    if(fast_exact_is_equal(value, 1.0))
        return f;
    else
    {
        int nouts = f->outputs.size();
        VarArray outs(nouts);
        for(int i=0; i<nouts; i++)
            outs[i] = f->outputs[i]/value;
        return Func(f->inputs, outs);
    }
}


/** Function **/

Function::Function()
    :inputsize(-1), outputsize(-1)
{}


Function::Function(const VarArray& the_inputs, const VarArray& the_outputs)
    :inputs(the_inputs), outputs(the_outputs)
{  
    build_();
}

Function::Function(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
    : inputs(the_inputs), parameters(parameters_to_optimize), outputs(the_outputs)
{
    build_();
}

/*void Function::bprop(VarArray& parameters_to_optimize)
  {
  //bproppath = propagationPath(inputs, parameters_to_optimize,outputs);
  }
*/

PLEARN_IMPLEMENT_OBJECT(Function, "Implements a function defined as a var graph", "NO HELP");

void Function::declareOptions(OptionList& ol)
{
    declareOption(ol, "inputs", &Function::inputs, OptionBase::buildoption,
                  "The list of input variabes of this function");
    declareOption(ol, "parameters", &Function::parameters, OptionBase::buildoption,
                  "The list of parameters to optimize");
    declareOption(ol, "outputs", &Function::outputs, OptionBase::buildoption,
                  "The list of output variables of this function");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void Function::build_()
{
    if(parameters.isEmpty())
        parameters = nonInputSources(inputs, outputs);
  
    inputsize = inputs.nelems();
    outputsize = outputs.nelems(); 
  
    fproppath = propagationPath(inputs, outputs);
    if (fproppath.length()==0) // to handle the weird case in which there is no path from inputs to outputs
        // but outputs still depends on parameters and we want to represent that dependency 
    {
        fproppath = propagationPath(inputs & parameters, outputs);
        bproppath = propagationPath(inputs & parameters, outputs);
    }
    else
        bproppath = propagationPath(inputs, outputs);

    parentspath = propagationPathToParentsOfPath(inputs, outputs);

    //parameters_to_optimize.printNames();
    //cout<<"**************Func::printInfo(inputs, outputs);"<<endl;
    //printInfo(inputs, outputs);
    //cout<<"**************Func::printInfo(parameters_to_optimize, outputs);"<<endl;
    //printInfo(parameters_to_optimize,outputs);
    //displayVarGraph(fproppath,true, 333, "ffpp", false);
    //displayVarGraph(bproppath,true, 333, "fbpp", false);
    
    
    // Let's see if getting everything in a single chunk of memory will improve efficiency...
    // Hmm, doesn't seem to.
    /*
      VarArray criticalvars = the_inputs & fproppath;
      int n = criticalvars.nelems();
      Vec data(2*n);
      criticalvars.makeSharedValue(data);
      criticalvars.makeSharedGradient(data,n);
    */
}

void Function::build()
{
    inherited::build();
    build_();
}

void Function::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Object::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(inputs, copies);
    deepCopyField(outputs, copies);
    deepCopyField(fproppath, copies);
    deepCopyField(bproppath, copies);
    deepCopyField(parentspath, copies);
    deepCopyField(df, copies);
    deepCopyField(parameters, copies);
}

void Function::fprop(const Vec& in, const Vec& out) const
{
    inputs << in;
    fproppath.fprop();
    outputs >> out;
}

void Function::fprop(const Array<Vec>& in, const Array<Vec>& out) const
{
    inputs << in;
    fproppath.fprop();
    outputs >> out;
}

void Function::sizefprop(const Vec& in, const Vec& out) const
{
    inputs << in;
    fproppath.sizefprop();
    outputs >> out;
}

void Function::sizefprop(const Array<Vec>& in, const Array<Vec>& out) const
{
    inputs << in;
    fproppath.sizefprop();
    outputs >> out;
}

real Function::operator()(const Vec& input1, const Vec& input2) const
{
    if(inputs.size()!=2 || outputsize!=1)
        PLERROR("You can only call real Function::operator()(const Vec& input1, const Vec& input2) for a function that has 2 input Vars and a single scalar output Var"); 
    inputs[0]->copyFrom(input1);
    inputs[1]->copyFrom(input2);
    fproppath.fprop();
    return outputs[0]->value[0];
}

void Function::fbprop(const Vec& in, const Vec& out, const Vec& input_gradient, const Vec& output_gradient)
{
    inputs << in;
    inputs.clearGradient();
    fproppath.clearGradient();
    outputs.copyGradientFrom(output_gradient);
    fproppath.fbprop();
    outputs >> out;
    inputs.copyGradientTo(input_gradient);

#ifdef BOUNDCHECK
    if (out.hasMissing())
        PLERROR("Function::fbprop: detected MISSING_VALUE in function output!");
  
    //static bool displayvargraph=false;
    //if (displayvargraph)
    //  displayVarGraph(outputs,true);
#endif
}

void Function::fbprop(const Array<Vec>& in, const Array<Vec>& out, const Array<Vec>& input_gradient, const Array<Vec>& output_gradient)
{
    inputs << in;
    inputs.clearGradient();
    fproppath.clearGradient();
    outputs.copyGradientFrom(output_gradient);
    fproppath.fbprop();
    outputs >> out;
    inputs.copyGradientTo(input_gradient);

#ifdef BOUNDCHECK
    if (out.hasMissing())
        PLERROR("Function::fbprop: detected MISSING_VALUE in function output!");
#endif
}

void Function::sizefbprop(const Vec& in, const Vec& out, const Vec& input_gradient, const Vec& output_gradient)
{
    inputs << in;
    inputs.clearGradient();
    fproppath.clearGradient();
    outputs.copyGradientFrom(output_gradient);
    fproppath.sizefbprop();
    outputs >> out;
    inputs.copyGradientTo(input_gradient);

#ifdef BOUNDCHECK
    if (out.hasMissing())
        PLERROR("Function::fbprop: detected MISSING_VALUE in function output!");
  
    //static bool displayvargraph=false;
    //if (displayvargraph)
    //  displayVarGraph(outputs,true);
#endif
}

void Function::sizefbprop(const Array<Vec>& in, const Array<Vec>& out, const Array<Vec>& input_gradient, const Array<Vec>& output_gradient)
{
    inputs << in;
    inputs.clearGradient();
    fproppath.clearGradient();
    outputs.copyGradientFrom(output_gradient);
    fproppath.sizefbprop();
    outputs >> out;
    inputs.copyGradientTo(input_gradient);

#ifdef BOUNDCHECK
    if (out.hasMissing())
        PLERROR("Function::fbprop: detected MISSING_VALUE in function output!");
#endif
}

void Function::fbbprop(const Vec& in, const Vec& output, const Vec& gradient, const Mat& hessian)
{
    if(df==0)
        df = differentiate();

    inputs << in; // inputs and df->inputs are supposed to be the same...
    fproppath.fprop();
    outputs >> output;
    df->fproppath.fprop();
    df->outputs >> gradient;

    df->outputs.clearGradient();
    int pos = 0;
    for(int varnum=0; varnum<df->outputs.size(); varnum++)
    {
        Var& outputvar = df->outputs[varnum];
        for(int i=0; i<outputvar->nelems(); i++)
        {
            df->inputs.clearGradient();
            df->bproppath.clearGradient();
            outputvar->gradient[i] = 1.0;
            df->bproppath.bprop();
            Vec hessian_row = hessian(pos++);
            df->inputs.copyGradientTo(hessian_row);
            outputvar->gradient[i] = 0.0;
        }
    }
}

void Function::fbbpropAcc(const Vec& in, const Vec& output, const Vec& gradient, const Mat& hessian)
{
    if(df==0)
        df = differentiate();

    inputs << in; // inputs and df->inputs are supposed to be the same...
    fproppath.fprop();
    outputs.accumulateTo(output);
    df->fproppath.fprop();
    df->outputs.accumulateTo(gradient);

    df->outputs.clearGradient();
    int pos = 0;
    for(int varnum=0; varnum<df->outputs.size(); varnum++)
    {
        Var& outputvar = df->outputs[varnum];
        for(int i=0; i<outputvar->nelems(); i++)
        {
            df->inputs.clearGradient();
            df->bproppath.clearGradient();
            outputvar->gradient[i] = 1.0;
            df->bproppath.bprop();
            Vec hessian_row = hessian(pos++);
            df->inputs.accumulateGradientTo(hessian_row);
            outputvar->gradient[i] = 0.0;
        }
    }
}

void Function::rfprop(const Vec& in, const Vec& out, const Vec& input_rvalue, const Vec& output_rvalue, bool do_fprop)
{
    if (do_fprop) fprop(in,out);
  
    inputs.copyRValueFrom(input_rvalue);  
    fproppath.rfprop();
    outputs.copyRValueTo(output_rvalue);
}

void Function::recomputeParents()
{ parentspath.fprop(); }

Func Function::differentiate()
{
    if (outputs.size()>1)
        PLERROR("In Function::differentiate cannot differentiate function with more than one output variable");
    Var output = outputs[0];
    if(df==0)
    {
        output->g = Var(1,"output->g"); ////// ultimatley remove???
        output->g = 1.0; // fill gradient
        fproppath.symbolicBprop();
        // Give the symbolic gradient vars reasonable names
        for(int i=0; i<fproppath.size(); i++)
        {
            if(!fproppath[i]->g)
            {
                string name = "gr_" + fproppath[i]->getName();
                fproppath[i]->g->setName(name);
            }
        }
        for(int i=0; i<inputs.size(); i++)
        {
            if(inputs[i]->g.isNull()) // must create it, even though it will remain 0
                inputs[i]->g = Var(inputs[i]->length(), inputs[i]->width());
            string name = "gr_" + inputs[i]->getName();
            inputs[i]->g->setName(name);
        }
        VarArray dinputs = inputs.symbolicGradient();
        // Sanity check:
        if(dinputs.nelems() != inputs.nelems())
            PLERROR("Problem in Function::differentiate() please send a bug report to vincentp@iro.umontreal.ca");

        cerr << "i0: " << inputs[0]->classname() << endl;
        cerr << "i1: " << inputs[1]->classname() << endl;
        cerr << "di0: " << dinputs[0]->classname() << endl;
        cerr << "di1: " << dinputs[1]->classname() << endl;
        dinputs.resizeRValue();
        cerr << "di0 = " << dinputs[0]->rvaluedata << endl;
        df = Func(inputs, dinputs);
        df->fproppath = propagationPath(fproppath.parents() & (VarArray)output->g, dinputs);
        fproppath.clearSymbolicGradient();
    }
    return df;
}

Vec Function::operator()(const Vec& input) const
{ 
    Vec output(outputsize);
    fprop(input,output); 
    return output;
}

// new version that uses the new deepCopy system

VarArray Function::operator()(const VarArray& new_inputs) const
{
    CopiesMap copies;

    // make sure the clones of the old inputs are the new inputs
    for(int i=0; i<inputs.size(); i++)
    {
        if(new_inputs[i]->length()!=inputs[i]->length() || new_inputs[i]->width()!=inputs[i]->width())
            PLERROR("In Function::operator()(const VarArray& new_inputs) dimensions of variables in new_inputs and inputs do not match");
        copies[(Variable*)inputs[i]] = (Variable*)new_inputs[i];
        if (!new_inputs[i]->nameIsSet() && inputs[i]->nameIsSet())
            new_inputs[i]->setName(inputs[i]->getName());
    }

    // make sure that only the vars on the direct path from inputs to outputs
    // get cloned but the clones should have the same parents as the
    // originals so that gradients can be accumulated in these originals and
    // then back propagated to shared sources.
    VarArray parofpath = nonInputParentsOfPath(inputs, outputs);
    for(int i=0; i<parofpath.size(); i++)
        copies[(Variable*)parofpath[i]] = (Variable*)parofpath[i];

    // do the deep copying
    VarArray new_outputs = outputs;
    new_outputs.makeDeepCopyFromShallowCopy(copies);
        
    return new_outputs;
}


// Old Version that uses the old clone system
/*
  VarArray Function::operator()(const VarArray& new_inputs) const
  {
  for(int i=0; i<inputs.size(); i++)
  {
  if(new_inputs[i]->length()!=inputs[i]->length() || new_inputs[i]->width()!=inputs[i]->width())
  PLERROR("In Function::operator()(const VarArray& new_inputs) dimensions of variables in new_inputs and inputs do not match");
  inputs[i]->clone_ = new_inputs[i];
  }

  VarArray clones(fproppath.size());
  for(int i=0; i<fproppath.size(); i++)
  clones[i] = fproppath[i]->clone();

  VarArray new_outputs(outputs.size());
  for(int i=0; i<outputs.size(); i++)
  new_outputs[i] = outputs[i]->clone();

  inputs.clearClone();
  fproppath.clearClone();
  outputs.clearClone();
        
  return new_outputs;
  }
*/

void Function::verifyHessian(const Vec& input, real step)
{
    // Job a Charles...
    // Note: L'utilisation de l'option -DUSEDOUBLE dans le Makefile_option
    // permet d'eviter certains problemes numeriques d'approximation
    // et donc d'utiliser des valeurs de step plus petites
    if(outputsize!=1)
        PLERROR("In Function::verifyHessian(...) Can verify hessian only for output of size 1");
    real out1,out2,out3,out4;
    real doublestep = 2*step;
    Vec output(1);
    Vec gradient(inputsize);
    Mat hessian(inputsize,inputsize);
    fbbprop(input, output, gradient, hessian);
    cerr << "** Verifying hessian computation **" << endl;
    cerr << "Input:                " << input;
    cerr << "Output:               " << output;
    cerr << "Computed  hessian:    " << hessian;    
    // Now computing the gradient by finite difference
    //
    // f(x1+dx1,x2+dx2)-f(x1-dx1,x2+dx2)-f(x1+dx1,x2-dx2)+f(x1-dx1,x2-dx2)
    // ------------------------------------------------------------------
    //                    2 * dx1 * 2 * dx2
    //
    Vec newinput1 = input.copy();
    Vec newinput2 = input.copy();
    Vec newinput3 = input.copy();
    Vec newinput4 = input.copy();
    Mat finitediffhessian(inputsize,inputsize);
    Mat rel(inputsize,inputsize);
    double h,f;
    for(int i=0; i<inputsize; i++)
    {
        for(int j=0; j<inputsize; j++)
        {
            newinput1[i] = newinput1[i]-step;
            newinput1[j] = newinput1[j]-step;
            newinput2[i] = newinput2[i]+step;
            newinput2[j] = newinput2[j]-step;
            newinput3[i] = newinput3[i]-step;
            newinput3[j] = newinput3[j]+step;
            newinput4[i] = newinput4[i]+step;
            newinput4[j] = newinput4[j]+step;
            fprop(newinput1,output);
            out1 = output[0];
            fprop(newinput2,output);
            out2 = output[0];
            fprop(newinput3,output);
            out3 = output[0];
            fprop(newinput4,output);
            out4 = output[0];
            finitediffhessian(i,j) = ((out4-out3)/doublestep-(out2-out1)/doublestep)/doublestep;
            newinput1[i] = input[i];
            newinput1[j] = input[j];
            newinput2[i] = input[i];
            newinput2[j] = input[j];
            newinput3[i] = input[i];
            newinput3[j] = input[j];
            newinput4[i] = input[i];
            newinput4[j] = input[j];
        }
    }  
    cerr << "Estimated hessian:   " << finitediffhessian;
    cerr << "-------------------" << endl;
    for (int i=0; i<inputsize; i++)
    {
        for(int j=0; j<inputsize; j++)
        {
            h = hessian(i,j);
            f = finitediffhessian(i,j);
            rel(i,j) = 2*fabs(h-f)/(fabs(h)+fabs(f));
        }    
    }
    cerr << "relative difference: " << rel << endl;
    cerr << "-------------------" << endl;
    cerr << "max relative difference: " << max(rel) << endl;
}

  

void Function::verifyGradient(const Vec& input, real step)
{
    if(outputsize!=1)
        PLWARNING("In Function::verifyGradient(...) Will verify gradient only for the first output");
    Vec output(outputsize);
    Vec output_gradient(outputsize);
    output_gradient[0]=1.0;
    Vec gradient(inputsize);
    fbprop(input, output, gradient,output_gradient);
    cerr << "** Verifying gradient computation **" << endl;
    cerr << "Input:                " << input << endl;
    cerr << "Output:               " << output[0] << endl;
    cerr << "Computed  gradient:   " << gradient << endl;
    //displayFunction(this,true);
    // Now computing the gradient by finite difference
    Vec newinput = input.copy();
    Vec finitediffgradient(inputsize);
    double doublestep = step+step;
    for(int i=0; i<inputsize; i++)
    {
        real in = input[i];
        newinput[i] = in+step;
        fprop(newinput,output);
        real out1 = output[0];
        newinput[i] = in-step;
        fprop(newinput,output);
        real out2 = output[0];
        finitediffgradient[i] = (out1-out2)/doublestep;
        newinput[i] = input[i] = in;
    }
    // copy the original input into the VarArray
    fprop(newinput,output);
    cerr << "Estimated gradient:   " << finitediffgradient;
    cerr << "-------------------" << endl;
  
    cerr << "relative difference: ";
    // 'Safe' relative difference, that does not display a 'nan' when both
    // computed and estimated gradients are zero.
    Vec num = apply(gradient - finitediffgradient,FABS);
    Vec denom = real(0.5)*apply(gradient + finitediffgradient,FABS);
    for (int i = 0; i < num.length(); i++)
        if (!fast_exact_is_equal(num[i], 0))
            num[i] /= denom[i];
    cerr << num << endl;
    //    apply(gradient - finitediffgradient,(tRealFunc)fabs)/(0.5*apply(gradient + finitediffgradient,(tRealFunc)fabs));
    cerr << "-------------------" << endl;
    cerr << "max relative difference: ";
    // As above, this is a 'safe' relative difference.
    // TODO Question: are we re-doing the same computations as above?
    num = apply(gradient - finitediffgradient,(tRealFunc)FABS);
    denom = real(0.5)*apply(gradient + finitediffgradient,(tRealFunc)FABS);
    for (int i = 0; i < num.length(); i++)
        if (!fast_exact_is_equal(num[i], 0))
            num[i] /= denom[i];
    cerr << max(num) << endl;
    real cos_angle = dot(gradient,finitediffgradient)/(norm(gradient)*norm(finitediffgradient));
    if (cos_angle > 1)
        cos_angle = 1;      // Numerical imprecisions can lead to such situation.
    cerr << "cos(angle) : " << cos_angle << endl;
    cerr << "angle : " << acos(cos_angle) << endl;
}

void Function::verifyGradient(real minval, real maxval, real step)
{
    Vec input(inputsize);
    fill_random_uniform(input,minval, maxval);
    verifyGradient(input, step);
}

void Function::verifyGradient(real step)
{
    Vec input(inputsize);
    inputs >> input;
    verifyGradient(input, step);
}

void Function::verifySymbolicGradient(const Vec& in)
{
    if(in.length()!=inputsize)
        PLERROR("In Function::verifySymbolicGradient(const Vec& in) in does not have the size that this function expects");
    Vec out(outputsize);
    Vec output_gradient(outputsize,1.0);
    Vec gradient1(inputsize);
    fbprop(in,out,gradient1,output_gradient);
    cout << "Bprop computed gradient: " << gradient1 << endl; 
    //cout << "Display f proppath" << endl;
    //displayFunction(this, true, false);
  
    Func df = differentiate();
    //cout << "Display df proppath" << endl;
    Vec gradient2 = df(in);
    //displayFunction(df, true, false);
    cout << "Symbolically computed gradient: " << gradient2 << endl; 
}

void Function::verifyrfprop(const Vec& in, real step)
{
    //This is developed to make sure that the code of rfprop is correct.
  
    Vec gradient(inputsize);
    Vec rfpropRgradient(inputsize);
    Vec fbbRgradient(inputsize);
    Mat hessian(inputsize,inputsize);
    Vec rel(inputsize);
    Vec out(outputsize);
    real b,r;
    
    if(df==0)
        df = differentiate();
  
    fbbprop(in, out, gradient, hessian);
    fbbRgradient = transposeProduct(hessian, gradient); 

    df->inputs.copyRValueFrom(gradient);
    df->fproppath.rfprop();
    df->outputs.copyRValueTo(rfpropRgradient);
  
    for (int i=0; i<inputsize; i++)
    {
        b = fbbRgradient[i];
        r = rfpropRgradient[i];
        if (fast_exact_is_equal(b, 0) && fast_exact_is_equal(r, 0))
            rel[i] = 0.0;
        else rel[i] = fabs(b-r)/(fabs(b)+fabs(r));
    }    
    cerr << "max relative difference of H*g between rfprop and fbbprop: " << max(rel) << endl;
    //cerr << "max & min of rfprop rgradient: " << max(rfpropRgradient) << " " << min(rfpropRgradient) << endl;
    //cerr << "max & min of fbb rgradient: " << max(fbbRgradient) << " " << min(fbbRgradient) << endl;
}

template <>
void deepCopyField(Func& field, CopiesMap& copies)
{
    if (field)
        field = static_cast<Function*>(field->deepCopy(copies));
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
