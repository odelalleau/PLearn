// -*- C++ -*-

// DynamicallyLinkedRBMsModel.cc
//
// Copyright (C) 2006 Stanislas Lauly
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

/*! \file DynamicallyLinkedRBMsModel.cc */


#define PL_LOG_MODULE_NAME "DynamicallyLinkedRBMsModel"
#include <plearn/io/pl_log.h>

#include "DynamicallyLinkedRBMsModel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DynamicallyLinkedRBMsModel,
    "Model made of RBMs linked through time",
    ""
    );

DynamicallyLinkedRBMsModel::DynamicallyLinkedRBMsModel() :
    rbm_learning_rate( 0.01 ),
    dynamic_learning_rate( 0.01 ),
    fine_tuning_learning_rate( 0.01 ),
    rbm_nstages( 1 ),
    dynamic_nstages( 1 ),
    visible_size( -1 )
{
    random_gen = new PRandom();
}

void DynamicallyLinkedRBMsModel::declareOptions(OptionList& ol)
{
    declareOption(ol, "rbm_learning_rate", &DynamicallyLinkedRBMsModel::rbm_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during RBM contrastive "
                  "divergence learning phase");

    declareOption(ol, "dynamic_learning_rate", &DynamicallyLinkedRBMsModel::dynamic_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the dynamic links "
                  "learning phase");

    declareOption(ol, "fine_tuning_learning_rate", &DynamicallyLinkedRBMsModel::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning "
                  "phase");

    declareOption(ol, "rbm_nstages", &DynamicallyLinkedRBMsModel::rbm_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for rbm phase");

    declareOption(ol, "dynamic_nstages", &DynamicallyLinkedRBMsModel::dynamic_nstages,
                  OptionBase::buildoption,
                  "Number of epochs for dynamic phase");

    declareOption(ol, "visible_layer", &DynamicallyLinkedRBMsModel::visible_layer,
                  OptionBase::buildoption,
                  "The visible layer of the RBMs");

    declareOption(ol, "hidden_layer", &DynamicallyLinkedRBMsModel::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the RBMs. Its size must be set, and will\n"
                  "correspond to the RBMs hidden layer size.");

    declareOption(ol, "connections", &DynamicallyLinkedRBMsModel::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the RBM "
                  "visible and hidden layers");

    declareOption(ol, "dynamic_connections", &DynamicallyLinkedRBMsModel::dynamic_connections,
                  OptionBase::buildoption,
                  "OnlineLearningModule corresponding to dynamic links "
                  "between RBMs' hidden layers");

    /*
    declareOption(ol, "", &DynamicallyLinkedRBMsModel::,
                  OptionBase::learntoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DynamicallyLinkedRBMsModel::build_()
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

    MODULE_LOG << "build_() called" << endl;

    if(train_set)
    {
        visible_size = 0;
        symbol_sizes.resize(0);
        PP<Dictionary> dict;        
        for(int i=0; i<train_set->inputsize(); i++)
        {
            dict = train_set->getDictionary(i);
            if(dict)
            {
                if( dict->size() == 0 )
                    PLERROR("DynamicallyLinkedRBMsModel::build_(): dictionary "
                        "of field %d is empty", i);
                symbol_sizes.push_back(dict->size());
                // Adjust size to include one-hot vector
                visible_size += dict->size();
            }
            else
            {
                symbol_sizes.push_back(-1);
                visible_size++;
            }
        }

        // Set and verify sizes
        if(hidden_layer->size <= 0)
            PLERROR("DynamicallyLinkedRBMsModel::build_(): hidden_layer->size "
                "must be > 0");

        pos_down_values.resize(visible_size);
        pos_up_values.resize(hidden_layer->size);
        hidden_layer_target.resize(hidden_layer->size);
        hidden_layer_sample.resize(hidden_layer->size);

        visible_layer->size = visible_size;

        connections->down_size = visible_size;
        connections->up_size = hidden_layer->size;
        
        dynamic_connections->input_size = hidden_layer->size;
        dynamic_connections->output_size = hidden_layer->size;

        // Set random generators
        visible_layer->random_gen = random_gen;
        hidden_layer->random_gen = random_gen;
        connections->random_gen = random_gen;
        dynamic_connections->random_gen = random_gen;

        // Build components
        visible_layer->build();
        hidden_layer->build();
        connections->build();
        dynamic_connections->build();
    }

}

// ### Nothing to add here, simply calls build_
void DynamicallyLinkedRBMsModel::build()
{
    inherited::build();
    build_();
}


void DynamicallyLinkedRBMsModel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(visible_layer, copies);
    // deepCopyField(, copies);

    PLERROR("DynamicallyLinkedRBMsModel::makeDeepCopyFromShallowCopy(): "
        "not implemented yet");
}


int DynamicallyLinkedRBMsModel::outputsize() const
{
    int out_size = 1; // Not really...
    return out_size;
}

void DynamicallyLinkedRBMsModel::forget()
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
    
    visible_layer->forget();
    hidden_layer->forget();
    connections->forget();
    dynamic_connections->forget();

    stage = 0;
}

void DynamicallyLinkedRBMsModel::train()
{
    MODULE_LOG << "train() called " << endl;

    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

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

    if(visible_size < 0)
        PLERROR("DynamicallyLinkedRBMsModel::train(): \n"
                "build() must be called before train()");                

    Vec input( inputsize() );
    Vec target( targetsize() );// Unused
    real weight = 0; // Unused
    Vec train_costs( getTrainCostNames().length() );

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    ProgressBar* pb = 0;

    // clear stats of previous epoch
    train_stats->forget();

    /***** RBM training phase *****/
    if(stage < rbm_nstages)
    {
        MODULE_LOG << "Training connection weights between RBMs" << endl;

        int init_stage = stage;
        int end_stage = min( rbm_nstages, nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  rbm_learning_rate = " << rbm_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "RBM training phase of "+classname(),
                                  end_stage - init_stage );

        visible_layer->setLearningRate( rbm_learning_rate );
        connections->setLearningRate( rbm_learning_rate );
        hidden_layer->setLearningRate( rbm_learning_rate );
        real mean_cost = 0;
        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                train_set->getExample(sample, input, target, weight);

                //cout << "lalalalala" << input << endl;
                //exit(0);
                if(train_set->getString(sample,0) == "<oov>")
                    continue;

                clamp_visible_units(input);
                
                mean_cost += rbm_update();
            }

            if( pb )
                pb->update( stage + 1 - init_stage);
            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }

    /***** dynamic phase training  *****/

    if(stage < rbm_nstages +  dynamic_nstages)
    {
        MODULE_LOG << "Training dynamic connections between RBMs' hidden layers" << endl;

        int init_stage = stage;
        int end_stage = min( rbm_nstages + dynamic_nstages, nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  dynamic_learning_rate = " << dynamic_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Dynamic training phase of "+classname(),
                                  end_stage - init_stage );

        previous_hidden_layer.resize(hidden_layer->size);
        dynamic_connections->setLearningRate( dynamic_learning_rate );
        real mean_cost = 0;
        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                if(sample > 0)
                {
                    previous_hidden_layer << hidden_layer_sample;
                    // ** or **
                    // hidden_layer->generateSample();
                    // previous_hidden_layer << hidden_layer->sample;
                }
                else
                    previous_hidden_layer.clear();

                train_set->getExample(sample, input, target, weight);

                if(train_set->getString(sample,0) == "<oov>")
                {
                    hidden_layer_sample.clear();
                    continue;
                }
            
                clamp_visible_units(input);
                
                mean_cost += dynamic_connections_update();
                                
            }
            if( pb )
                pb->update( stage + 1 - init_stage);

            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }

    /***** fine-tuning *****/
    if( stage >= nstages )
        return;

    if(stage >= rbm_nstages +  dynamic_nstages )
    {
        MODULE_LOG << "Training the whole model" << endl;

        int init_stage = stage;
        //int end_stage = max(0,nstages-(rbm_nstages + dynamic_nstages));
        int end_stage = nstages;

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  fine_tuning_learning_rate = " << fine_tuning_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Fine-tuning training phase of "+classname(),
                                  end_stage - init_stage );

        previous_hidden_layer.resize(hidden_layer->size);
        dynamic_connections->setLearningRate( fine_tuning_learning_rate );
        visible_layer->setLearningRate( fine_tuning_learning_rate );
        connections->setLearningRate( fine_tuning_learning_rate );

        real mean_cost = 0;
        while(stage < end_stage)
        {
            for(int sample=0 ; sample<train_set->length() ; sample++ )
            {
                if(sample > 0)
                {
                    previous_hidden_layer << hidden_layer_sample;
                    // ** or **
                    // hidden_layer->generateSample();
                    // previous_hidden_layer << hidden_layer->sample;
                }
                else
                    previous_hidden_layer.clear();

                train_set->getExample(sample, input, target, weight);

                if(train_set->getString(sample,0) == "<oov>")
                {
                    hidden_layer_sample.clear();
                    continue;
                }
                
                clamp_visible_units(input);
                
                mean_cost += fine_tuning_update();                                
            }

            if( pb )
                pb->update( stage + 1 - init_stage);

            if(verbosity>0)
                cout << "mean cost at stage " << stage << 
                    " = " << mean_cost/train_set->length() << endl;
            mean_cost = 0;
            stage++;
        }    
        if( pb )
        {
            delete pb;
            pb = 0;
        }

    }

    train_stats->finalize();
}


