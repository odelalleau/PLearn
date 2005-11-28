// -*- C++ -*-

// PDistribution.cc
//
// Copyright (C) 2003 Pascal Vincent 
// Copyright (C) 2004-2005 University of Montreal
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
 ******************************************************* */

/*! \file PDistribution.cc */

#include "PDistribution.h"
#include <plearn/base/tostring.h>
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

///////////////////
// PDistribution //
///////////////////
PDistribution::PDistribution():
    random(new PRandom()),
    delta_curve(0.1),
    n_input(-1),
    n_target(-1),
    n_input_(0),
    n_target_(-1),
    lower_bound(0.),
    upper_bound(0.),
    n_curve_points(-1),
    outputs_def("l")
{}

PLEARN_IMPLEMENT_OBJECT(PDistribution, 
    "Base class for PLearn probability distributions.\n",
    "PDistributions derive from PLearner (as some of them may be fitted to\n"
    "data by training, but they have additional methods allowing e.g. to\n"
    "compute density or generate data points.\n"
    "\n"
    "By default, a PDistribution may be conditional to an input part x, in\n"
    "order to represent the conditional distribution of P(Y | X = x). An\n"
    "unconditional distribution should derive from UnconditionalDistribution\n"
    "as it has a simpler interface.\n"
    "Since we want to be able to compute for instance P(Y = y | X = x), both\n"
    "the input part 'x' and the target part 'y' must be considered as input\n"
    "from the PLearner framework point of view. Thus one must specify the\n"
    "size of the input part by the 'n_input' option, and the size of the\n"
    "target by the 'n_target' option, satisfying the following equality:\n"
    "   n_input + n_target == inputsize  (1)\n"
    "Optionally, 'n_input' or 'n_target' (but not both) may be set to -1,\n"
    "and the PDistribution will automatically guess the other size so that\n"
    "equation (1) is satisfied. This makes unconditional distributions\n"
    "feasible by setting 'n_input' to 0 and 'n_target' to -1.\n"
    "\n"
    "The default implementations of the learner-type methods for computing\n"
    "outputs and costs work as follows:\n"
    "  - the 'outputs_def' option allows to choose what is in the output\n"
    "    (e.g. log density, expectation, ...)\n"
    "  - the cost is a vector of size 1 containing only the negative log-\n"
    "    likelihood (NLL), i.e. -log(P(y|x)).\n"
    "\n"
    "For conditional distributions, the input must always be made of both\n"
    "the 'input' part (x) and the 'target' part (y), even if the output may\n"
    "not need the target part (e.g. to compute E[Y | X = x]). The exception\n"
    "is when computeOutput() needs to be called successively with the same\n"
    "value of 'x': in this case, after a first call with both 'x' and 'y',\n"
    "one may only provide 'y' as input in later calls, and 'x' will be\n"
    "assumed to be unchanged. Or, alternatively, the 'input_part' option\n"
    "can be set first, either through the options system or using the\n"
    "setInput(..) method.\n"
);

