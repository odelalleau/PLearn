// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// Learner.h
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
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



#ifndef Learner_INC
#define Learner_INC

#include <plearn/measure/Measurer.h>
#include <plearn/ker/Kernel.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/math/StatsIterator.h>
#include <plearn/vmat/VVec.h>
//#include "TimeMeasurer.h"

namespace PLearn {
using namespace std;

/*! @deprecated This class is DEPRECATED, derive from PLearner instead.

  The base class for learning algorithms, which
  should be the main "products" of PLearn.
    
  The main thing that a Learner can do are:
  void train(VMat training_set); <  get trained
  void use(const Vec& input, Vec& output); <  compute output given input
  Vec test(VMat test_set); <  compute some performance statistics on a test set
  <  compute outputs and costs when applying trained model on data
  void applyAndComputeCosts(const VMat& data, VMat outputs, VMat costs);
    
*/
class Learner: public Object, public Measurer//, public TimeMeasurer
{
protected:

    Vec tmpvec; // for temporary storage.

    // EN FAIRE UN POINTEUR AUSSI
    ofstream* train_objective_stream; //!< file stream where to save objecties and costs during training 
    Array<ofstream*> test_results_streams; //!< opened streams where to save test results

private:

    static Vec tmp_input; // temporary input vec
    static Vec tmp_target; // temporary target vec
    static Vec tmp_weight; // temporary example weight vec
    static Vec tmp_output; // temporary output vec
    static Vec tmp_costs; // temporary costs vec

protected:

    //! opens the train.objective file for appending in the expdir
    void openTrainObjectiveStream();
    
    //! resturns the stream for writing train objective (and other costs)
    //! The stream is opened by calling openTrainObjectivestream if it wasn't already
    ostream& getTrainObjectiveStream();

    //! opens the files in append mode for writing the test results
    void openTestResultsStreams();

    //! Returns the stream corresponding to testset #k (as specified by setTestDuringTrain)
    //! The stream is opened by calling opentestResultsStreams if it wasn's already.
    ostream& getTestResultsStream(int k);
    
    //! frees the resources used by the test_results_streams
    void freeTestResultsStreams();

    //! output a test result line to a file
    void outputResultLineToFile(const string & filename, const Vec& results,bool append,const string& names);

protected:
    //! the directory in which to save files related to this model (see setExperimentDirectory())
    //! You may assume that it ends with a slash (setExperimentDirectory(...) ensures this).
    string expdir; 
    
    //! It's used as part of the model filename saved by calling save(), which measure() does if ??? incomplete ???
    int epoch_;
    
    //! This is set to true to indicate that MPI parallelization occured at the level of this learner
    //! possibly with data distributed across several nodes (in which case PLMPI::synchronized should be false)
    //! (this is initially false)
    bool distributed_;


public:

    //! returns expdir+train_set->getAlias()  (if train_set is indeed defined and has an alias...)
    string basename() const;
    
    typedef Object inherited;
    
    int inputsize_;  //!<  The data VMat's are assumed to be formed of inputsize()
    int targetsize_; //!<  columns followed by targetsize() columns.
    int outputsize_; //!<  the use() method produces an output vector of size outputsize().
    int weightsize_; //!< number of weight fields in the target vec (all_targets = actual_target & weights)

    //! By default, MPI parallelization done at given level prevents further parallelization
    //! at lower levels. If true, this means "don't parallelize processing at this level" 
    bool dont_parallelize; //!< (default: false)

    //!  test during train specifications
    //oassignstream testout;
    PStream testout;
    int test_every;
    Vec avg_objective; //!<  average of the objective function(s) over the last test_every steps
    Vec avgsq_objective; //!<  average of the squared objective function(s) over the last test_every steps
    VMat train_set; //!< the current set being used for training
    Array<VMat> test_sets; //!< test sets to test on during train
    int minibatch_size; //!< test by blocks of this size using apply rather than use

/*!       report test progress in vlog (see below) every that many iterations
  For each nth test sample, this will print a "Test sample #n" line in
  vlog (where n is the value in report_test_progress_every)
*/
    int report_test_progress_every;