void DynamicallyLinkedRBMsModel::clamp_visible_units(const Vec& input) const
{
    int it = 0;
    int ss = -1;
    for(int i=0; i<inputsize_; i++)
    {
        ss = symbol_sizes[i];
        // If input is a real ...
        if(ss < 0) 
        {
            //cout << "yoyoyoyoyo" << endl;
            visible_layer->expectation[it++] = input[i];
        }
        else // ... or a symbol
        {
            // Convert to one-hot vector
            visible_layer->expectation.subVec(it,ss).clear();
            visible_layer->expectation[it+(int)input[i]] = 1;
            it += ss;
        }
    }
}

real DynamicallyLinkedRBMsModel::rbm_update()
{

    //###### Positive phase #####################

    //up phase
    connections->setAsDownInput( visible_layer->expectation );
    
    hidden_layer->getAllActivations( connections );

    hidden_layer->computeExpectation();


    //save the stats for the positive phase
    pos_down_values << visible_layer->expectation;
    pos_up_values << hidden_layer->expectation;



    //down phase
    hidden_layer->generateSample();
    
    connections->setAsUpInput( hidden_layer->sample );

    visible_layer->getAllActivations( connections );

    visible_layer->computeExpectation();
    
    visible_layer->generateSample();




    //############ Negative phase  ##################

    connections->setAsDownInput( visible_layer->sample );
    
    hidden_layer->getAllActivations( connections );

    hidden_layer->computeExpectation();

    hidden_layer->generateSample();

    //############ CD update #########################

    visible_layer->update( 
        pos_down_values, visible_layer->sample ); // ... of visible_layer bias ...

    hidden_layer->update( 
        pos_up_values, hidden_layer->expectation );// ... of hidden_layer bias ...
    
    connections->update( 
        pos_down_values, pos_up_values, visible_layer->sample
        , hidden_layer->sample ); // ... of connections between layers.
    
    // Compute reconstruction error
    
    connections->setAsUpInput( pos_up_values );

    visible_layer->getAllActivations( connections );

    visible_layer->computeExpectation();
    
    return visible_layer->fpropNLL(pos_down_values);
   
}

