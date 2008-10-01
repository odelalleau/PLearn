// -*- C++ -*-

// RBMTrainer.cc
//
// Copyright (C) 2007 Pascal Lamblin
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

/*! \file RBMTrainer.cc */


#include "RBMTrainer.h"

#include <plearn_learners/online/RBMBinomialLayer.h>
#include <plearn_learners/online/RBMGaussianLayer.h>
#include <plearn_learners/online/RBMMatrixConnection.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/io/openFile.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMTrainer,
    "Trains an RBM",
    "Glop"
    );

RBMTrainer::RBMTrainer():
    n_visible(-1),
    n_hidden(-1),
    visible_type("binomial"),
    update_with_h0_sample(false),
    sample_v1_in_chain(true),
    compute_log_likelihood(true),
    n_stages(1),
    learning_rate(0.01),
    seed(1827),
    n_train(-1),
    n_valid(-1),
    n_test(-1),
    batch_size(1),
    print_debug(false),
    use_fast_approximations(false),
    n_ports(0),
    n_state_ports(0),
    nll_index(-1),
    visible_index(-1),
    rec_err_index(-1)
{
}

// ### Nothing to add here, simply calls build_
void RBMTrainer::build()
{
    inherited::build();
    build_();
}

void RBMTrainer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);
}

void RBMTrainer::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_visible", &RBMTrainer::n_visible,
                  OptionBase::buildoption,
                  "n_visible");

    declareOption(ol, "n_hidden", &RBMTrainer::n_hidden,
                  OptionBase::buildoption,
                  "n_hidden");

    declareOption(ol, "visible_type", &RBMTrainer::visible_type,
                  OptionBase::buildoption,
                  "visible_type");

    declareOption(ol, "update_with_h0_sample",
                  &RBMTrainer::update_with_h0_sample,
                  OptionBase::buildoption,
                  "update_with_h0_sample");

    declareOption(ol, "sample_v1_in_chain", &RBMTrainer::sample_v1_in_chain,
                  OptionBase::buildoption,
                  "sample_v1_in_chain");

    declareOption(ol, "compute_log_likelihood",
                  &RBMTrainer::compute_log_likelihood,
                  OptionBase::buildoption,
                  "compute_log_likelihood");

    declareOption(ol, "n_stages", &RBMTrainer::n_stages,
                  OptionBase::buildoption,
                  "n_stages");

    declareOption(ol, "learning_rate", &RBMTrainer::learning_rate,
                  OptionBase::buildoption,
                  "learning_rate");

    declareOption(ol, "seed", &RBMTrainer::seed,
                  OptionBase::buildoption,
                  "seed");

    declareOption(ol, "n_train", &RBMTrainer::n_train,
                  OptionBase::buildoption,
                  "n_train");

    declareOption(ol, "n_valid", &RBMTrainer::n_valid,
                  OptionBase::buildoption,
                  "n_valid");

    declareOption(ol, "n_test", &RBMTrainer::n_test,
                  OptionBase::buildoption,
                  "n_test");

    declareOption(ol, "batch_size", &RBMTrainer::batch_size,
                  OptionBase::buildoption,
                  "batch_size");

    declareOption(ol, "data_filename", &RBMTrainer::data_filename,
                  OptionBase::buildoption,
                  "data_filename");

    declareOption(ol, "save_path", &RBMTrainer::save_path,
                  OptionBase::buildoption,
                  "save_path");

    declareOption(ol, "save_name", &RBMTrainer::save_name,
                  OptionBase::buildoption,
                  "save_name");

    declareOption(ol, "print_debug", &RBMTrainer::print_debug,
                  OptionBase::buildoption,
                  "print_debug");

    declareOption(ol, "use_fast_approximations",
                  &RBMTrainer::use_fast_approximations,
                  OptionBase::buildoption,
                  "use_fast_approximations");

    declareOption(ol, "data", &RBMTrainer::data,
                  OptionBase::learntoption,
                  "data");

    declareOption(ol, "train_input", &RBMTrainer::train_input,
                  OptionBase::learntoption,
                  "train_input");

    declareOption(ol, "valid_input", &RBMTrainer::valid_input,
                  OptionBase::learntoption,
                  "valid_input");

    declareOption(ol, "test_input", &RBMTrainer::test_input,
                  OptionBase::learntoption,
                  "test_input");

    declareOption(ol, "rbm", &RBMTrainer::rbm,
                  OptionBase::learntoption,
                  "rbm");

    declareOption(ol, "visible", &RBMTrainer::visible,
                  OptionBase::learntoption,
                  "visible");

    declareOption(ol, "hidden", &RBMTrainer::hidden,
                  OptionBase::learntoption,
                  "hidden");

    declareOption(ol, "connection", &RBMTrainer::connection,
                  OptionBase::learntoption,
                  "connection");

    /*
    declareOption(ol, "", &RBMTrainer::,
                  OptionBase::learntoption,
                  "");
    */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMTrainer::declareMethods(RemoteMethodMap& rmm)
{
    // Make sure that inherited methods are declared
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm, "NLL", &RBMTrainer::NLL,
                  (BodyDoc("Computes NLL"),
                   ArgDoc ("examples", "The examples"),
                   RetDoc ("The NLL")
                  ));

    declareMethod(rmm, "recError", &RBMTrainer::recError,
                  (BodyDoc("Computes reconstruction error"),
                   ArgDoc ("examples", "The examples"),
                   RetDoc ("The reconstruction error")
                  ));

    declareMethod(rmm, "CD1", &RBMTrainer::CD1,
                  (BodyDoc("Performs one step of CD"),
                   ArgDoc ("examples", "The examples")));
}