    //!  DEPRECATED
    //!  options in the construction of the model through setModel
    Vec options;

    //!  early-stopping parameters
    int earlystop_testsetnum; //!<  index of test set (in test_sets) to use for early stopping
    int earlystop_testresultindex; //!<  index of statistic (as returned by test) to use
    real earlystop_max_degradation; //!<  maximum degradation in error from last best value
    real earlystop_min_value; //!<  minimum error beyond which we stop
    real earlystop_min_improvement; //!<  minimum improvement in error otherwise we stop
    bool earlystop_relative_changes;  //!<  are max_degradation and min_improvement relative?
    bool earlystop_save_best; //!<  if yes, then return with saved "best" model
    int earlystop_max_degraded_steps; //!< max. nb of steps beyond best found [in version >= 1]

    bool save_at_every_epoch; //!<  save learner at each epoch?
    bool save_objective; //!< whether to save in basename()+".objective" the cost after each measure (e.g. after each epoch)
    int best_step; //!<  the step (usually epoch) at which validation cost was best

protected:
    //!  temporary values relevant for early stopping
    real earlystop_previousval;
public:
    real earlystop_minval;

    // DPERECATED. Please use the expdir system from now on, through setExperimentDirectory
    string experiment_name;

protected:
    //strstream earlystop_best_model; //!<  string stream where the currently best model is saved
    
    //!  array of measurers:
    Array<Measurer*> measurers;
    
    bool measure_cpu_time_first; // the first el. in measure(..) will be cpu time instead of courant step

    bool each_cpu_saves_its_errors;
public:
    Array<CostFunc> test_costfuncs;
    StatsItArray test_statistics;

    static int use_file_if_bigger; //!<  number of elements above which a file VMatrix rather 
    //!  than an in-memory one should be used 
    //!  (when computing statistics requiring multiple passes over a test set)

    static bool force_saving_on_all_processes; //!< otherwise in MPI only CPU0 actually saves

    static PStream& /*oassignstream&*/ default_vlog(); //!<  The default stream to which lout is set upon construction of all Learners (defaults to cout)
    //oassignstream vlog; //!<  The log stream to which all the verbose output from this learner should be sent
    //oassignstream objectiveout; //!<  The log stream to use to record the objective function during training
    PStream vlog; //!<  The log stream to which all the verbose output from this learner should be sent
    PStream objectiveout; //!<  The log stream to use to record the objective function during training

/*!       **** SUBCLASS WRITING: ****
  All subclasses of Learner should implement this form of constructor
  Constructors should simply set all build options (member variables)
  to acceptable values and call build() that will do the actual job of
  constructing the object.
*/
    Learner(int the_inputsize=0, int the_targetsize=0, int the_outputsize=0);

    virtual ~Learner();

    //! The experiment directory is the directory in which files 
    //! related to this model are to be saved. 
    /*! Typically, the following files will be saved in that directory:
      model.psave  (saved best model)
      model#.psave (model saved after epoch #)
      model#.<trainset_alias>.objective (training objective and costs after each epoch)
      model#.<testset_alias>.results  (test results after each epoch)
    */
    virtual void setExperimentDirectory(const PPath& the_expdir);
    string getExperimentDirectory() const { return expdir; }

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    PLEARN_DECLARE_ABSTRACT_OBJECT(Learner);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

private:
    /*!       **** SUBCLASS WRITING: ****
      The build_ and build methods should be redefined in subclasses
      build_ should do the actual building of the Learner according
      to build options (member variables) previously set.  (These may have
      been set by hand, by a constructor, by the load method, or by
      setOption) As build() may be called several times (after changing
      options, to "rebuild" an object with different build options), make
      sure your implementation can handle this properly.
    */
    void build_();
    
public:
    //!  **** SUBCLASS WRITING: ****
    //! This method should be redefined in subclasses, to just call inherited::build() and then build_()
    virtual void build();

