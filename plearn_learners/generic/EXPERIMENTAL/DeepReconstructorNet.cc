// -*- C++ -*-

// DeepReconstructorNet.cc
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
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

// Authors: Simon Lemieux, Pascal Vincent

/*! \file DeepReconstructorNet.cc */


#include "DeepReconstructorNet.h"
#include <plearn/display/DisplayUtils.h> 
#include <plearn/var/Var_operators.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
   DeepReconstructorNet,
   "ONE LINE DESCRIPTION",
   "MULTI-LINE \nHELP");

DeepReconstructorNet::DeepReconstructorNet()
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
}

void DeepReconstructorNet::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    
    declareOption(ol, "training_schedule", &DeepReconstructorNet::training_schedule,
                  OptionBase::buildoption,
                  "training_schedule[k] conatins the number of epochs for the training of the hidden layer taking layer k as input (k=0 corresponds to input layer).");

    declareOption(ol, "layers", &DeepReconstructorNet::layers,
                  OptionBase::buildoption,
                  "layers[0] is the input variable ; last layer is final output layer");

    declareOption(ol, "reconstruction_costs", &DeepReconstructorNet::reconstruction_costs,
                  OptionBase::buildoption,
                  "recontruction_costs[k] is the reconstruction cost for layer[k]");

    declareOption(ol, "reconstruction_optimizer", &DeepReconstructorNet::reconstruction_optimizer,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "target", &DeepReconstructorNet::target,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "supervised_cost", &DeepReconstructorNet::supervised_cost,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "minibatch_size", &DeepReconstructorNet::minibatch_size,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "supervised_optimizer", &DeepReconstructorNet::supervised_optimizer,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "fine_tuning_optimizer", &DeepReconstructorNet::fine_tuning_optimizer,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "good_improvement_rate", &DeepReconstructorNet::good_improvement_rate,
                  OptionBase::buildoption,
                  "");

    
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepReconstructorNet::declareMethods(RemoteMethodMap& rmm)
{
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm,
                  "getParameterValue",
                  &DeepReconstructorNet::getParameterValue,
                  (BodyDoc("Returns the matValue of the parameter variable with the given name"),
                   ArgDoc("varname", "name of the variable searched for"),
                   RetDoc("Returns the value of the parameter as a Mat")));


    declareMethod(rmm,
                  "listParameterNames",
                  &DeepReconstructorNet::listParameterNames,
                  (BodyDoc("Returns a list of the names of the parameters"),
                   RetDoc("Returns a list of the names of the parameters")));

    declareMethod(rmm,
                  "listParameter",
                  &DeepReconstructorNet::listParameter,
                  (BodyDoc("Returns a list of the parameters"),
                   RetDoc("Returns a list of the names")));
}

void DeepReconstructorNet::build_()
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
    
    int nlayers = layers.length();
    compute_layer.resize(nlayers-1);
    for(int k=0; k<nlayers-1; k++)
        compute_layer[k] = Func(layers[k], layers[k+1]);
    compute_output = Func(layers[0], layers[nlayers-1]);
    nout = layers[nlayers-1]->size();

    if(supervised_cost.isNull())
        PLERROR("You must provide a supervised_cost");
    fullcost = supervised_cost;
    int n_rec_costs = reconstruction_costs.length();
    for(int k=0; k<n_rec_costs; k++)
        fullcost = fullcost + reconstruction_costs[k];
    //displayVarGraph(fullcost);
    Var input = layers[0];
    Func f(input&target, fullcost);
    parameters = f->parameters;
    outmat.resize(n_rec_costs);
}

// ### Nothing to add here, simply calls build_
void DeepReconstructorNet::build()
{
    if(random_gen.isNull())
        random_gen = new PRandom();
    inherited::build();
    build_();
}

void DeepReconstructorNet::initializeParams(bool set_seed)
{
    perr << "Initializing parameters..." << endl;
    if (set_seed && seed_ != 0)
        random_gen->manual_seed(seed_);

    for(int i=0; i<parameters.length(); i++)
        dynamic_cast<SourceVariable*>((Variable*)parameters[i])->randomInitialize(random_gen);
}


void DeepReconstructorNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DeepReconstructorNet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int DeepReconstructorNet::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).

    //TODO : retourner la bonne chose ici
    return 0;
}

