// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearner.h
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
   * $Id: PLearner.h,v 1.2 2003/05/03 05:02:18 plearner Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/PLearner.h */

#ifndef PLearner_INC
#define PLearner_INC

#include "Kernel.h"
#include "VecStatsCollector.h"
#include "VVec.h"
//#include "TimeMeasurer.h"

namespace PLearn <%
using namespace std;

/*!     The base class for learning algorithms, which
    should be the main "products" of PLearn.
*/
  class PLearner: public Object
  {
  protected:

    VMat train_set;  // The training set as set by setTrainingSet

    //! the directory in which to save files related to this model (see setExperimentDirectory())
    //! You may assume that it ends with a slash (setExperimentDirectory(...) ensures this).
    string expdir; 

  public:

    typedef Object inherited;
    
    // Build options
    int inputsize_;  //!<  The data VMat's are assumed to be formed of inputsize()
    int targetsize_; //!<  columns followed by targetsize() columns.
    int outputsize_; //!<  the use() method produces an output vector of size outputsize().
    int weightsize_; //<! number of weight fields in the target vec (all_targets = actual_target & weights)
    long seed;
    int stage;
    int nstages;

  public:

    PLearner();
    virtual ~PLearner();

    //! The experiment directory is the directory in which files 
    //! related to this model are to be saved. 
    /*! Typically, the following files will be saved in that directory:
      model.psave  (saved best model)
      model#.psave (model saved after epoch #)
      model#.<trainset_alias>.objective (training objective and costs after each epoch)
      model#.<testset_alias>.results  (test results after each epoch)
    */
    virtual void setExperimentDirectory(const string& the_expdir);
    string getExperimentDirectory() const { return expdir; }

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(PLearner);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  private:
    /*!       **** SUBCLASS WRITING: ****
      The build_ and build methods should be redefined in subclasses
      build_ should do the actual building of the PLearner according
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

    //!  Simple accessor methods: (do NOT overload! Set inputsize_ and outputsize_ instead)
    inline int inputsize() const { return inputsize_; }
    inline int targetsize() const { return targetsize_; }
    inline int outputsize() const { return outputsize_; }
    inline int weightsize() const { return weightsize_; }

    /*!       *** SUBCLASS WRITING: ***
      This method should be called AFTER or inside the build method,
      e.g. in order to re-initialize parameters. It should
      put the PLearner in a 'fresh' state, not being influenced
      by any past call to train (everything learned is forgotten!).
    */
    virtual void forget();

    //! Declare the train_set
    virtual void setTrainingSet(VMat training_set) { train_set = training_set; }
    inline VMat getTrainingSet() { return train_set; }

    
    /*! *** SUBCLASS WRITING: ***
      Should do the actual training until epoch==nepochs 
      and should call update on the stats with training costs measured on-line
    */
    virtual void train(VecStatsCollector& train_stats) =0;

    //! *** SUBCLASS WRITING: ***
    //! This should be defined in subclasses to compute the output from the input
    virtual void computeOutput(const VVec& input, Vec& output) =0;

    //! *** SUBCLASS WRITING: ***
    //! This should be defined in subclasses to compute the weighted costs from
    //! already computed output.
    virtual void computeCostsFromOutputs(const VVec& input, const Vec& output, 
                                         const VVec& target, const VVec& weight,
                                         Vec& costs) =0;
                                
    //! Default calls computeOutput and computeCostsFromOutputs
    //! You may overload this if you have a more efficient way to 
    //! compute both output and weighted costs at the same time.
    virtual void computeOutputAndCosts(const VVec& input, VVec& target, const VVec& weight,
                                       Vec& output, Vec& costs);

    //! Default calls computeOutputAndCosts
    //! This may be overloaded if there is a more efficient way to compute the costs
    //! directly, without computing the whole output vector.
    virtual void computeCostsOnly(const VVec& input, VVec& target, VVec& weight, 
                              Vec& costs);
    
  
    //! Performs test on testset, updating test cost statistics,
    //! and optionally filling testoutputs and testcosts
    //! The default version repeatedly calls computeOutputAndCosts or computeCostsOnly
    virtual void test(VMat testset, VecStatsCollector& test_stats, 
                      VMat testoutputs=0, VMat testcosts=0);

    //! *** SUBCLASS WRITING: ***
    //! This should return the names of the costs computed by computeCostsFromOutpus
    virtual TVec<string> getTestCostNames() const =0;

    //! *** SUBCLASS WRITING: ***
    //! This should return the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const =0;

    inline int nTestCosts() const { return getTestCostNames().size(); }
    inline int nTrainCosts() const { return getTrainCostNames().size(); }

    //! returns the index of the given cost in the vector of testcosts (returns -1 if not found)
    int getTestCostIndex(const string& costname) const;

    //! returns the index of the given cost in the vector of traincosts (objectives) (returns -1 if not found)
    int getTrainCostIndex(const string& costname) const;

  protected:
    static void declareOptions(OptionList& ol);

  };

  DECLARE_OBJECT_PTR(PLearner);


%> // end of namespace PLearn

#endif