    //! Declare the train_set
    virtual void setTrainingSet(VMat training_set) { train_set = training_set; }
    inline VMat getTrainingSet() { return train_set; }

/*!       *** SUBCLASS WRITING: ***
  Does the actual training. Subclasses must implement this method.  
  The method should upon entry, call setTrainingSet(training_set);
  Make sure that a if(measure(step, objective_value)) is done after each
  training step, and that training is stopped if it returned true
*/
    virtual void train(VMat training_set) =0; 


    /*! *** SUBCLASS WRITING: ***
      Should do the actual training until epoch==nepochs 
      and should call update on the stats with training costs measured on-line
    */
    virtual void newtrain(VecStatsCollector& train_stats);


    //! Should perform test on testset, updating test cost statistics,
    //! and optionally filling testoutputs and testcosts
    virtual void newtest(VMat testset, VecStatsCollector& test_stats, 
                         VMat testoutputs=0, VMat testcosts=0);

    /*
      virtual void useAndCost(Vec input, Vec target, Vec output, Vec cost)

      virtual void trainTest(VMat train, Array<VMat> testsets);
      virtual void trainKFold(VMat trainset, int k);
      virtual void trainBootstrap(VMat trainset, int k, Array<VMat> testsets);
      virtual void trainSequential(VMat dataset, sequence_spec);
      
    */

/*!       *** SUBCLASS WRITING: ***
  Does the actual training.
  Permit to train from a sampling of a training set.
*/
    virtual void train(VMat training_set, VMat accept_prob,
                       real max_accept_prob=1.0, VMat weights=VMat())
    { PLERROR("This method is not implemented for this learner"); }

/*!       *** SUBCLASS WRITING: ***
  Uses a trained decider on input, filling output.
  If the cost should also be computed, then the user
  should call useAndCost instead of this method.
*/
    virtual void use(const Vec& input, Vec& output) =0;
    virtual void use(const Mat& inputs, Mat outputs) 
    { 
        for (int i=0;i<inputs.length();i++) 
        {
            Vec input = inputs(i);
            Vec output = outputs(i);
            use(input,output);
        }
    }

    //! **Next generation** learners allow inputs to be anything, not just Vec
    Vec vec_input;

    //! *** SUBCLASS WRITING: ***
    //! This should be overloaded in subclasses to compute the output from the input
    // NOTE: For backward compatibility, default version currently calls
    // deprecated method use which should ultimately be removed...
    virtual void computeOutput(const VVec& input, Vec& output);

    //! *** SUBCLASS WRITING: ***
    //! This should be overloaded in subclasses to compute the weighted costs from
    //! already computed output.
    // NOTE: For backward compatibility, default version currently calls the
    // deprecated method computeCost which should ultimately be removed...
    virtual void computeCostsFromOutputs(const VVec& input, const Vec& output, 
                                         const VVec& target, const VVec& weight,
                                         Vec& costs);

                                
    //! Default calls computeOutput and computeCostsFromOutputs
    //! You may overload this if you have a more efficient way to 
    //! compute both output and weighted costs at the same time.
    virtual void computeOutputAndCosts(const VVec& input, VVec& target, const VVec& weight,
                                       Vec& output, Vec& costs);

    //! Default calls computeOutputAndCosts
    //! This may be overloaded if there is a more efficient way to compute the costs
    //! directly, without computing the whole output vector.
    virtual void computeCosts(const VVec& input, VVec& target, VVec& weight, 
                              Vec& costs);
    

/*!       ** DEPRECATED ** Do not use! 
  use the setOption and build methods instead
*/
    virtual void setModel(const Vec& new_options);

/*!       *** SUBCLASS WRITING: ***
  This method should be called AFTER or inside the build method,
  e.g. in order to re-initialize parameters. It should
  put the Learner in a 'fresh' state, not being influenced
  by any past call to train (everything learned is forgotten!).
*/
    virtual void forget();

/*!       **** SUBCLASS WRITING:
 * This method should be called by iterative training algorithm's
 train method after each training step (meaning of training step is
 learner-dependent) passing it the current step number and the costs
 relevant for the training process.
 * Training must be stopped if the returned value is true: it
 indicates early-stopping criterion has been met.
 * Default version writes step and costs to objectiveout stream at
 each step
 * Default version also performs the tests specified by
 setTestDuringTrain every 'test_every' steps and decides upon
 early-stopping as specified by setEarlyStopping.
 * Default version also calls the measure method of all measurers 
 that have been declared for addition with appendMeasurer
      
 This is the measure method from Measurer.
 You may override this method if you wish to measure 
 other things during the training.
 In this case your method will probably want to call this default
 version (Learner::measure) as part of it. 
*/
    virtual bool measure(int step, const Vec& costs);

/*!       *** SUBCLASS WRITING: ***
  This matched pair of Object functions needs to be 
  redefined by sub-classes. They are used for saving/loading
  a model to memory or to file. However, subclasses can call this
  one to deal with the saving/loading of the following data fields:
  the current options and the early stopping parameters.
*/
    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

