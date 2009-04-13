// -*- C++ -*-

// PLearner.h
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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



#ifndef PLearner_INC
#define PLearner_INC

#include <plearn/base/Object.h>
#include <plearn/io/PPath.h>
#include <plearn/math/PRandom.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;

/**
 *  The base class for learning algorithms, which should be the main "products"
 *  of PLearn.
 *
 *  PLearner provides a base class for all learning algorithms within PLearn.
 *  It presents an abstraction of learning that centers around a "train-test"
 *  paradigm:
 *
 *  - Phase 1: TRAINING.  In this phase, one must first establish an experiment
 *    directory (usually done by an enclosing PTester) to store any temporary
 *    files that the learner might seek to create.  Then, one sets a training
 *    set VMat (also done by the enclosing PTester), which contains the set of
 *    input-target pairs that the learner should attempt to represent.  Finally
 *    one calls the train() virtual member function to carry out the actual
 *    action of training the model.
 *
 *  - Phase 2: TESTING.  In this phase (to be done after training), one
 *    repeatedly calls functions from the computeOutput() family to evaluate
 *    the trained model on new input vectors.
 *
 *  Note that the PTester class is the usual "driver" for a PLearner (and
 *  automatically calls the above functions in the appropriate order), in the
 *  usual scenario wherein one wants to evaluate the generalization performance
 *  on a dataset.
 */
class PLearner: public Object
{
    typedef Object inherited;

    //! Cached number of training costs
    mutable int n_train_costs_;

    //! Cached number of test costs
    mutable int n_test_costs_;

    //! Global storage to save memory allocations.
    mutable Vec tmp_output;

public:
    //#####  Build Options  ###################################################

    /**
     *  Path of the directory associated with this learner, in which it should
     *  save any file it wishes to create.  The directory will be created if it
     *  does not already exist.  If expdir is the empty string (the default),
     *  then the learner should not create *any* file. Note that, anyway, most
     *  file creation and reporting are handled at the level of the PTester
     *  class rather than at the learner's.
     */
    PPath expdir; 

    /**
     *  The seed used for the random number generator in initializing the
     *  learner (see forget() method).  Default value=1827 for experiment
     *  reproducibility.
     */
    int seed_;

    /**
     *  The current training stage, since last fresh initialization (forget()):
     *  0 means untrained, n often means after n epochs or optimization steps,
     *  etc...  The true meaning is learner-dependant.  You should never modify
     *  this option directly!  It is the role of forget() to bring it back to
     *  0, and the role of train() to bring it up to 'nstages'...
     */
    int stage;

    /**
     *  The stage until which train() should train this learner and return.
     *  The meaning of 'stage' is learner-dependent, but for learners whose
     *  training is incremental (such as involving incremental optimization),
     *  it is typically synonym with the number of 'epochs', i.e. the number of
     *  passages of the optimization process through the whole training set,
     *  since the last fresh initialisation.
     */
    int nstages;

    //! Should progress in learning and testing be reported in a ProgressBar
    bool report_progress;

    /**
     *  Level of verbosity. If 0, should not write anything on cerr. If >0 may
     *  write some info on the steps performed (the amount of detail written
     *  depends on the value of this option).
     */
    int verbosity; 

    /**
     *  Max number of computation servers to use in parallel with the main
     *  process.
     *  DEPRECATED: use parallelize_here instead
     */
    int nservers; 

    /**
     *  Size of minibatches used during testing to take advantage
     *  of efficient (possibly parallelized) implementations when
     *  multiple exemples are processed at once. 
     */
    int test_minibatch_size;

    /**
     *  Whether the training set should be saved upon a call to
     *  setTrainingSet().  The saved file is put in the learner's expdir
     *  (assuming there is one) and has the form "<prefix>_trainset_XXX.pmat"
     *  The prefix is what this option specifies.  'XXX' is a unique serial
     *  number that is globally incremented with each saved setTrainingSet.
     *  This option is useful when manipulating very complex nested learner
     *  structures, and you want to ensure that the inner learner is getting
     *  the correct results.  (Default="", i.e. don't save anything.)
     */
    string save_trainingset_prefix;

    /**
     * Wether parallelism should be exploited at this object's level
     */
    bool parallelize_here;

    /**
     * For PLearner::test in parallel:
     * if true, the master reads the testset and sends rows to the slaves;
     * otherwise, the master sends a description of the testset to the slaves
     */
    bool master_sends_testset_rows;

