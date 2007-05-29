// -*- C++ -*-

// ModuleTester.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

/*! \file ModuleTester.cc */


#include "ModuleTester.h"

#define PL_LOG_MODULE_NAME "ModuleTester"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ModuleTester,
    "Test an OnlineLearningModule to ensure it is properly implemented.",
    ""
);

ModuleTester::ModuleTester():
    seeds(TVec<long>(1, long(1827))),
    default_length(10),
    default_width(5),
    max_in(1),
    max_out_grad(MISSING_VALUE),
    min_in(0),
    min_out_grad(MISSING_VALUE),
    step(1e-6),
    absolute_tolerance_threshold(1),
    absolute_tolerance(1e-5),
    relative_tolerance(1e-5)
{}

void ModuleTester::build()
{
    inherited::build();
    build_();
}

void ModuleTester::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ModuleTester::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void ModuleTester::declareOptions(OptionList& ol)
{
    declareOption(ol, "module", &ModuleTester::module,
                  OptionBase::buildoption,
        "The module to be tested.");

    declareOption(ol, "configurations", &ModuleTester::configurations,
                  OptionBase::buildoption,
        "List of port configurations to test. Each element is a map from a\n"
        "string to a list of corresponding ports. This string can be one of:\n"
        " - 'in_grad': input ports for which a gradient must be computed\n"
        " - 'in_nograd': input ports for which no gradient is computed\n"
        " - 'out_grad': output ports for which a gradient must be provided\n"
        " - 'out_nograd': output ports for which no gradient is provided");

    declareOption(ol, "min_in", &ModuleTester::min_in,
                  OptionBase::buildoption,
        "Minimum value used when uniformly sampling input data.");

    declareOption(ol, "max_in", &ModuleTester::max_in,
                  OptionBase::buildoption,
        "Maximum value used when uniformly sampling input data.");

    declareOption(ol, "min_out_grad", &ModuleTester::min_out_grad,
                  OptionBase::buildoption,
        "Minimum value used when uniformly sampling output gradient data.\n"
        "If missing, then 'min_in' is used.");

    declareOption(ol, "max_out_grad", &ModuleTester::max_out_grad,
                  OptionBase::buildoption,
        "Maximum value used when uniformly sampling output gradient data.\n"
        "If missing, then 'max_in' is used.");

    declareOption(ol, "seeds", &ModuleTester::seeds,
                  OptionBase::buildoption,
        "Seeds used for random number generation. You can try different seeds "
        "if you want to test more situations.");

    declareOption(ol, "default_length", &ModuleTester::default_length,
                  OptionBase::buildoption,
        "Default length of a port used when the module returns an undefined "
        "port length (-1 in getPortLength())");

    declareOption(ol, "default_width", &ModuleTester::default_width,
                  OptionBase::buildoption,
        "Default width of a port used when the module returns an undefined "
        "port width (-1 in getPortWidth())");

    declareOption(ol, "step", &ModuleTester::step,
                  OptionBase::buildoption,
        "Small offset used to modify inputs in order to estimate the\n"
        "gradient by finite difference.");

    declareOption(ol, "absolute_tolerance_threshold",
                  &ModuleTester::absolute_tolerance_threshold,
                  OptionBase::buildoption,
        "Value below which we use absolute tolerance instead of relative in\n"
        "order to compare gradients.");

    declareOption(ol, "absolute_tolerance",
                  &ModuleTester::absolute_tolerance,
                  OptionBase::buildoption,
        "Absolute tolerance when comparing gradients.");

    declareOption(ol, "relative_tolerance",
                  &ModuleTester::relative_tolerance,
                  OptionBase::buildoption,
        "Relative tolerance when comparing gradients.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


////////////
// build_ //
////////////
void ModuleTester::build_()
{
    if (!module)
        return;

    PP<PRandom> random_gen = new PRandom();
    TVec<Mat*> fprop_data(module->nPorts()); // Input to 'fprop'.
    TVec<Mat*> bprop_data(module->nPorts()); // Input to 'bpropAccUpdate'.
    // We also use additional matrices to store gradients in order to ensure
    // the module is properly accumulating.
    TVec<Mat*> bprop_check(module->nPorts());
    // Store previous fprop result in order to be able to estimate gradient.
    TVec<Mat*> fprop_check(module->nPorts());
    // Initialize workspace for matrices. Note that we need to allocate enough
    // memory from start, as otherwise an append may make previous Mat*
    // pointers invalid.
    int max_mats_size = 1000;
    TVec<Mat> mats(max_mats_size);

    bool ok = true;
    for (int j = 0; ok && j < seeds.length(); j++) {
        random_gen->manual_seed(seeds[j]);
        for (int i = 0; ok && i < configurations.length(); i++) {
            map<string, TVec<string> >& conf = configurations[i];
            const TVec<string>& in_grad = conf["in_grad"];
            const TVec<string>& in_nograd = conf["in_nograd"];
            const TVec<string>& out_grad = conf["out_grad"];
            const TVec<string>& out_nograd = conf["out_nograd"];
            TVec<string> all_in(in_grad.length() + in_nograd.length());
            all_in.subVec(0, in_grad.length()) << in_grad;
            all_in.subVec(in_grad.length(), in_nograd.length()) << in_nograd;
            TVec<string> all_out(out_grad.length() + out_nograd.length());
            all_out.subVec(0, out_grad.length()) << out_grad;
            all_out.subVec(out_grad.length(), out_nograd.length()) << out_nograd;
            mats.resize(0);
            // Prepare fprop data.
            fprop_data.fill(NULL);
            fprop_check.fill(NULL);
            for (int k = 0; k < all_in.length(); k++) {
                const string& port = all_in[k];
                int length = module->getPortLength(port);
                int width = module->getPortWidth(port);
                if (length < 0)
                    length = default_length;
                if (width < 0)
                    width = default_width;
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                Mat* in_k = & mats.lastElement();
                fprop_data[module->getPortIndex(port)] = in_k;
                // Fill 'in_k' randomly.
                in_k->resize(length, width);
                random_gen->fill_random_uniform(*in_k, min_in, max_in);
            }
            for (int k = 0; k < all_out.length(); k++) {
                const string& port = all_out[k];
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                Mat* out_k = & mats.lastElement();
                int idx = module->getPortIndex(port);
                fprop_data[idx] = out_k;
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                fprop_check[idx] = & mats.lastElement();
            }
            // Perform fprop.
            module->forget();
            module->fprop(fprop_data);
            // Debug output.
            string output;
            PStream out_s = openString(output, PStream::plearn_ascii, "w");
            for (int k = 0; k < fprop_data.length(); k++) {
                out_s.setMode(PStream::raw_ascii);
                out_s << "FPROP(" + module->getPortName(k) + "):\n";
                Mat* m = fprop_data[k];
                if (!m) {
                    out_s << "null";
                } else {
                    out_s.setMode(PStream::plearn_ascii);
                    out_s << *m;
                }
            }
            out_s << endl;
            out_s = NULL;
            DBG_MODULE_LOG << output;
            // Prepare bprop data.
            bprop_data.fill(NULL);
            bprop_check.fill(NULL);
            for (int k = 0; k < in_grad.length(); k++) {
                const string& port = in_grad[k];
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                Mat* in_grad_k = & mats.lastElement();
                int idx = module->getPortIndex(port);
                Mat* in_k = fprop_data[idx];
                // We fill 'in_grad_k' with random elements to check proper
                // accumulation.
                in_grad_k->resize(in_k->length(), in_k->width());
                random_gen->fill_random_uniform(*in_grad_k, -1, 1);
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                // Do a copy of initial gradient to allow comparison later.
                Mat* in_check_k = & mats.lastElement();
                in_check_k->resize(in_grad_k->length(), in_grad_k->width());
                *in_check_k << *in_grad_k;
                in_grad_k->resize(0, in_grad_k->width());
                bprop_data[idx] = in_grad_k;
                bprop_check[idx] = in_check_k;
            }
            for (int k = 0; k < out_grad.length(); k++) {
                const string& port = out_grad[k];
                mats.append(Mat());
                PLCHECK( mats.length() <= max_mats_size );
                Mat* out_grad_k = & mats.lastElement();
                int idx = module->getPortIndex(port);
                Mat* out_k = fprop_data[idx];
                out_grad_k->resize(out_k->length(), out_k->width());
                real min = is_missing(min_out_grad) ? min_in : min_out_grad;
                real max = is_missing(max_out_grad) ? max_in : max_out_grad;
                if (fast_exact_is_equal(min, max))
                    // Special cast to handle in particular the case when we
                    // want the gradient to be exactly 1 (for instance for a
                    // cost).
                    out_grad_k->fill(min);
                else
                    random_gen->fill_random_uniform(*out_grad_k, min, max);
                bprop_data[idx] = out_grad_k;
            }
            // Perform bprop.
            module->bpropAccUpdate(fprop_data, bprop_data);
            // Debug output.
            out_s = openString(output, PStream::plearn_ascii, "w");
            for (int k = 0; k < bprop_data.length(); k++) {
                out_s.setMode(PStream::raw_ascii);
                out_s << "BPROP(" + module->getPortName(k) + "):\n";
                Mat* m = bprop_data[k];
                if (!m) {
                    out_s << "  *** NULL ***\n";
                } else {
                    out_s.setMode(PStream::plearn_ascii);
                    out_s << *m;
                }
            }
            out_s << endl;
            out_s = NULL;
            DBG_MODULE_LOG << output;
            // Check the gradient was properly accumulated.
            // First compute the difference between computed gradient and the
            // initial value stored in the gradient matrix.
            for (int k = 0; k < in_grad.length(); k++) {
                int idx = module->getPortIndex(in_grad[k]);
                Mat* grad = bprop_data[idx];
                if (grad) {
                    Mat* grad_check = bprop_check[idx];
                    PLASSERT( grad_check );
                    *grad_check -= *grad;
                    negateElements(*grad_check);
                }
            }
            // Then perform a new bprop pass, without accumulating.
            for (int k = 0; k < in_grad.length(); k++) {
                int idx = module->getPortIndex(in_grad[k]);
                bprop_data[idx]->resize(0, bprop_data[idx]->width());
            }
            module->forget(); // Ensure we are using same parameters.
            module->bpropUpdate(fprop_data, bprop_data);
            // Then compare 'bprop_data' and 'bprop_check'.
            for (int k = 0; k < in_grad.length(); k++) {
                int idx = module->getPortIndex(in_grad[k]);
                Mat* grad = bprop_data[idx];
                PLASSERT( grad );
                Mat* check = bprop_check[idx];
                PLASSERT( check );
                // TODO Using the PLearn diff mechanism would be better.
                for (int p = 0; p < grad->length(); p++)
                    for (int q = 0; q < grad->width(); q++)
                        if (!is_equal((*grad)(p,q), (*check)(p,q))) {
                            pout << "Gradient for port '" <<
                                module->getPortName(k) << "' was not " <<
                                "properly accumulated: " << (*grad)(p,q) <<
                                " != " << (*check)(p,q) << endl;
                            ok = false;
                        }
            }
            // Continue only if accumulation test passed.
            if (!ok)
                return;
            // Verify gradient is coherent with the input, through a subtle
            // perturbation of said input.
            // Save result of fprop.
            for (int k = 0; k < all_out.length(); k++) {
                int idx = module->getPortIndex(all_out[k]);
                Mat* val = fprop_data[idx];
                Mat* check = fprop_check[idx];
                PLASSERT( val && check );
                check->resize(val->length(), val->width());
                *check << *val;
            }
            for (int k = 0; ok && k < in_grad.length(); k++) {
                int idx = module->getPortIndex(in_grad[k]);
                Mat* grad = bprop_data[idx];
                Mat* val = fprop_data[idx];
                Mat* b_check = bprop_check[idx];
                PLASSERT( grad && val && b_check );
                grad->fill(0); // Will be used to store estimated gradient.
                for (int p = 0; p < grad->length(); p++)
                    for (int q = 0; q < grad->width(); q++) {
                        real backup = (*val)(p, q);
                        (*val)(p, q) += step;
                        for (int r = 0; r < all_out.length(); r++) {
                            int to_clear = module->getPortIndex(all_out[r]);
                            PLASSERT( to_clear != idx );
                            fprop_data[to_clear]->resize(0, 0);
                        }
                        module->forget();
                        module->fprop(fprop_data);
                        (*val)(p, q) = backup;
                        // Estimate gradient w.r.t. each output.
                        for (int r = 0; r < out_grad.length(); r++) {
                            int out_idx = module->getPortIndex(out_grad[r]);
                            Mat* out_val = fprop_data[out_idx];
                            Mat* out_prev = fprop_check[out_idx];
                            Mat* out_grad = bprop_data[out_idx];
                            PLASSERT( out_val && out_prev && out_grad );
                            for (int oi = 0; oi < out_val->length(); oi++)
                                for (int oj = 0; oj < out_val->width(); oj++) {
                                    real diff = (*out_val)(oi, oj) - 
                                        (*out_prev)(oi, oj);
                                    (*grad)(p, q) +=
                                        diff * (*out_grad)(oi, oj) / step;
                                }
                        }
                    }
                // Compare estimated and computed gradients.
                for (int p = 0; p < grad->length(); p++)
                    for (int q = 0; q < grad->width(); q++)
                        if (!is_equal((*grad)(p,q), (*b_check)(p,q),
                                    absolute_tolerance_threshold,
                                    absolute_tolerance, relative_tolerance)) {
                            pout << "Gradient for port '" <<
                                module->getPortName(k) << "' was not " <<
                                "properly estimated: " << (*grad)(p,q) <<
                                " != " << (*b_check)(p,q) << endl;
                            ok = false;
                        }

            }
        }
    }
    if (ok)
        pout << "All tests passed successfully" << endl;
    else
        pout << "*** ERRROR ***" << endl;
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