void RBMTrainer::build_()
{
    visible_type = lowerstring(visible_type);
    PLCHECK( visible_type == "binomial" || visible_type == "gaussian" );

    // visible
    if (visible_type == "binomial")
        visible = new RBMBinomialLayer();
    else if (visible_type == "gaussian")
        visible = new RBMGaussianLayer();
    else
        PLERROR("Unknown visible_type (%s).", visible_type.c_str());

    visible->size = n_visible;
    visible->setLearningRate(learning_rate);
    visible->use_fast_approximations = use_fast_approximations;
    visible->build();

    // hidden
    hidden = new RBMBinomialLayer();
    hidden->size = n_hidden;
    hidden->setLearningRate(learning_rate);
    hidden->use_fast_approximations = use_fast_approximations;
    hidden->build();

    // connection
    connection = new RBMMatrixConnection();
    connection->down_size = n_visible;
    connection->up_size = n_hidden;
    connection->setLearningRate(learning_rate);
    connection->use_fast_approximations = use_fast_approximations;
    connection->build();

    // RBM
    rbm = new RBMModule();
    rbm->visible_layer = visible;
    rbm->hidden_layer = hidden;
    rbm->connection = connection;
    rbm->reconstruction_connection = connection;
    rbm->compute_log_likelihood = compute_log_likelihood;
    rbm->random_gen = new PRandom(seed);
    rbm->cd_learning_rate = learning_rate;
    rbm->use_fast_approximations = use_fast_approximations;
    rbm->build();

    // data
    if (!data_filename.isEmpty())
    {
        PP<AutoVMatrix> data_ = new AutoVMatrix();
        data_->filename = data_filename;
        data_->defineSizes(n_visible, 1);
        data_->build();
        data = get_pointer(data_);

        // train_input
        train_input = new SubVMatrix(data,      // source
                                     0,         // istart
                                     0,         // jstart
                                     n_train,   // length
                                     n_visible, // width
                                     true       // call_build
                                     );

        // valid_input
        valid_input = new SubVMatrix(data,
                                     n_train,
                                     0,
                                     n_valid,
                                     n_visible,
                                     true
                                     );

        // valid_input
        test_input = new SubVMatrix(data,
                                    n_train + n_valid,
                                    0,
                                    n_test,
                                    n_visible,
                                    true
                                    );
    }

    // ports
    ports = rbm->getPorts();
    n_ports = ports.length();
    for (int i=0; i<n_ports; i++)
    {
        if (ports[i].find(".state", 0) != string::npos)
        {
            state_ports.append(ports[i]);
            state_ports_indices.append(i);
        }
    }

    n_state_ports = state_ports.length();
    nll_index = rbm->getPortIndex("neg_log_likelihood");
    visible_index = rbm->getPortIndex("visible");
    rec_err_index = rbm->getPortIndex("reconstruction_error.state");

    // nll_values
    nll_values.resize(n_ports);
    for (int i=0; i<n_state_ports; i++)
        nll_values[state_ports_indices[i]] = new Mat();

    // rec_err_values
    rec_err_values.resize(n_ports);
    for (int i=0; i<n_state_ports; i++)
        rec_err_values[state_ports_indices[i]] = new Mat();
}

Mat RBMTrainer::NLL(const Mat& examples)
{
    Mat nll;

    for (int i=0; i<n_state_ports; i++)
        nll_values[state_ports_indices[i]]->resize(0,0);

    nll_values[nll_index] = &nll;
    nll_values[visible_index] = const_cast<Mat *>(&examples);

    rbm->fprop(nll_values);

    if (print_debug)
    {
        pout << "In NLL:" << endl;
        for (int i=0; i<n_state_ports; i++)
        {
            int portnum = state_ports_indices[i];
            string portname = rbm->getPortName(portnum);
            pout << portname << ": " << nll_values[portnum]->size() << endl;
        }
    }

    PLASSERT(nll.length() == examples.length() && nll.width() == 1);
    return nll;
}