////////////////////
// declareOptions //
////////////////////
void PDistribution::declareOptions(OptionList& ol)
{

    // Build options.

    declareOption(
        ol, "outputs_def", &PDistribution::outputs_def,
                           OptionBase::buildoption,
        "Defines what will be given in output. This is a string where the\n"
        "characters have the following meaning:\n"
        "- 'l' : log_density\n"
        "- 'd' : density\n"
        "- 'c' : cdf\n"
        "- 's' : survival_fn\n"
        "- 'e' : expectation\n"
        "- 'v' : variance.\n"
        "\n"
        "If these options are specified in lower case they give the value\n"
        "associated with a given observation. In upper case, a curve is\n"
        "evaluated at regular intervals and produced in output (as a\n"
        "histogram). For 'L', 'D', 'C', 'S', it is the target part that\n"
        "varies, while for 'E' and 'V' it is the input part (for conditional\n"
        "distributions).\n"
        "The number of curve points is given by the 'n_curve_points' option.\n"
        "Note that the upper case letters only work for scalar variables, in\n"
        "order to produce a one-dimensional curve."
        );
    // TODO Make it TVec<string> for better clarity?

    declareOption(ol, "n_input",  &PDistribution::n_input_,
                                  OptionBase::buildoption,
        "The (user-provided) size of the input x in p(y|x). A value of -1\n"
        "means the algorithm should find it out by itself.");

    declareOption(ol, "n_target",  &PDistribution::n_target_,
                                   OptionBase::buildoption,
        "The (user-provided) size of the target y in p(y|x). A value of -1\n"
        "means the algorithm should find it out by itself.");

    declareOption(ol, "input_part", &PDistribution::input_part,
                                    OptionBase::buildoption,
        "For conditional distributions, the input part (= x in P(Y|X=x)).\n");
 
    declareOption(ol, "n_curve_points", &PDistribution::n_curve_points,
                                        OptionBase::buildoption,
        "The number of points for which the output is evaluated when\n"
        "outputs_defs is upper case (produces a histogram).\n"
        "The lower_bound and upper_bound options specify where the curve\n"
        "begins and ends. Note that these options (upper case letters) only\n"
        "work for scalar variables.");

    declareOption(ol, "lower_bound",  &PDistribution::lower_bound,
                                      OptionBase::buildoption,
        "The lower bound of scalar Y values to compute a histogram of the\n"
        "distribution when upper case outputs_def are specified.");

    declareOption(ol, "upper_bound",  &PDistribution::upper_bound,
                                      OptionBase::buildoption,
        "The upper bound of scalar Y values to compute a histogram of the\n"
        "distribution when upper case outputs_def are specified.");

    // Learnt options.

    declareOption(ol, "n_input_learnt",  &PDistribution::n_input,
                                         OptionBase::learntoption,
        "The (true) size of the input x in p(y|x). If 'n_input' is\n"
        "non-negative, it is equal to 'n_input'. Otherwise, it is set to the\n"
        "data dimension minus 'n_target'.");

    declareOption(ol, "n_target_learnt",  &PDistribution::n_target,
                                          OptionBase::learntoption,
        "The (true) size of the target y in p(y|x). if 'n_target' is\n"
        "non-negative, it is equal to 'n_target'. Otherwise, it is set to\n"
        "the data dimension minus 'n_input'.");
      
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void PDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void PDistribution::build_()
{
    // Reset the random number generator seed.
    resetGenerator(seed_);

    // Typical code for a PDistribution: the class makes the operations it
    // needs when the input and target sizes are defined, and when the input is
    // defined. In the build_() method, it should not call the parent's methods
    // since they should have already been called during the parent's build.
    PDistribution::setInputTargetSizes(n_input_, n_target_, false);
    PDistribution::setInput(input_part, false);

    // Set the step between two points in the output curve.
    if (n_curve_points > 0)
        delta_curve = (upper_bound - lower_bound) / real(n_curve_points);
}

///////////////////
// computeOutput //
///////////////////
void PDistribution::computeOutput(const Vec& input, Vec& output) const
{
    // TODO Add an output to generate samples.

    // Set the true 'input' (x in P(Y = y| X=x)) and 'target' (y) parts.
    splitCond(input);

    string::size_type l = outputs_def.length();
    int k = 0;
    for(unsigned int i=0; i<l; i++)
    {
        switch(outputs_def[i])
        {
        case 'l':
            output[k++] = log_density(target_part);
            break;
        case 'd':
            output[k++] = density(target_part);
            break;
        case 'c':
            output[k++] = cdf(target_part);
            break;
        case 's':
            output[k++] = survival_fn(target_part);
            break;
        case 'e':
            store_expect = output.subVec(k, n_target);
            expectation(store_expect);
            k += n_target;
            break;
        case 'v':
            store_cov =
                output.subVec(k, square(n_target)).toMat(n_target, n_target);
            variance(store_cov);
            k += square(n_target);
            break;
        case 'E':
        case 'V':
            if (n_target > 1)
                PLERROR("In PDistribution::computeOutput - Can only plot "
                        "histogram of expectation or variance for "
                        "one-dimensional target");
            if (n_target == 0)
                PLERROR("In PDistribution::computeOutput - Cannot plot "
                        "histogram of expectation or variance for "
                        "unconditional distributions");
        case 'L':
        case 'D':
        case 'C':
        case 'S':
            real t;
            store_result.resize(1);
            store_result[0] = lower_bound;
            for (int j = 0; j < n_curve_points; j++) {
                switch(outputs_def[i]) {
                case 'L':
                    t = log_density(store_result);
                    break;
                case 'D':
                    t = density(store_result);
                    break;
                case 'C':
                    t = cdf(store_result);
                    break;
                case 'S':
                    t = survival_fn(store_result);
                    break;
                case 'E':
                    setInput(store_result);
                    expectation(store_expect);
                    t = store_expect[0];
                    break;
                case 'V':
                    setInput(store_result);
                    store_cov = store_expect.toMat(1,1);
                    variance(store_cov);
                    t = store_expect[0];
                    break;
                default:
                    PLERROR("In PDistribution::computeOutput - This should "
                            "never happen");
                    t = 0; // To make the compiler happy.
                }
                output[j + k] = t;
                store_result[0] += delta_curve;
            }
            k += n_curve_points;
            break;
        default:
            // Maybe a subclass knows about this output?
            // TODO This is quite ugly. See how to do this better.
            unknownOutput(outputs_def[i], input, output, k);
            break;
        }
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void PDistribution::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                            const Vec& target, Vec& costs) const
{
    costs.resize(1);
    char c = outputs_def[0];
    if(c == 'l')
    {
        costs[0] = -output[0];
    }
    else if(c == 'd')
    {
        costs[0] = -pl_log(output[0]);
    }
    else
        PLERROR("In PDistribution::computeCostsFromOutputs currently can only "
                "compute' NLL cost from log likelihood or density returned as "
                "first output");
}                                

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> PDistribution::getTestCostNames() const
{
    static TVec<string> nll_cost;
    if (nll_cost.isEmpty())
        nll_cost.append("NLL");
    return nll_cost;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> PDistribution::getTrainCostNames() const
{
    // Default = no train cost computed. This may be overridden in subclasses.
    static TVec<string> no_cost;
    return no_cost;
}

///////////////
// generateN //
///////////////
void PDistribution::generateN(const Mat& Y) const
{
    Vec v;
    if (Y.width() != n_target)
        PLERROR("In PDistribution::generateN - Matrix width (%d) differs from "
                "n_target (%d)", Y.width(), n_target);
    int N = Y.length();  
    for(int i=0; i<N; i++)
    {
        v = Y(i);
        generate(v);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(store_expect,       copies);
    deepCopyField(store_result,       copies);
    deepCopyField(store_cov,          copies);
    deepCopyField(random,             copies);
    deepCopyField(input_part,         copies);
    deepCopyField(target_part,        copies);
}

////////////////
// outputsize //
////////////////
int PDistribution::outputsize() const
{
    int l = 0;
    for (size_t i=0; i<outputs_def.length(); i++) {
        if (outputs_def[i]=='L' || outputs_def[i]=='D' || outputs_def[i]=='C'
         || outputs_def[i]=='S' || outputs_def[i]=='E' || outputs_def[i]=='V')
            l+=n_curve_points;
        else if (outputs_def[i]=='e')
            l += n_target;
        else if (outputs_def[i]=='v')
            // Variance is full (n x n) matrix.
            l += square(n_target);
        else l++;
    }
    return l;
}

////////////////////
// resetGenerator //
////////////////////
void PDistribution::resetGenerator(long g_seed)
{
    if (g_seed != 0) {
        seed_ = g_seed;
        random->manual_seed(g_seed);
    }
}

//////////////
// setInput //
//////////////
void PDistribution::setInput(const Vec& input, bool call_parent) const {
    // Default behavior: only fill input_part with first elements of 'input'.
    assert(input.length() >= n_input);
    input_part << input.subVec(0, n_input);
}

/////////////////////////
// setInputTargetSizes //
/////////////////////////
bool PDistribution::setInputTargetSizes(int n_input, int n_target,
                                        bool call_parent)
{
    assert( (n_input  >= 0 || n_input  == -1) &&
            (n_target >= 0 || n_target == -1) );
    int backup_n_input = this->n_input;
    int backup_n_target = this->n_target;
    this->n_input = n_input;
    this->n_target = n_target;
    if (this->n_input < 0) {
        if (this->n_target < 0)
            PLERROR("In PDistribution::setInputTargetSizes - You need to "
                    "specify at least one non-negative value");
        if (inputsize_ >= 0) {
            if (this->n_target > inputsize_)
                PLERROR("In PDistribution::setInputTargetSizes - 'n_target' "
                        "(%d) cannot be higher than inputsize (%d)",
                        this->n_target, inputsize_);
            this->n_input = inputsize_ - this->n_target;
        }
    } else if (this->n_target < 0) {
        if (inputsize_ >= 0) {
            if (this->n_input > inputsize_)
                PLERROR("In PDistribution::setInputTargetSizes - 'n_input' "
                        "(%d) cannot be higher than inputsize (%d)",
                        this->n_input, inputsize_);
            this->n_target = inputsize_ - this->n_input;
        }
    }
    if (inputsize_ >= 0 && this->n_input + this->n_target != inputsize_)
        PLERROR("In PDistribution::setInputTargetSizes - n_input (%d) + "
                "n_target (%d) != inputsize (%d)",
                this->n_input, this->n_target, inputsize_);
    if (this->n_input >= 0)
        input_part.resize(this->n_input);
    if (this->n_target >= 0)
        target_part.resize(this->n_target);
    return (this->n_input  != backup_n_input ||
            this->n_target != backup_n_target);
}

///////////////
// splitCond //
///////////////
void PDistribution::splitCond(const Vec& input) const {
    if (n_input == 0 || (n_input > 0 && input.length() == n_target)) {
        // No input part provided: this means this is the same as before (or
        // that there is none at all).
        target_part << input;
    } else {
        assert( input.length() == n_input + n_target );
        target_part << input.subVec(n_input, n_target);
        setInput(input);
    }
}

////////////
// forget //
////////////
void PDistribution::forget() {
    stage = 0;
    n_input = -1;
    n_target = -1;
    resetGenerator(seed_);
}

////////////////////
// subclass stuff //
////////////////////

real PDistribution::log_density(const Vec& y) const
{ PLERROR("density not implemented for this PDistribution"); return 0; }

real PDistribution::density(const Vec& y) const
{ return exp(log_density(y)); }

real PDistribution::survival_fn(const Vec& y) const
{ PLERROR("survival_fn not implemented for this PDistribution"); return 0; }

real PDistribution::cdf(const Vec& y) const
{ PLERROR("cdf not implemented for this PDistribution"); return 0; }

void PDistribution::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for this PDistribution"); }

void PDistribution::variance(Mat& covar) const
{ PLERROR("variance not implemented for this PDistribution"); }

void PDistribution::generate(Vec& y) const
{ PLERROR("generate not implemented for this PDistribution"); }

void PDistribution::train()
{ PLERROR("The train() method is not implemented for this PDistribution"); }

///////////////////
// unknownOutput //
///////////////////
void PDistribution::unknownOutput(char def, const Vec& input, Vec& output,
                                  int& k) const
{
    // Default is to throw an error.
    // TODO Can we find a better way to do this?
    PLERROR("In PDistribution::unknownOutput - Unrecognized outputs_def "
            "character: '%c'", def);
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