real DynamicallyLinkedRBMsModel::dynamic_connections_update()
{
    // Obtain target hidden_layer h_t
    connections->setAsDownInput(visible_layer->expectation);
    hidden_layer->getAllActivations(connections);
    hidden_layer->computeExpectation();
    hidden_layer_target << hidden_layer->expectation;
    hidden_layer->generateSample();
    hidden_layer_sample << hidden_layer->sample;
    
    // Use "previous_hidden_layer" field and "dynamic_connections" module 
    // to set bias of "hidden_layer"

    dynamic_connections->fprop(previous_hidden_layer,hidden_layer->activation);
    // hidden_layer->activation *= alpha;
    hidden_layer->expectation_is_up_to_date = false;
    hidden_layer->computeExpectation();

    // Ask "hidden_layer" for maximum likelyhood gradient on bias
    real nll = hidden_layer->fpropNLL(hidden_layer_target);
    hidden_layer->bpropNLL(hidden_layer_target, nll, bias_gradient);

    // bpropUpdate through dynamic_connections
    // real delta = 0;
    // for(int i=0; i<hidden_layer->size; i++)
    //    delta -= dynamic_learning_rate * bias_gradient[i] * hidden_layer->activation[i]/alpha;
    // bias_gradient *= alpha;
    // alpha += delta;
    dynamic_connections->bpropUpdate(previous_hidden_layer,
                                     hidden_layer->activation,
                                     input_gradient, bias_gradient);

    return nll;
}