Mat RBMTrainer::recError(const Mat& examples)
{
    Mat rec_err;
    for (int i=0; i<n_state_ports; i++)
        rec_err_values[state_ports_indices[i]]->resize(0,0);

    rec_err_values[rec_err_index] = &rec_err;
    rec_err_values[visible_index] = const_cast<Mat *>(&examples);

    rbm->fprop(rec_err_values);

    if (print_debug)
    {
        pout << "In recError:" << endl;
        for (int i=0; i<n_state_ports; i++)
        {
            int portnum = state_ports_indices[i];
            string portname = rbm->getPortName(portnum);
            pout << portname << ": " << rec_err_values[portnum]->size() << endl;
        }
    }

    PLASSERT(rec_err.length() == examples.length()
             && rec_err.width() == 1);
    return rec_err;
}

void RBMTrainer::CD1(const Mat& examples)
{
    int n_examples = examples.length();
    if (print_debug)
    {
        pout << "v0 = " << endl << examples << endl;
    }

    // Positive phase
    connection->setAsDownInputs(examples);
    hidden->getAllActivations(connection, 0, true);
    hidden->computeExpectations();
    hidden->generateSamples();

    h0_a = hidden->activations;
    h0_s.resize(n_examples, n_hidden);
    h0_s << hidden->samples.copy();

    if (update_with_h0_sample)
    {
        h0 = h0_s;
    }
    else
    {
        h0_e.resize(n_examples, n_hidden);
        h0_e << hidden->getExpectations();
        h0 = h0_e;
    }

    if (print_debug)
    {
        pout << "h0_a = " << endl << h0_a << endl;
        pout << "h0_e = " << endl << hidden->getExpectations() << endl;
        pout << "h0_s = " << endl << h0_s << endl;
    }

    // Downward pass
    connection->setAsUpInputs(h0_s);
    visible->getAllActivations(connection, 0, true);
    visible->computeExpectations();
    visible->generateSamples();

    v1_a = visible->activations;
    v1_e = visible->getExpectations();
    v1_s = visible->samples;

    // Negative phase
    if (sample_v1_in_chain)
        v1 = v1_s;
    else
        v1 = v1_e;

    if (print_debug)
    {
        pout << "v1_a = " << endl << v1_a << endl;
        pout << "v1_e = " << endl << v1_e << endl;
        pout << "v1_s = " << endl << v1_s << endl;
    }

    connection->setAsDownInputs(v1);
    hidden->getAllActivations(connection, 0, true);
    hidden->computeExpectations();

    Mat h1 = hidden->getExpectations();
    if (print_debug)
        pout << "h1 = " << endl << h1 << endl;

    rbm->CDUpdate(examples, h0, v1, h1);
}

void RBMTrainer::run()
{
    Mat results(n_stages+1, 6);
    for (int stage=0; stage<n_stages; stage++)
    {
        pout << "stage: " << stage << endl;
        results(stage, 0) = mean(NLL(train_input));
        results(stage, 3) = mean(recError(train_input));

        if (n_valid > 0)
        {
            results(stage, 1) = mean(NLL(valid_input));
            results(stage, 4) = mean(recError(valid_input));
        }

        if (n_test > 0)
        {
            results(stage, 2) = mean(NLL(test_input));
            results(stage, 5) = mean(recError(test_input));
        }

        pout << "NLL:      " << results(stage).subVec(0,3) << endl;
        pout << "RecError: " << results(stage).subVec(3,3) << endl;

        for (int i=0; i<n_train/batch_size; i++)
            CD1(train_input.subMatRows(i*batch_size, batch_size));
    }
    pout << "stage: " << n_stages << endl;
    results(n_stages, 0) = mean(NLL(train_input));
    results(n_stages, 3) = mean(recError(train_input));
    if (n_valid > 0)
    {
        results(n_stages, 1) = mean(NLL(valid_input));
        results(n_stages, 4) = mean(recError(valid_input));
    }
    if (n_test > 0)
    {
        results(n_stages, 2) = mean(NLL(test_input));
        results(n_stages, 5) = mean(recError(test_input));
    }
    pout << "NLL:      " << results(n_stages).subVec(0,3) << endl;
    pout << "RecError: " << results(n_stages).subVec(3,3) << endl;
    pout << "results = " << endl << results << endl;

    if (!save_path.isEmpty())
    {
        PPath filename;
        if (save_name.isEmpty())
            filename = save_path / "results.amat";
        else
            filename = save_path / save_name;

        PStream file = openFile(filename, PStream::raw_ascii, "w");
        file << results << endl;
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