    /**
     * This option allows to perform testing always in the same
     * conditions in terms of the random generator (if testing involves
     * some non-deterministic component, this can be useful in order
     * to obtain repeatable test results).
     * If non-zero, the base class test() method will use a different
     * random generator than the rest of the code (i.e. training).
     * The non-zero value is the seed to be used during testing.
     * A value of -1 sets the seed differently each time depending on clock.
     * (which is probably not desired here).
     */
    int use_a_separate_random_generator_for_testing;


    /**
     * (default false)
     * After training(when finalized() is called) it will be set to true.
     * When true, it mean the learner it won't be trained again and this
     * allow some optimization.
     */
    bool finalized;

protected:

    /**
     *  The training set as set by setTrainingSet.  Data-sets are seen as
     *  matrices whose columns or fields are layed out as follows: a number of
     *  input fields, followed by (optional) target fields, followed by a
     *  (optional) weight field (to weigh each example).  The sizes of those
     *  areas are given by the VMatrix options inputsize targetsize, and
     *  weightsize, which are typically used by the learner upon building.
     */
    VMat train_set;  

    //! Validation set used in some contexts
    VMat validation_set;

    //#####  Learnt Options  ##################################################

    //! Learnt inputsize obtained from train_set when doing setTrainingSet
    int inputsize_;

    //! Learnt targetsize obtained from train_set when doing setTrainingSet
    int targetsize_;

    //! Learnt weightsize obtained from train_set when doing setTrainingSet
    int weightsize_;

    //! Learnt number of examples obtained from train_set when doing setTrainingSet
    int n_examples;

    /**
     *  The stats_collector responsible for collecting train cost statistics
     *  during training. This is typically set by some external training
     *  harness that wants to collect some stats.
     */
    PP<VecStatsCollector> train_stats;

    /**
     *  Whether or not to call 'forget' when the training set changes, in
     *  setTrainingSet.
     */
    bool forget_when_training_set_changes;

    /**
     *  The random generator used by this PLearner. A subclass of PLearner that
     *  wants to use it must create it in its own constructor, as by default it
     *  is not created in the PLearner class itself.  The random generator, if
     *  present, will be automatically initialized from the 'seed' option in
     *  build() and forget().
     */
    mutable PP<PRandom> random_gen;

public:
    //! Default Constructor
    PLearner();


    //#####  Experiment Context  ##############################################
    
    /**
     *  Declares the training set.  Then calls build() and forget() if
     *  necessary.  Also sets this learner's inputsize_ targetsize_ weightsize_
     *  from those of the training_set.  Note: You shouldn't have to override
     *  this in subclasses, except in maybe to forward the call to an
     *  underlying learner.
     */
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! Returns the current train_set
    inline VMat getTrainingSet() const
    {
        return train_set;
    }

    //! Set the validation set (optionally) for learners that are able to use
    //! it directly 
    virtual void setValidationSet(VMat validset);

    //! Returns the current validation set
    VMat getValidationSet() const
    {
        return validation_set;
    }

    /**
     *  Sets the statistics collector whose update() method will be called
     *  during training.  Note: You shouldn't have to override this in
     *  subclasses, except maybe to forward the call to an underlying learner.
     */
    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //! Returns the train stats collector
    inline PP<VecStatsCollector> getTrainStatsCollector()
    {
        return train_stats;
    }

    /**
     *  The experiment directory is the directory in which files related to
     *  this model are to be saved.  If it is an empty string, it is understood
     *  to mean that the user doesn't want any file created by this learner.
     */
    virtual void setExperimentDirectory(const PPath& the_expdir);

    //! This returns the currently set expdir (see setExperimentDirectory)
    PPath getExperimentDirectory() const { return expdir; }

    //! Default returns inputsize_ cached from train_set->inputsize()
    virtual int inputsize() const;

    //! Default returns targetsize_ cached from train_set->targetsize()
    virtual int targetsize() const; 

    //! Default returns weightsize_ cached from train_set->weightsize()
    virtual int weightsize() const; 

    /**
     *  SUBCLASS WRITING: override this so that it returns the size of this
     *  learner's output, as a function of its inputsize(), targetsize() and
     *  set options.
     */
    virtual int outputsize() const = 0;

public:
    //! Finish building the object; just call inherited::build followed by
    //! build_()
    virtual void build();

protected:
    //! Building part of the PLearner that needs the train_set
    virtual void build_from_train_set() { }