real DynamicallyLinkedRBMsModel::fine_tuning_update()
{

 // Obtain target hidden_layer h_t
//    connections->setAsDownInput(visible_layer->expectation);
//    hidden_layer->getAllActivations(connections);
//    hidden_layer->computeExpectation();
//    hidden_layer_target << hidden_layer->expectation;
//    hidden_layer->generateSample();
//    hidden_layer_sample << hidden_layer->sample;
    
    // Use "previous_hidden_layer" field and "dynamic_connections" module 
    // to set bias of "hidden_layer"


    // cond_bias.clear();

     

    cond_bias.resize(hidden_layer->size);
   
    dynamic_connections->fprop(previous_hidden_layer, cond_bias);

//     MODULE_LOG << cond_bias->data[0] << endl;
//    cout << (&(real *)(cond_bias->data))[0] << endl;
   
  

    hidden_layer->getAllBias(cond_bias);
    // hidden_layer->expectation_is_up_to_date = false;
    //hidden_layer->computeExpectation();


    // cout <<  hidden_layer->bias << endl;



 //###### Positive phase #####################

    //up phase
    connections->setAsDownInput( visible_layer->expectation );
    hidden_layer->getAllActivations( connections );
    hidden_layer->computeExpectation();


    //save the stats for the positive phase
    pos_down_values << visible_layer->expectation;
    pos_up_values << hidden_layer->expectation;



    //down phase
    hidden_layer->generateSample();
    hidden_layer_sample << hidden_layer->sample;
    connections->setAsUpInput( hidden_layer->sample );

    visible_layer->getAllActivations( connections );

    visible_layer->computeExpectation();
    
    visible_layer->generateSample();




    //############ Negative phase  ##################

    connections->setAsDownInput( visible_layer->sample );
    
    hidden_layer->getAllActivations( connections );

    hidden_layer->computeExpectation();

    hidden_layer->generateSample();

    


    //cout <<  hidden_layer->bias << endl;
     //############ CD update #########################

    visible_layer->update( 
        pos_down_values, visible_layer->sample ); // ... of visible_layer bias ...

    


// cout << pos_up_values << endl;
// cout << "blabla" << endl;
// cout << hidden_layer->sample << endl;
   
    hidden_layer->bpropCD(pos_up_values, hidden_layer->expectation, bias_gradient);


    dynamic_connections->bpropUpdate(previous_hidden_layer,
                                     cond_bias,
                                     input_gradient, bias_gradient);

   
    // hidden_layer->update( 
    //    pos_up_values, hidden_layer->sample );// ... of hidden_layer bias ...
    
    connections->update( 
        pos_down_values, pos_up_values, visible_layer->sample
        , hidden_layer->sample ); // ... of connections between layers.

    //cout << ((PLearn::PP<PLearn::GradNNetLayerModule>)dynamic_connections)->weights << endl;








    // Use "previous_hidden_layer" field and "dynamic_connections" module 
    // to set bias of "hidden_layer"

    // Positive phase

    // Negative phase

    // CD update ...

    // .. ask "hidden_layer" for CD gradient on bias and ...

    // ... bpropUpdate through dynamic_connections 
    // to update conditional bias (*)
    
    // ... of visible_layer bias ...

    // ... of connections between layers.

    // NB.: it is important not to update the hidden_layer's bias
    //      using usual CD update. Conditional bias was updated
    //      before at (*)





    // Compute reconstruction error

    //cond_bias.clear();
    //dynamic_connections->fprop(previous_hidden_layer, cond_bias);
    //hidden_layer->getAllBias(cond_bias);

    



    connections->setAsUpInput( pos_up_values );

    visible_layer->getAllActivations( connections );

    visible_layer->computeExpectation();
    
    return visible_layer->fpropNLL(pos_down_values);

    


}

