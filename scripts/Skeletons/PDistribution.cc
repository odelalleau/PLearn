#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;

DERIVEDCLASS::DERIVEDCLASS() 
/* ### Initialise all fields to their default value here */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

  PLEARN_IMPLEMENT_OBJECT(DERIVEDCLASS, "ONE LINE DESCR", "NO HELP");

  void DERIVEDCLASS::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &DERIVEDCLASS::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string DERIVEDCLASS::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "DERIVEDCLASS implements a ...\n";
  }

  void DERIVEDCLASS::build_()
  {
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
  }

  // ### Nothing to add here, simply calls build_
  void DERIVEDCLASS::build()
  {
    inherited::build();
    build_();
  }

  void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }


double DERIVEDCLASS::log_density(const Vec& x) const
{ PLERROR("density not implemented for DERIVEDCLASS"); return 0; }

double DERIVEDCLASS::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented for DERIVEDCLASS"); return 0; }

double DERIVEDCLASS::cdf(const Vec& x) const
{ PLERROR("cdf not implemented for DERIVEDCLASS"); return 0; }

void DERIVEDCLASS::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for DERIVEDCLASS"); }

void DERIVEDCLASS::variance(Mat& covar) const
{ PLERROR("variance not implemented for DERIVEDCLASS"); }

void DERIVEDCLASS::resetGenerator(long g_seed) const
{ PLERROR("resetGenerator not implemented for DERIVEDCLASS"); }

void DERIVEDCLASS::generate(Vec& x) const
{ PLERROR("generate not implemented for DERIVEDCLASS"); }


// Default version of inputsize returns learner->inputsize()
// If this is not appropriate, you should uncomment this and define
// it properly in the .cc
// int DERIVEDCLASS::inputsize() const {}

//! Remove this method, if your distribution does not implement it
void DERIVEDCLASS::forget()
{
  //! (Re-)initialize the PDistribution in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
         - initialize a random number generator with the seed option
         - initialize the learner's parameters, using this random generator
         - stage = 0
    */
}
    
//! Remove this method, if your distribution does not implement it
void DERIVEDCLASS::train()
{
  PLERROR("train method not implemented for DERIVEDCLASS");
  
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:

      static Vec input  // static so we don't reallocate/deallocate memory each time...
      static Vec target
      input.resize(inputsize())    // the train_set's inputsize()
      target.resize(targetsize())  // the train_set's targetsize()
      real weight

      if(!train_stats)  // make a default stats collector, in case there's none
         train_stats = new VecStatsCollector()

      if(nstages<stage) // asking to revert to a previous stage!
         forget()  // reset the learner to stage=0

      while(stage<nstages)
        {
          // clear statistics of previous epoch
          train_stats->forget() 
          
          //... train for 1 stage, and update train_stats,
          // using train_set->getSample(input, target, weight)
          // and train_stats->update(train_costs)
          
          ++stage
          train_stats->finalize() // finalize statistics for this epoch
        }
    */
}



%> // end of namespace PLearn