    /**
     *  This method may be called by any PLearner at the very beginning of the
     *  'train' method. It will:
     *  - ensure that 'nstages' is non-negative (return false otherwise)
     *  - compare the stage to reach ('nstages') to the current PLearner stage:
     *    + if nstages > stage, do nothing (standard case)
     *    + if nstages == stage, return false
     *    + if nstages < stage, display a warning message (when verbosity >= 1)
     *      and call forget() (reverting to a previous stage means we need to 
     *      start again from stage 0)
     *  - check that a training set has been properly set (if it is not the
     *    case, a warning is displayed and 'false' is returned)
     *  - initialize a standard train_stats VecStatsCollector if there is none
     *    already
     *  Except in the cases described above, 'true' is returned. A 'false'
     *  value means that no training should take place.
     */
    bool initTrain();

private:
    /**
     *  **** SUBCLASS WRITING: ****
     *
     *  This method should finish building of the object, according to set
     *  'options', in *any* situation.
     * 
     *  Typical situations include:
     *
     *  - Initial building of an object from a few user-specified options
     *  - Building of a "reloaded" object: i.e. from the complete set of all
     *    serialised options.
     *  - Updating or "re-building" of an object after a few "tuning" options 
     *    (such as hyper-parameters) have been modified.
     * 
     *  You can assume that the parent class' build_() has already been called.
     * 
     *  A typical build method will want to know the inputsize(), targetsize()
     *  and outputsize(), and may also want to check whether
     *  train_set->hasWeights(). All these methods require a train_set to be
     *  set, so the first thing you may want to do, is check if(train_set),
     *  before doing any heavy building...
     * 
     *  Note: build() is always called by setTrainingSet.
     */
    void build_();

public:
    //#####  Training Protocol  ###############################################
    
    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  (Re-)initializes the PLearner in its fresh state (that state may depend
     *  on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
     *  a fresh learner!)
     *
     *  A typical forget() method should do the following:
     *
     *  - call inherited::forget() to initialize the random number generator
     *    with the 'seed' option
     *
     *  - initialize the learner's parameters, using this random generator
     *
     *  - stage = 0;
     *
     *  This method is typically called by the build_() method, after it has
     *  finished setting up the parameters, and if it deemed useful to set or
     *  reset the learner in its fresh state.  (remember build may be called
     *  after modifying options that do not necessarily require the learner to
     *  restart from a fresh state...)  forget is also called by the
     *  setTrainingSet method, after calling build(), so it will generally be
     *  called TWICE during setTrainingSet!
     */
    virtual void forget();

    /**
     *  *** SUBCLASS WRITING: ***
     *
     * When this method is called the learner know it we will never train it again.
     * So it can free resources that are needed only during the training.
     * The functions test()/computeOutputs()/... should continue to work.
     */
    virtual void finalize();

    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  The role of the train method is to bring the learner up to
     *  stage==nstages, updating the stats with training costs measured on-line
     *  in the process.
     *
     *  TYPICAL CODE:
     *  
     *  @code
     *  static Vec input;  // static so we don't reallocate/deallocate memory each time...
     *  static Vec target; // (but be careful that static means shared!)
     *  input.resize(inputsize());    // the train_set's inputsize()
     *  target.resize(targetsize());  // the train_set's targetsize()
     *  real weight;
     *  
     *  if(!train_stats)   // make a default stats collector, in case there's none
     *      train_stats = new VecStatsCollector();
     *  
     *  if(nstages<stage)  // asking to revert to a previous stage!
     *      forget();      // reset the learner to stage=0
     *  
     *  while(stage<nstages)
     *  {
     *      // clear statistics of previous epoch
     *      train_stats->forget(); 
     *            
     *      //... train for 1 stage, and update train_stats,
     *      // using train_set->getSample(input, target, weight);
     *      // and train_stats->update(train_costs)
     *          
     *      ++stage;
     *      train_stats->finalize(); // finalize statistics for this epoch
     *  }
     *  @endcode
     */
    virtual void train() =0;


    //#####  Output Computation  ##############################################

    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  This should be defined in subclasses to compute the output from the
     *  input.
     */
    virtual void computeOutput(const Vec& input, Vec& output) const;
    //! if it is more efficient to compute multipe outputs simultaneously,
    //! it can be advantageous to define the latter instead, in which
    //! each row of the matrices is associated with one example.
    virtual void computeOutputs(const Mat& input, Mat& output) const;

    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  This should be defined in subclasses to compute the weighted costs from
     *  already computed output. The costs should correspond to the cost names
     *  returned by getTestCostNames().
     *
     *  NOTE: In exotic cases, the cost may also depend on some info in the
     *  input, that's why the method also gets so see it.
     */
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const = 0;
    