void DynamicallyLinkedRBMsModel::computeOutput(const Vec& input, Vec& output) const
{
    PLERROR("DynamicallyLinkedRBMsModel::computeOutput(): this is a dynamic, "
            "generative model, that can only compute negative log-likelihood "
            "costs for a whole VMat");
}

void DynamicallyLinkedRBMsModel::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    PLERROR("DynamicallyLinkedRBMsModel::computeCostsFromOutputs(): this is a "
            "dynamic, generative model, that can only compute negative "
            "log-likelihooh costs for a whole VMat");
}

void DynamicallyLinkedRBMsModel::test(VMat testset, PP<VecStatsCollector> test_stats,
                  VMat testoutputs, VMat testcosts)const
{
   

    int len = testset.length();
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());
    Vec costs(nTestCosts());

    PP<ProgressBar> pb;
    if (report_progress) 
        pb = new ProgressBar("Testing learner", len);

    if (len == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }


    int begin = 0;
    int nb_oov = 0;
    for (int i = 0; i < len; i++)
    {
        testset.getExample(i, input, target, weight);
      
       


        if(testset->getString(i,0) == "<oov>")
        {
            begin = 0;
            nb_oov++;
            continue;
        }
        




        clamp_visible_units(input);




        if(begin > 0)
        {

            //h*_{t-1}
            //////////////////////////////////
            dynamic_connections->fprop(previous_hidden_layer, cond_bias);
           
            hidden_layer->getAllBias(cond_bias); //**************************


            //up phase
            connections->setAsDownInput( previous_input );
            hidden_layer->getAllActivations( connections );
            hidden_layer->computeExpectation();
            //////////////////////////////////

            previous_hidden_layer << hidden_layer->expectation;//h_{t-2} au prochain tour//******************************

            //h*_{t}
            ////////////
            dynamic_connections->fprop( hidden_layer->expectation ,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            hidden_layer->expectation_is_up_to_date = false;
            hidden_layer->computeExpectation();//h_{t}
            ///////////

            previous_input << visible_layer->expectation;//v_{t-1}
            
        }
        else
        {
            previous_hidden_layer.clear();//h_{t-1}
            dynamic_connections->fprop(previous_hidden_layer,hidden_layer->activation);//conection entre h_{t-1} et h_{t}
            hidden_layer->expectation_is_up_to_date = false;
            hidden_layer->computeExpectation();//h_{t}

            previous_input.resize(visible_layer->size);
            previous_input << visible_layer->expectation;
            begin++;
        }


       






     



        connections->setAsUpInput( hidden_layer->expectation );

        visible_layer->getAllActivations( connections );

        visible_layer->computeExpectation();

       

        costs[0] = visible_layer->fpropNLL(previous_input) / inputsize() ;




        // costs[0] = 0; //nll/nb_de_temps_par_mesure

        if (testoutputs)
            testoutputs->putOrAppendRow(i, output);

        if (testcosts)
            testcosts->putOrAppendRow(i, costs);

        if (test_stats)
            test_stats->update(costs, weight);

        if (report_progress)
            pb->update(i);
    }

    //costs[0] = costs[0]/(len - nb_oov) ;

    //cout << "Probabilite moyenne : " << costs[0] << endl;

}


TVec<string> DynamicallyLinkedRBMsModel::getTestCostNames() const
{
    TVec<string> cost_names;
    cost_names.append( "NLL" );
    return cost_names;
}

TVec<string> DynamicallyLinkedRBMsModel::getTrainCostNames() const
{
    return getTestCostNames();
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