    //! DEPRECATED. Call PLearn::save(filename, object) instead
    void save(const PPath& filename="") const;
    //! DEPRECATED. Call PLearn::load(filename, object) instead
    void load(const PPath& filename="");

    //! stopping condition, by default when a file
    //! named experiment_name + "_stop" is found to exist.
    //! If that is the case then this file is removed
    //! and exit(0) is performed.
    virtual void stop_if_wanted();

    //!  Simple accessor methods: (do NOT overload! Set inputsize_ and outputsize_ instead)
    inline int inputsize() const { return inputsize_; }
    inline int targetsize() const { return targetsize_; }
    inline int outputsize() const { return outputsize_; }
    inline int weightsize() const { return weightsize_; }
    inline int epoch() const { return epoch_; }

    //!  **** SUBCLASS WRITING:
    //!  should be re-defined if user re-defines computeCost
    //! default version returns 
    virtual int costsize() const;
  
    //!  Call this method to define what cost functions are computed by default
    //!  (these are generic cost functions which compare the output with the target)
    void setTestCostFunctions(Array<CostFunc> costfunctions)
    { test_costfuncs = costfunctions; }

    //!  This method defines what statistics are computed on the costs
    //!  (which compute a vector of statistics that depend on all the test costs)
    void setTestStatistics(StatsItArray statistics)
    { test_statistics = statistics; }

    //!  testout: the stream where the test results are to be written
    //!  every: how often (number of iterations) the tests should be performed
    virtual void setTestDuringTrain(ostream& testout, int every,
                                    Array<VMat> testsets);

    //! 
    virtual void setTestDuringTrain(Array<VMat> testsets);


    //!  return the test sets that are used during training
    const Array<VMat>& getTestDuringTrain() const {
        return test_sets;
    }
      

/*!       which_testset and which_testresult select the appropriate testset and
  costfunction to base early-stopping on from those that were specified
  in setTestDuringTrain 
  * degradation is the difference between the current value and the
  smallest value ever attained, training will be stopped if it grows
  beyond max_degradation 
  * training will be stopped if current value goes below min_value
  * training will be stopped if difference between previous value and
  current value is below min_improvement
  * if (relative_changes) is true then max_degradation is relative to
  the smallest value ever attained, and min_improvement is relative
  to the previous value.
  * if (save_best) then save the lowest validation error model
  (with the write method, to memory), and if early stopping occurs
  reload this saved model (with the read method).
*/
    void setEarlyStopping(int which_testset, int which_testresult, 
                          real max_degradation, real min_value=-FLT_MAX, 
                          real min_improvement=0, bool relative_changes=true, 
                          bool save_best=true, int max_degraded_steps=-1);

    //! computes the cost vec, given input, target and output
    //! The default version applies the declared CostFunc's 
    //! on the (output,target) pair, putting the cost
    //! computed for each CostFunc in an element of the cost vector.
    //! If you overload this method in subclasses (e.g. to compute 
    //! a cost that depends on the internal elements of the model), 
    //! you must also redefine costsize() and costNames() accordingly.
    virtual void computeCost(const Vec& input, const Vec& target, const Vec& output, const Vec& cost);

    //! By default this function calls use(input, output) and then computeCost(input, target, output, cost)
    //! So you can overload computeCost to change cost computation.
    virtual void useAndCost(const Vec& input, const Vec& target, 
                            Vec output, Vec cost);