    /**
     *  Default calls computeOutput and computeCostsFromOutputs.  You may
     *  override this if you have a more efficient way to compute both output
     *  and weighted costs at the same time.
     */
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;
    //! minibatch version of computeOutputAndCosts
    virtual void computeOutputsAndCosts(const Mat& input, const Mat& target,
                                        Mat& output, Mat& costs) const;

    /**
     *  Default calls computeOutputAndCosts.  This may be overridden if there
     *  is a more efficient way to compute the costs directly, without
     *  computing the whole output vector.
     */
    virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;

    /**
     *  Compute a confidence intervals for the output, given the input and the
     *  pre-computed output (resulting from computeOutput or similar).  The
     *  probability level of the confidence interval must be specified.
     *  (e.g. 0.95).  Result is stored in a TVec of pairs low:high for each
     *  output variable (this is a "box" interval; it does not account for
     *  correlations among the output variables).
     *
     *  If the interval can be computed, the function returns TRUE; otherwise
     *  (i.e. interval computation is not available), it returns FALSE.  The
     *  default implementation in PLearner is to return FALSE (with missing
     *  values in the returned intervals).
     */
    virtual
    bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                     real probability,
                                     TVec< pair<real,real> >& intervals) const;

    /**
     *  Version of computeOutput that is capable of returning an output matrix
     *  given an input matrix (set of output vectors), as well as the complete
     *  covariance matrix between the outputs
     *
     *  A separate covariance matrix is returned for each output dimension, but
     *  these matrices are allowed to share the same storage.  This would be
     *  the case in situations where the output covariance really depends only
     *  on the location of the training inputs, as in, e.g.,
     *  GaussianProcessRegressor.
     *
     *  The default implementation is to repeatedly call computeOutput,
     *  followed by computeConfidenceFromOutput (sampled with probability
     *  Erf[1/(2*Sqrt(2))], to extract 1*stddev given by subtraction of the two
     *  intervals, then squaring the stddev to obtain the variance), thereby
     *  filling a diagonal output covariance matrix.  If
     *  computeConfidenceFromOutput returns 'false' (confidence intervals not
     *  supported), the returned covariance matrix is filled with
     *  MISSING_VALUE.
     */
    virtual void computeOutputCovMat(const Mat& inputs, Mat& outputs,
                                     TVec<Mat>& covariance_matrices) const;
    
    /**
     *  Repeatedly calls computeOutput and computeConfidenceFromOutput with the
     *  rows of inputs.  Writes outputs_and_confidence rows (as a series of
     *  triples (output, low, high), one for each output)
     */
    virtual
    void batchComputeOutputAndConfidence(VMat inputs, real probability,
                                         VMat outputs_and_confidence) const;

    /**
     *  Computes outputs for the input part of testset.  testset is not
     *  required to contain a target part.  The default version repeatedly
     *  calls computeOutput.
     */
    virtual void use(VMat testset, VMat outputs) const;

    //! Returns a Mat that is a concatenation of the inputs and computed outputs.
    Mat computeInputOutputMat(VMat inputs) const;

    /**
     *  Return a Mat that is the contatenation of inputs, outputs, lower
     *  confidence bound, and upper confidence bound.  If confidence intervals
     *  cannot be computed for the learner, they are filled with MISSING_VALUE.
     */
    Mat computeInputOutputConfMat(VMat inputs, real probability) const;

    /**
     *  Return a Mat that is the contatenation of outputs, lower confidence
     *  bound, and upper confidence bound.  If confidence intervals cannot be
     *  computed for the learner, they are filled with MISSING_VALUE.
     */
    Mat computeOutputConfMat(VMat inputs, real probability) const;
    
    /**
     *  Compute the output on the training set of the learner, and save the
     *  result in the provided matrix.
     */
    virtual void useOnTrain(Mat& outputs) const;

    /**
     * 'remote' version of useOnTrain
     */
    virtual Mat remote_useOnTrain() const;

    /**
     *  Performs test on testset, updating test cost statistics, and optionally
     *  filling testoutputs and testcosts.  The default version repeatedly
     *  calls computeOutputAndCosts or computeCostsOnly.  Note that neither
     *  test_stats->forget() nor test_stats->finalize() is called, so that you
     *  should call them yourself (respectively before and after calling this
     *  method) if you don't plan to accumulate statistics.
     */
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs=0, VMat testcosts=0) const;

    /**
     *  sub-test:
     *  Called by parallel test on chunks of the testset.
     *  Performs test on testset, returns stats and optionally testoutputs and testcosts
     */
    virtual tuple<PP<VecStatsCollector>, VMat, VMat> sub_test(VMat testset, PP<VecStatsCollector> test_stats,
                                                      bool rtestoutputs, bool rtestcosts) const;
    
    /**
     * 'remote' interface for test
     */
    virtual tuple<PP<VecStatsCollector>, VMat, VMat> remote_test(VMat testset, PP<VecStatsCollector> test_stats,
                                                      bool rtestoutputs, bool rtestcosts) const;
    
    

    /**
     * Process a full dataset (possibly containing input,target,weight,extra
     * parts). Returns processed view of that dataset. The default version
     * uses computeOutput to process the input part, and simply passes on
     * the other parts unchanged.
     */
    virtual VMat processDataSet(VMat dataset) const;

    //#####  Cost Names  ######################################################

    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  This should return the names of the costs computed by
     *  computeCostsFromOutputs.
     */
    virtual TVec<string> getTestCostNames() const =0;

    /**
     *  *** SUBCLASS WRITING: ***
     *
     *  This should return the names of the objective costs that the train
     *  method computes and for which it updates the VecStatsCollector
     *  train_stats.
     */
    virtual TVec<string> getTrainCostNames() const =0;

    /**
     *  Returns a vector of length outputsize() containing the outputs' names.
     *  Default version returns ["out0", "out1", ...]
     *  Don't forget name should not have space or it will cause trouble when
     *  they are saved in the file {metadatadir}/fieldnames
     */
    virtual TVec<string> getOutputNames() const;

    /**
     *  Caches getTestCostNames().size() in an internal variable the first time
     *  it is called, and then returns the content of this variable.
     */
    virtual int nTestCosts() const;

    /**
     *  Caches getTrainCostNames().size() in an internal variable the first
     *  time it is called, and then returns the content of this variable.
     */
    virtual int nTrainCosts() const;

    /**
     *  Returns the index of the given cost in the vector of testcosts; calls
     *  PLERROR (throws a PLearnException) if requested cost is not found.
     */
    int getTestCostIndex(const string& costname) const;

    /**
     *  Returns the index of the given cost in the vector of traincosts
     *  (objectives); calls PLERROR (throws a PLearnException) if requested
     *  cost is not found.
     */
    int getTrainCostIndex(const string& costname) const;


    //#####  Stateful Learning  ###############################################

    //! If any, reset the internal state
    //! Default: do nothing
    virtual void resetInternalState();

    //! Does this PLearner has an internal state?
    //! Default: false
    virtual bool isStatefulLearner() const;

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