void DeepReconstructorNet::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */    
    inherited::forget();
    initializeParams();    
}

void DeepReconstructorNet::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    PPath outmatfname = expdir/"outmat";

    int nreconstructions = reconstruction_costs.length();
    int insize = train_set->inputsize();
    VMat dset = train_set.subMatColumns(0,insize);

    // hack...
    dset = dset.subMatRows(0,10);
    for(int k=0; k<nreconstructions; k++)
    {
        trainHiddenLayer(k, dset);
        int width = layers[k+1].width();
        outmat[k] = new FileVMatrix(outmatfname+tostring(k)+".pmat",0,width);
        outmat[k]->defineSizes(width,0);
        buildHiddenLayerOutputs(k, dset, outmat[k]);
        dset = outmat[k];
    }    

    /*
    while(stage<nstages)
    {        
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */
}

void DeepReconstructorNet::buildHiddenLayerOutputs(int which_input_layer, VMat inputs, VMat outputs)
{
    int l = inputs.length();
    Vec in;
    Vec target;
    real weight;
    Func f = compute_layer[which_input_layer];
    Vec out(f->outputsize);
    for(int i=0; i<l; i++)
    {
        inputs->getExample(i,in,target,weight);
        f->fprop(in, out);
        outputs->putOrAppendRow(i,out);
    }
}

void DeepReconstructorNet::trainHiddenLayer(int which_input_layer, VMat inputs)
{
    int l = inputs->length();
    int nepochs = training_schedule[which_input_layer];
    perr << "\n\n*********************************************" << endl;
    perr << "*** Training layer " << which_input_layer << " for " << nepochs << " epochs " << endl;
    perr << "*** each epoch has " << l << " examples and " << l/minibatch_size << " optimizer stages (updates)" << endl;
    Func f(layers[which_input_layer], reconstruction_costs[which_input_layer]);
    //displayVarGraph(reconstruction_costs[which_input_layer]);
    // displayVarGraph(fproppath,true, 333, "ffpp", false);
    Var totalcost = sumOf(inputs, f, minibatch_size);
    VarArray params = totalcost->parents();
    reconstruction_optimizer->setToOptimize(params, totalcost);
    reconstruction_optimizer->reset();
    VecStatsCollector st;
    real prev_mean = -1;
    real relative_improvement = good_improvement_rate;
    for(int n=0; n<nepochs && relative_improvement >= good_improvement_rate; n++)
    {
        st.forget();
        reconstruction_optimizer->nstages = l/minibatch_size;
        reconstruction_optimizer->optimizeN(st);
        const StatsCollector& s = st.getStats(0);
        real m = s.mean();
        perr << "Epoch " << n << " mean error: " << m << " +- " << s.stderror() << endl;
        if(prev_mean>0)
        {
            relative_improvement = ((prev_mean-m)/prev_mean)*100;
            perr << "Relative improvement: " << relative_improvement << " %"<< endl;
        }
        prev_mean = m;
    }
}


void DeepReconstructorNet::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(nout);
    compute_output->fprop(input, output);
}

void DeepReconstructorNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output.
// ...
}

TVec<string> DeepReconstructorNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ...
    
    //TODO : retourner la bonne chose ici !
    TVec<string> todo(0);
    return todo;
}

TVec<string> DeepReconstructorNet::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    // ...

    //TODO : retourner la bonne chose ici !
    TVec<string> todo(0);
    return todo;
}

Mat DeepReconstructorNet::getParameterValue(const string& varname)
{
    for(int i=0; i<parameters.length(); i++)
        if(parameters[i]->getName() == varname)
            return parameters[i]->matValue;
    PLERROR("There is no parameter  named %s", varname.c_str());
    return Mat(0,0);
}

TVec<string> DeepReconstructorNet::listParameterNames()
{
    TVec<string> nameListe(0);
    for (int i=0; i<parameters.length(); i++)
        if (parameters[i]->getName() != "")
            nameListe.append(parameters[i]->getName());
    return nameListe;
}

TVec<Mat> DeepReconstructorNet::listParameter()
{
    TVec<Mat> matList(0);
    for (int i=0; i<parameters.length(); i++)
        matList.append(parameters[i]->matValue);
    return matList;
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