    //! Default version calls useAndCost on test_set(i)
    //! so you don't need to overload this method
    //! unless you want to provide a more efficient implementation
    //! (for ex. if you have precomputed things for the test_set
    //! that you can use).
    virtual void useAndCostOnTestVec(const VMat& test_set, int i, const Vec& output, const Vec& cost);

/*!       Calls the 'use' method many times on the first inputsize() elements of
  each row of a 'data' VMat, and put the
  machine's 'outputs' in a writable VMat (e.g. maybe a file, or a matrix).
  Note: if one wants to compute costs as well, then the
  method applyAndComputeCosts should be called instead.
*/
    virtual void apply(const VMat& data, VMat outputs);

/*!       This method calls useAndCost repetitively on all the rows of data,
  putting all the resulting output and cost vectors in the outputs and
  costs VMat's.
*/
    virtual void applyAndComputeCosts(const VMat& data, VMat outputs, VMat costs);

    //! Like useAndCostOnTestVec, but on a block (of length minibatch_size) of rows
    //! from the test set: apply learner and compute outputs and costs for the block
    //! of test_set rows starting at i. By default calls applyAndComputeCosts.
    virtual void applyAndComputeCostsOnTestMat(const VMat& test_set, int i, const Mat& output_block, 
                                               const Mat& cost_block);

/*!       This method calls useAndCost repetitively on all the rows of data,
  throwing away the resulting output vectors but putting all the cost vectors
  in the costs VMat.
*/
    virtual void computeCosts(const VMat& data, VMat costs);

    //!  For each data point i, trains with dataset removeRow(data,i) and
    //!  calls useAndCost on point i, puts results in costs vmat
    virtual void computeLeaveOneOutCosts(const VMat& data, VMat costs);

/*!       Same as above, except a single cost passed as argument is computed,
  rather than all the Learner's costs setTestCostFunctions (and its
  possible additional internal cost).
*/
    virtual void computeLeaveOneOutCosts(const VMat& data, VMat costsmat, CostFunc costf);

/*!       Given a VMat of costs as computed for example with computeCosts
  or with applyAndComputeCosts, compute and the test statistics over
  those costs. This is the concatenation of the statistics computed
  for each of the columns (cost functions) of costs.
*/
    Vec computeTestStatistics(const VMat& costs);

/*!       Return statistics computed by test_statistics on the test_costfuncs.
  If (save_test_outputs) then the test outputs are saved in the given file,
  and similary if (save_test_costs).
*/
    virtual Vec test(VMat test_set, const string& save_test_outputs="", 
                     const string& save_test_costs="");

/*!       returns an Array of strings for the names of the components of the
  cost.  Default version returns the info() strings of the cost
  functions in test_costfuncs
*/
    virtual Array<string> costNames() const;

/*!       returns an Array of strings for the names of the cost statistics
  returned by methods test and computeTestStatistics. Default
  version returns a cross product between the info() strings of
  test_statistics and the cost names returned by costNames()
*/
    virtual Array<string> testResultsNames() const;

    //! returns an array of strings corresponding to the names of the 
    //! fields that will be written to objectiveout
    //! (by default this calls testResultsNames() )
    virtual Array<string> trainObjectiveNames() const;

/*!       Declare a new measurer whose measure method will be called
  when the measure method of this learner is called (in particular
  after each training epoch).
*/
    void appendMeasurer(Measurer& measurer)
    { measurers.append(&measurer); }

protected:
    static void declareOptions(OptionList& ol);

    void setTrainCost(Vec &cost)
    { train_cost.resize(cost.length()); train_cost << cost; };
    Vec train_cost;
public:
    Vec getTrainCost() { return train_cost; };
};

DECLARE_OBJECT_PTR(Learner);

typedef PP<Learner> PPLearner;
	
inline void prettyprint_test_results(ostream& out, const Learner& learner, const Vec& results)
{
    Array<string> names = learner.testResultsNames();
    for (int i=0; i<names.size(); i++)
        out << names[i] << ": " << results[i] << endl;
}
  

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