private:
    // List of methods that are called by Remote Method Invocation.  Our
    // convention is to have them start with the remote_ prefix.
    Vec remote_computeOutput(const Vec& input) const;
    Mat remote_computeOutputs(const Mat& input) const;
    pair<Mat, Mat> remote_computeOutputsAndCosts(const Mat& input,
                                                 const Mat& target) const;
    void remote_use(VMat inputs, string output_fname) const;
    Mat remote_use2(VMat inputs) const;
    tuple<Vec,Vec> remote_computeOutputAndCosts(const Vec& input, const Vec& target) const;
    Vec remote_computeCostsFromOutputs(const Vec& input,
                                       const Vec& output, const Vec& target) const;
    Vec remote_computeCostsOnly(const Vec& input, const Vec& target) const;
    TVec< pair<real,real> > remote_computeConfidenceFromOutput(const Vec& input,
                                                               const Vec& output,
                                                               real probability) const;
    tuple<Mat, TVec<Mat> > remote_computeOutputCovMat(const Mat& inputs) const;
    void remote_batchComputeOutputAndConfidence(VMat inputs, real probability,
                                                string pmat_fname) const;
protected:
    mutable Mat b_inputs, b_targets, b_outputs, b_costs;
    mutable Vec b_weights;
    
public:
    PLEARN_DECLARE_ABSTRACT_OBJECT(PLearner);

};

DECLARE_OBJECT_PTR(PLearner);

} // end of namespace PLearn

#endif


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
