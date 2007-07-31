    // -*- C++ -*-


// TransformationLearner.cc
//
// Copyright (C) 2007 Lysiane Bouchard
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

// Authors: Lysiane Bouchard

/*! \file TransformationLearner.cc */


#include "TransformationLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TransformationLearner,
    "ONE LINE DESCR",
    "NO HELP"
);

//////////////////
// TransformationLearner //
//////////////////
TransformationLearner::TransformationLearner():
    behavior(BEHAVIOR_LEARNER),
    minimumProba(0.0001),
    transformFamily(TRANSFORM_FAMILY_LINEAR_INCREMENT),
    withBias(false),
    learnNoiseVariance(false),
    regOnNoiseVariance(false),
    learnTransformDistribution(false),
    regOnTransformDistribution(false),
    initializationMode(INIT_MODE_DEFAULT),
    largeEStepAPeriod(UNDEFINED),
    largeEStepAOffset(UNDEFINED),
    largeEStepBPeriod(UNDEFINED),
    largeEStepBOffset(UNDEFINED),
    noiseVariancePeriod(UNDEFINED),
    noiseVarianceOffset(UNDEFINED),
    noiseAlpha(NOISE_ALPHA_NO_REG),
    noiseBeta(NOISE_BETA_NO_REG),
    transformDistributionPeriod(UNDEFINED),
    transformDistributionOffset(UNDEFINED),
    transformDistributionAlpha(TRANSFORM_DISTRIBUTION_ALPHA_NO_REG),
    transformsPeriod(UNDEFINED),
    transformsOffset(UNDEFINED),
    biasPeriod(UNDEFINED),
    biasOffset(UNDEFINED),
    noiseVariance(UNDEFINED),
    transformsVariance(1.0),
    nbTransforms(2),
    nbNeighbors(2)
{

    pout << "constructor called" <<endl;

}


////////////////////
// declareOptions //
////////////////////
void TransformationLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &TransformationLearner::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...


    //buildoption
  

    declareOption(ol,
                  "behavior",
                  &TransformationLearner::behavior,
                  OptionBase::buildoption,
                  "a transformationLearner might behave as a learner or as a generator");
    declareOption(ol,
                  "minimumProba",
                  &TransformationLearner::minimumProba,
                  OptionBase::buildoption,
                  "initial weight that will be needed sometimes");
    declareOption(ol,
                  "transformFamily",
                  &TransformationLearner::transformFamily,
                  OptionBase::buildoption,
                  "global form of the transformation functions");
    declareOption(ol,
                  "withBias",
                  &TransformationLearner::withBias,
                  OptionBase::buildoption,
                  "yes/no: add a bias to the transformation function ?");
    declareOption(ol,
                  "learnNoiseVariance",
                  &TransformationLearner::learnNoiseVariance,
                  OptionBase::buildoption,
                  "the noise variance is ...fixed/learned ?");
    declareOption(ol,
                  "regOnNoiseVariance",
                  &TransformationLearner::regOnNoiseVariance,
                  OptionBase::buildoption,
                  "yes/no: prior assumptions on the noise variance?");
    declareOption(ol,
                  "learnTransformDistribution",
                  &TransformationLearner::learnTransformDistribution,
                  OptionBase::buildoption,
                  "the transformation distribution is ... fixed/learned ?");
    declareOption(ol,
                  "regOnTransformDistribution",
                  &TransformationLearner::regOnTransformDistribution,
                  OptionBase::buildoption,
                  "yes/no: prior assumptions on the transformation distribution ?");
    
    declareOption(ol,
                  "initializationMode",
                  &TransformationLearner::initializationMode,
                  OptionBase::buildoption,
                  "how the initial values of the parameters to learn are choosen?");
    
    declareOption(ol,
                  "largeEStepAPeriod",
                  &TransformationLearner::largeEStepAPeriod,
                  OptionBase::buildoption,
                  "time interval between two updates of the reconstruction set\n"
                  "(version A, method largeEStepA())");
    declareOption(ol,
                  "largeEStepAOffset",
                  &TransformationLearner::largeEStepAOffset,
                  OptionBase::buildoption,
                  "time of the first update of the reconstruction set"
                  "(version A, method largeEStepA())");
    declareOption(ol,
                  "largeEStepBPeriod",
                  &TransformationLearner::largeEStepBPeriod,
                  OptionBase::buildoption,
                  "time interval between two updates of the reconstruction set\n"
                  "(version  B, method largeEStepB())"); 
    declareOption(ol,
                  "noiseVariancePeriod",
                  &TransformationLearner::noiseVariancePeriod,
                  OptionBase::buildoption,
                  "time interval between two updates of the noise variance");
    declareOption(ol,
                  "noiseVarianceOffset",
                  &TransformationLearner::noiseVarianceOffset,
                  OptionBase::buildoption,
                  "time of the first update of the noise variance");
    declareOption(ol,
                  "noiseAlpha",
                  &TransformationLearner::noiseAlpha,
                  OptionBase::buildoption,
                  "parameter of the prior distribution of the noise variance");
   declareOption(ol,
                 "noiseBeta",
                 &TransformationLearner::noiseBeta,
                 OptionBase::buildoption,
                 "parameter of the prior distribution of the noise variance");
   declareOption(ol,
                 "transformDistributionPeriod",
                 &TransformationLearner::transformDistributionPeriod,
                 OptionBase::buildoption,
                 "time interval between two updates of the transformation distribution");
   declareOption(ol, 
                 "transformDistributionOffset",
                 &TransformationLearner::transformDistributionOffset,
                 OptionBase::buildoption,
                 "time of the first update of the transformation distribution");
   declareOption(ol, 
                 "transformDistributionAlpha",
                 &TransformationLearner::transformDistributionAlpha,
                 OptionBase::buildoption,
                 "parameter of the prior distribution of the transformation distribution");
   declareOption(ol,
                 "transformsPeriod",
                 &TransformationLearner::transformsPeriod,
                 OptionBase::buildoption,
                 "time interval between two updates of the transformations matrices");
   declareOption(ol,
                 "transformsOffset",
                 &TransformationLearner::transformsOffset,
                 OptionBase::buildoption,
                 "time of the first update of the transformations matrices");

   declareOption(ol,
                 "biasPeriod",
                 &TransformationLearner::biasPeriod,
                 OptionBase::buildoption,
                 "time interval between two updates of the transformations bias");
   declareOption(ol,
                 "biasOffset",
                 &TransformationLearner::biasOffset,
                 OptionBase::buildoption,
                 "time of the first update of the transformations bias");

   declareOption(ol, 
                 "noiseVariance",
                 &TransformationLearner::noiseVariance,
                 OptionBase::buildoption,
                 "noise variance (noise = random variable normally distributed)");
   declareOption(ol, 
                 "transformsVariance",
                 &TransformationLearner::transformsVariance,
                 OptionBase::buildoption,
                 "variance on the transformation parameters (normally distributed)");
   declareOption(ol, 
                 "nbTransforms",
                 &TransformationLearner::nbTransforms,
                 OptionBase::buildoption,
                 "how many transformations?");
   declareOption(ol, 
                 "nbNeighbors",
                 &TransformationLearner::nbNeighbors,
                 OptionBase::buildoption,
                 "how many neighbors?");
   declareOption(ol, 
                 "transformDistribution",
                 &TransformationLearner::transformDistribution,
                 OptionBase::buildoption,
                 "transformation distribution");
   
   //learntoption
   declareOption(ol,
                 "train_set",
                 &TransformationLearner::train_set,
                 OptionBase::learntoption,
                 "We remember the training set, as this is a memory-based distribution." );
   declareOption(ol,
                 "transformsSet",
                 &TransformationLearner::transformsSet,
                 OptionBase::learntoption,
                 "set of transformations \n)"
                 "implemented as a mdXd matrix,\n"
                 "     where m is the number of transformations\n"
                 "           and d is dimensionality of the input space");
   declareOption(ol,
                 "transforms",
                 &TransformationLearner::transforms,
                 OptionBase::learntoption,
                 "set of transformations\n"
                 "vector form of the previous set:\n)"
                 "    kth element of the vector = view on the kth sub-matrix");
   declareOption(ol,
                 "biasSet",
                 &TransformationLearner::biasSet,
                 OptionBase::learntoption,
                 "set of bias (one by transformation)");
   declareOption(ol,
                 "inputSpaceDim",
                 &TransformationLearner::inputSpaceDim,
                 OptionBase::learntoption,
                 "dimensionality of the input space");
   
   declareOption(ol,
                 "reconstructionSet",
                 &TransformationLearner::reconstructionSet,
                 OptionBase::learntoption,
                 "set of weighted reconstruction candidates");
 
   // Now call the parent class' declareOptions().
   inherited::declareOptions(ol);
}

void TransformationLearner::declareMethods(RemoteMethodMap& rmm){



    rmm.inherited(inherited::_getRemoteMethodMap_());
    
    declareMethod(rmm, 
                  "initTransformsParameters",
                  &TransformationLearner::initTransformsParameters,
                  (BodyDoc("initializes the transformation parameters randomly \n"
                           "  (all parameters are a priori independent and normally distributed)")));
   
    declareMethod(rmm, 
                  "setTransformsParameters",
                  &TransformationLearner::setTransformsParameters,
                  (BodyDoc("initializes the transformation parameters with the given values"),
                   ArgDoc("TVec<Mat> transforms", "initial transformation matrices"),
                   ArgDoc("Mat  biasSet","initial bias (one by transformation) (optional)")));
    declareMethod(rmm, 
                  "initNoiseVariance",
                  &TransformationLearner::initNoiseVariance,
                  (BodyDoc("initializes the noise variance randomly (gamma distribution)")));
    declareMethod(rmm, 
                  "setNoiseVariance",
                  &TransformationLearner::setNoiseVariance,
                  (BodyDoc("initializes the noise variance to the given value"),
                   ArgDoc("real nv","noise variance")));
    declareMethod(rmm, 
                  "initTransformDistribution",
                  &TransformationLearner::initTransformDistribution,
                  (BodyDoc("initializes the transformation distribution randomly \n"
                           "-we use a dirichlet distribution \n"
                           "-we store log-probabilities instead probabilities")));
    declareMethod(rmm, 
                  "setTransformDistribution",
                  &TransformationLearner::setTransformDistribution,
                  (BodyDoc("initializes the transformation distribution with the given values \n"
                           " -the given values might represent log-probabilities"),
                   ArgDoc("Vec td","initial values of the transformation distribution")));
    
    declareMethod(rmm,
                  "returnPredictedFrom",
                  &TransformationLearner::returnPredictedFrom,
                  (BodyDoc("generates a sample data point from a source data point and returns it \n"
                           " - a specific transformation is used"),
                   ArgDoc("const Vec source","source data point"),
                   ArgDoc("int transformIdx","index of the transformation (optional)"),
                   RetDoc("Vec")));

    declareMethod(rmm,
                  "returnGeneratedSamplesFrom",
                  &TransformationLearner::returnGeneratedSamplesFrom,
                  (BodyDoc("generates samples data points form a source data point and return them \n"
                           "    -we use a specific transformation"),
                   ArgDoc("Vec source","source data point"),
                   ArgDoc("int n","number of samples"),
                   ArgDoc("int transformIdx", "index of the transformation (optional)"),
                   RetDoc("nXd matrix (one row = one sample)")));
    declareMethod(rmm,
                  "pickTransformIdx",
                  &TransformationLearner::pickTransformIdx,
                  (BodyDoc("select a transformation ramdomly"),
                   RetDoc("int (index of the choosen transformation)")));
               
    declareMethod(rmm,
                  "pickNeighborIdx",
                  &TransformationLearner::pickNeighborIdx,
                  (BodyDoc("select a neighbor among the data points in the training set"),
                   RetDoc("int (index of the data point in the training set)")));
    declareMethod(rmm,
                  "returnTreeDataSet",
                  &TransformationLearner::returnTreeDataSet,
                  (BodyDoc("creates and returns a data set using a 'tree generation process'\n"
                           " see 'treeDataSet()' implantation for more details"),
                   ArgDoc("Vec root","data point from which all the other data points will derive (directly or indirectly)"),
                   ArgDoc("int deepness","deepness of the tree reprenting the samples created"),
                   ArgDoc("int branchingFactor","branching factor of the tree representing the samples created"),
                   ArgDoc("int transformIdx", "index of the transformation to use (optional)"),
                   RetDoc("Mat (one row = one sample)")));
    declareMethod(rmm,
                  "returnSequenceDataSet",
                  &TransformationLearner::returnSequenceDataSet,
                  (BodyDoc("creates and returns a data set using a 'sequential procedure' \n"
                           "see 'sequenceDataSet()' implantation for more details"),
                   ArgDoc("const Vec start","data point from which all the other data points will derice (directly or indirectly)"),
                   ArgDoc("int n","number of sample data points to generate"),
                   ArgDoc("int transformIdx","index of the transformation to use (optional)"),
                   RetDoc("nXd matrix (one row = one sample)")));
    declareMethod(rmm,
                  "returnTrainingPoint",
                  &TransformationLearner::returnTrainingPoint,
                  (BodyDoc("returns the 'idx'th data point in the training set"),
                   ArgDoc("int idx","index of the data point in the training set"),
                   RetDoc("Vec")));
    declareMethod(rmm,
                  "returnReconstructionCandidates",
                  &TransformationLearner::returnReconstructionCandidates,
                  (BodyDoc("return all the reconstructions candidates associated to a given target"),
                   ArgDoc("int targetIdx","index of the target data point in the training set"),
                   RetDoc("TVec<ReconstructionCandidate>")));
    declareMethod(rmm,
                  "returnReconstructions",
                  &TransformationLearner::returnReconstructions,
                  (BodyDoc("returns the reconstructions of the 'targetIdx'th data point in the training set \n"
                           "(one reconstruction per reconstruction candidate)"),
                   ArgDoc("int targetIdx","index of the target data point in the training set"),
                   RetDoc("Mat (ith row = reconstruction associated to the ith reconstruction candidate)")));
    declareMethod(rmm,
                  "returnNeighbors",
                  &TransformationLearner::returnNeighbors,
                  (BodyDoc("returns the choosen neighbors of the target\n"
                           "  (one neighbor per reconstruction candidate)"),
                   ArgDoc("int targetIdx","index of the target in the training set"),
                   RetDoc("Mat (ith row = neighbor associated to the ith reconstruction candidate)")));
    declareMethod(rmm,
                  "returnTransform",
                  &TransformationLearner::returnTransform,
                  (BodyDoc("returns the parameters of the 'transformIdx'th transformation"),
                   ArgDoc("int transformIdx","index of the transformation"),
                   RetDoc("Mat")));
    declareMethod(rmm,
                  "returnAllTransforms",
                  &TransformationLearner::returnAllTransforms,
                  (BodyDoc("returns the parameters of each transformation"),
                   RetDoc("mdXd matrix, m = number of transformations \n"
                          "             d = dimensionality of the input space")));
    
    declareMethod(rmm,
                  "generatorBuild",
                  &TransformationLearner::generatorBuild,
                  (BodyDoc("generator specific initialization operations"),
                   ArgDoc("int inputSpaceDim","dimensionality of the input space"),
                   ArgDoc("TVec<Mat> transforms_", "transformations matrices"),
                   ArgDoc("Mat biasSet_","transformations bias"),
                   ArgDoc("real noiseVariance_","noise variance"),
                   ArgDoc("transformDistribution_", "transformation distribution")));
    declareMethod(rmm,
                  "gamma_sample",
                  &TransformationLearner::gamma_sample,
                  (BodyDoc("returns a pseudo-random positive real value using the distribution p(x)=Gamma(x |alpha,beta)"),
                   ArgDoc("real alpha",">=1"),
                   ArgDoc("real beta",">= 0 (optional: default value==1)"),
                   RetDoc("real >=0")));
    declareMethod(rmm,
                  "return_dirichlet_sample",
                  &TransformationLearner::return_dirichlet_sample,
                  (BodyDoc("returns a pseudo-random positive real vector using the distribution p(x)=Dirichlet(x|alpha)"),
                   ArgDoc("real alpha","all the parameters of the distribution are equal to 'alpha'"),
                   RetDoc("Vec (each element is between 0 and 1 , the elements sum to one)")));
/* declareMethod(rmm,
   "return_dirichlet_sample",
   &TransformationLearner::return_dirichlet_sample,
   (BodyDoc("returns a pseudo-random positive real vector using the distribution p(x)=Dirichlet(x|alphas)"),
   ArgDoc("Vec alphas","parameters of the distribution"),
   RetDoc("Vec (each element is between 0 and 1, the elements sum to one )"))); */
    declareMethod(rmm,
                  "initEStep",
                  &TransformationLearner::initEStep,
                  (BodyDoc("initial expectation step")));
    declareMethod(rmm,
                  "EStep",
                  &TransformationLearner::EStep,
                  (BodyDoc("coordination of the different kinds of expectation steps")));
    declareMethod(rmm,
                  "largeEStepA",
                  &TransformationLearner::largeEStepA,
                  (BodyDoc("update the reconstruction set \n"
                           "for each target, keeps the most probable <neighbor, transformation> pairs")));
    declareMethod(rmm,
                  "largeEStepB",
                  &TransformationLearner::largeEStepB,
                  (BodyDoc("update the reconstruction set \n"
                           "for each <target,transformation> pairs,choose the most probable neighbors ")));
    declareMethod(rmm,
                  "smallEStep",
                  &TransformationLearner::smallEStep,
                  (BodyDoc("update the weights of the reconstruction candidates")));
    declareMethod(rmm,
                  "MStep",
                  &TransformationLearner::MStep,
                  (BodyDoc("coordination of the different kinds of maximization step")));
    declareMethod(rmm,
                  "MStepTransformDistribution",
                  &TransformationLearner::MStepTransformDistribution,
                  (BodyDoc("maximization step with respect to transformation distribution parameters")));
    declareMethod(rmm,
                  "MStepTransformations",
                  &TransformationLearner::MStepTransformations,
                  (BodyDoc("maximization step with respect to transformation matrices (MAP version)")));
    declareMethod(rmm,
                  "MStepBias",
                  &TransformationLearner::MStepBias,
                  (BodyDoc("maximization step with respect to transformation bias (MAP version)")));
    declareMethod(rmm,
                  "MStepNoiseVariance",
                  &TransformationLearner::MStepNoiseVariance,
                  (BodyDoc("maximization step with respect to noise variance")));
    declareMethod(rmm,
                  "nextStage",
                  &TransformationLearner::nextStage,
                  (BodyDoc("increment 'stage' by one")));

}


///////////
// build //
///////////
void TransformationLearner::build()
{

    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TransformationLearner::build_()
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

    // ### In general, you will want to call this class' specific methods for
    // ### conditional distributions.
    // TransformationLearner::setPredictorPredictedSizes(predictor_size,
    //                                          predicted_size,
    //                                          false);
    // TransformationLearner::setPredictor(predictor_part, false);

 

    if(behavior == BEHAVIOR_LEARNER)
    {
        if(train_set.isNotNull())
        {
            mainLearnerBuild();
        }
     
    }
   
    else{
        generatorBuild(); //initialization of the parameters with all the default values
    }
        
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void TransformationLearner::forget()
{
    
    
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */    
    //PLERROR("forget method not implemented for TransformationLearner");
    
    inherited::forget();
    stage = 0;
    build();
   
    
}

//////////////
// generate //
//////////////

//!generate a point using the training set: 
//! - choose ramdomly a neighbor among data points in the training set
//! - choose randomly a transformation 
//! - apply the transformation on the choosen neighbor
//! - add some noise 
void TransformationLearner::generate(Vec & y) const
{
    //PLERROR("generate not implemented for TransformationLearner");
    PLASSERT(y.length() == inputSpaceDim);
    int neighborIdx ;
    neighborIdx=pickNeighborIdx();
    Vec neighbor;
    neighbor.resize(inputSpaceDim);
    seeNeighbor(neighborIdx, neighbor);
    generatePredictedFrom(neighbor, y);
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
int TransformationLearner::inputsize() const {
    return inputSpaceDim;
}

/////////////////
// log_density //
/////////////////
real TransformationLearner::log_density(const Vec& y) const
{
 
    pout << "in TransformationLearner :: log_density" << endl;
    PLASSERT(y.length() == inputSpaceDim);
    real weight;
    real totalWeight = INIT_weight(0);
    real scalingFactor = -1*(pl_log(pow(2*Pi*noiseVariance, inputSpaceDim/2.0)) 
                             +
                             pl_log(trainingSetLength));
    for(int neighborIdx=0; neighborIdx<trainingSetLength; neighborIdx++){
        for(int transformIdx=0 ; transformIdx<nbTransforms ; transformIdx++){
            weight = computeReconstructionWeight(y,
                                                 neighborIdx,
                                                 transformIdx);
            weight = MULT_weights(weight,
                                  transformDistribution[transformIdx]);
            totalWeight = SUM_weights(weight,totalWeight);
        }  
    }
    totalWeight = MULT_weights(totalWeight, scalingFactor);
    return totalWeight;
}



/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TransformationLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TransformationLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
/*void TransformationLearner::resetGenerator(long g_seed) const
{
    PLERROR("resetGenerator not implemented for TransformationLearner");
}
*/

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void TransformationLearner::train()
{
    
  
    //PLERROR("train method not implemented for TransformationLearner");
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

    if(stage==0)
        buildLearnedParameters();
        initEStep();
    while(stage<nstages)
    {
        MStep();
        EStep();
        stage ++;
    }
    
}



void TransformationLearner::buildLearnedParameters(){
    
    //LEARNED PARAMETERS


    //set of transformations matrices
    transformsSet = Mat(nbTransforms * inputSpaceDim, inputSpaceDim);
    
    //view on the set of transformations (vector)
    //    each transformation = one matrix 
    transforms.resize(nbTransforms);
    for(int k = 0; k< nbTransforms; k++){
        transforms[k] = transformsSet.subMatRows(k * inputSpaceDim, inputSpaceDim);       
    }
    
    //set of transformations bias (optional)
    if(withBias){
        biasSet = Mat(nbTransforms,inputSpaceDim);       
    }
    else{
        biasSet = Mat(0,0);   
    }

    //choose an initial value for each transformation parameter  (normal distribution) 
    initTransformsParameters();

    //initialize the noise variance
    if(noiseVariance == UNDEFINED){
        if(learnNoiseVariance && regOnNoiseVariance){
            initNoiseVariance();
        }
        else{
            noiseVariance = 1.0;
        }
    }

    //transformDistribution
    if(transformDistribution.length() == 0){
        if(learnTransformDistribution && regOnTransformDistribution)
            initTransformDistribution();
        else{
            transformDistribution.resize(nbTransforms);
            real w = INIT_weight(1.0/nbTransforms);
            for(int k=0; k<nbTransforms ; k++){
                transformDistribution[k] = w;
            }
        }       
    }
    else{
        PLASSERT(transformDistribution.length() == nbTransforms);
        PLASSERT(isWellDefined(transformDistribution));
    }


     //reconstruction set 
    reconstructionSet.resize(nbReconstructions);


}


//INITIALIZATION METHODS 


//! initialization operations that have to be done before the training
//!WARNING: the trainset ("train_set") must be given
void TransformationLearner::mainLearnerBuild(){
    int defaultPeriod = 1;
    int defaultTransformsOffset;
    int defaultBiasOffset;
    int defaultNoiseVarianceOffset;
    int defaultTransformDistributionOffset;

    defaultTransformsOffset = 0;
    
    if(withBias){
        defaultBiasOffset = defaultPeriod ;
        defaultPeriod++;
    }
    if(learnNoiseVariance){
        defaultNoiseVarianceOffset = defaultPeriod;
        defaultPeriod++;
    }
    if(learnTransformDistribution){
        defaultTransformDistributionOffset = defaultPeriod;
        defaultPeriod ++;
    }
    
    
    transformsSD = sqrt(transformsVariance);
    
    //DIMENSION VARIABLES
    
    //dimension of the input space
    inputSpaceDim = train_set->inputsize();
      
    //number of samples given in the training set
    trainingSetLength = train_set->length();
    
    
    //number of reconstruction candidates related to a specific target in the 
    //reconstruction set.   
    nbTargetReconstructions = nbNeighbors * nbTransforms;

    //total number of reconstruction candidates in the reconstruction set
    nbReconstructions = trainingSetLength * nbTargetReconstructions;
    
    
    

    if(withBias){
        if(biasPeriod == UNDEFINED || biasOffset == UNDEFINED){
            biasPeriod = defaultPeriod;
            biasOffset = defaultBiasOffset;
        }
    }

    else{
        biasPeriod = UNDEFINED ;
        biasOffset = UNDEFINED;
    }

 

   
    if(transformsPeriod == UNDEFINED || transformsOffset == UNDEFINED){
        transformsPeriod = defaultPeriod;
        transformsOffset = defaultTransformsOffset;
    }

    //training parameters for noise variance
    if(learnNoiseVariance){
        if(noiseVariancePeriod == UNDEFINED || noiseVarianceOffset == UNDEFINED){
            noiseVariancePeriod = defaultPeriod;
            noiseVarianceOffset = defaultNoiseVarianceOffset;
        }
        if(regOnNoiseVariance){
            if(noiseAlpha < 1)
                noiseAlpha = 1;
            if(noiseBeta <= 0){
                noiseBeta = 1;
            }
        }
        else{
            noiseAlpha = NOISE_ALPHA_NO_REG;
            noiseBeta = NOISE_BETA_NO_REG;
        }
    }
    else{
        noiseVariancePeriod = UNDEFINED;
        noiseVarianceOffset = UNDEFINED;
    }
    
 
    
     //training parameters for transformation distribution
     if(learnTransformDistribution){
         if(transformDistributionPeriod == UNDEFINED || transformDistributionOffset == UNDEFINED){
             transformDistributionPeriod = defaultPeriod;
             transformDistributionOffset = defaultTransformDistributionOffset;
         }
         if(regOnTransformDistribution){
             if(transformDistributionAlpha<=0){
                 transformDistributionAlpha =10;
             }
             else{
                 transformDistributionAlpha = TRANSFORM_DISTRIBUTION_ALPHA_NO_REG;
             }
         }
     }
     else{
         transformDistributionPeriod = UNDEFINED;
         transformDistributionOffset = UNDEFINED;
     }


 
   
    
    

    //OTHER VARIABLES
    
    
     
    //Storage space used in the update of the transformation parameters
    B_C = Mat(2 * nbTransforms * inputSpaceDim , inputSpaceDim);
    
    B.resize(nbTransforms);
    C.resize(nbTransforms);
    for(int k=0; k<nbTransforms; k++){
        B[k]= B_C.subMatRows(k*inputSpaceDim, inputSpaceDim);
    }
    for(int k= nbTransforms ; k<2*nbTransforms ; k++){
        C[(k % nbTransforms)] = B_C.subMatRows(k*inputSpaceDim, inputSpaceDim);
    }
    
    
}


//! initialization operations that have to be done before a generation process
//! (all the undefined parameters will be initialized  randomly)
void TransformationLearner::generatorBuild( int inputSpaceDim_,
                                            TVec<Mat> transforms_,
                                            Mat biasSet_,
                                            real noiseVariance_,
                                            Vec transformDistribution_){
    
    inputSpaceDim = inputSpaceDim_;
    transformsSD = sqrt(transformsVariance);
    

    //transformations parameters

    
    transformsSet = Mat(nbTransforms * inputSpaceDim, inputSpaceDim);
    transforms.resize(nbTransforms);
    for(int k = 0; k< nbTransforms; k++){
        transforms[k] = transformsSet.subMatRows(k * inputSpaceDim, inputSpaceDim);       
    }
    
    if(withBias){
        biasSet = Mat(nbTransforms,inputSpaceDim);
    }
    else{
        biasSet = Mat(0,0);
    }
    if(transforms_.length() == 0){
        initTransformsParameters();
    }
    else{
        setTransformsParameters(transforms_,biasSet_);
    }
    

    //noise variance
    if(noiseAlpha < 1){
            noiseAlpha = 1;
        }
    if(noiseBeta <= 0){
        noiseBeta = 1;
    }
    if(noiseVariance_ <= 0){
        initNoiseVariance();
    }
    else{
        setNoiseVariance(noiseVariance_);
    }
    //transformation distribution
    if(transformDistributionAlpha <=0)
        transformDistributionAlpha = 10;
    if(transformDistribution_.length()==0){
        initTransformDistribution();
    }
    else{
        setTransformDistribution(transformDistribution_);
    }
}


//!initializes the transformation parameters randomly 
//!(prior distribution= Normal(0,transformsVariance))
void TransformationLearner::initTransformsParameters()
{
    
    transformsSet .resize(nbTransforms*inputSpaceDim, inputSpaceDim);
    transforms.resize(nbTransforms);
    for(int k = 0; k< nbTransforms; k++){
        transforms[k] = transformsSet.subMatRows(k * inputSpaceDim, inputSpaceDim);       
    }
    for(int t=0; t<nbTransforms ; t++){
        random_gen->fill_random_normal(transforms[t], 0 , transformsSD);
    }
    if(withBias){
        biasSet = Mat(nbTransforms,inputSpaceDim);
        random_gen->fill_random_normal(biasSet, 0,transformsSD);
    }
    else{
        biasSet = Mat(0,0);
    }
    if(transformFamily == TRANSFORM_FAMILY_LINEAR){
        for(int t=0; t<nbTransforms;t++){
            addToDiagonal(transforms[t],1.0);
        }
    }
}

//!initializes the transformation parameters to the given values
//!(bias are set to 0)
void TransformationLearner::setTransformsParameters(TVec<Mat> transforms_,
                                                    Mat biasSet_)
{
    PLASSERT(transforms_.length() == nbTransforms);
    
    int nbRows = inputSpaceDim*nbTransforms;
    transformsSet.resize(nbRows,inputSpaceDim);
    transforms.resize(nbTransforms);
    for(int k = 0; k< nbTransforms; k++){
        transforms[k] = transformsSet.subMatRows(k * inputSpaceDim, inputSpaceDim);       
    }


    int rowIdx = 0;
    for(int t=0; t<nbTransforms; t++){
        PLASSERT(transforms_[t].width() == inputSpaceDim);
        PLASSERT(transforms_[t].length() == inputSpaceDim);
        transformsSet.subMatRows(rowIdx,inputSpaceDim) << transforms_[t];
        transforms[t]= transformsSet.subMatRows(rowIdx,inputSpaceDim);
        rowIdx += inputSpaceDim;
    }
    if(withBias){    
        PLASSERT(biasSet_.length() == nbTransforms);
        PLASSERT(biasSet_.width() == inputSpaceDim);
        biasSet = Mat(nbTransforms, inputSpaceDim);
        biasSet << biasSet_;
    }
    else{
        biasSet = Mat(0,0);
    }
    

}

//!initializes the noise variance randomly
//!(gamma distribution)
void TransformationLearner::initNoiseVariance()
{
    real noisePrecision = gamma_sample(noiseAlpha, noiseBeta);
    PLASSERT(noisePrecision != 0);
    noiseVariance = 1.0/noisePrecision;
}

//!initializes the noise variance with the given value
void TransformationLearner::setNoiseVariance(real nv)
{
    PLASSERT(nv > 0);
    noiseVariance = nv;
}


//!initializes the transformation distribution randomly
//!(dirichlet distribution)
void TransformationLearner::initTransformDistribution()
{
    
    transformDistribution.resize(nbTransforms);
    dirichlet_sample(transformDistributionAlpha, transformDistribution);
    for(int i=0; i<nbTransforms ;i++){
        transformDistribution[i] = INIT_weight(transformDistribution[i]);
    } 
}

//!initializes the transformation distribution with the given values
void TransformationLearner::setTransformDistribution(Vec td)
{
    PLASSERT(td.length() == nbTransforms);
    PLASSERT(isWellDefined(td));
    transformDistribution.resize(nbTransforms);
    transformDistribution << td;
}


//GENERATION

//!generates a sample data point from a source data point
void TransformationLearner::generatePredictedFrom(const Vec & source,
                                                  Vec & sample)const
{
    
    int transformIdx = pickTransformIdx();
    generatePredictedFrom(source, sample, transformIdx);
}

//!generates a sample data point from a source data point with a specific transformation
void TransformationLearner::generatePredictedFrom(const Vec & source,
                                                  Vec & sample,
                                                  int transformIdx)const
{
    //TODO
    real noiseSD = pow(noiseVariance,0.5);
    int d = source.length();
    PLASSERT(d == inputSpaceDim);
    PLASSERT(sample.length() == inputSpaceDim);
    PLASSERT(0<= transformIdx && transformIdx<nbTransforms);
    
    //apply the transformation
    applyTransformationOn(transformIdx,source,sample);
    
    //add noise
    for(int i=0; i<d; i++){
        sample[i] += random_gen->gaussian_mu_sigma(0, noiseSD);
    } 
}

//!generates a sample data point from a source data point and returns it
//! (if transformIdx >= 0 , we use the corresponding transformation )
Vec TransformationLearner::returnPredictedFrom(Vec source,
                                               int transformIdx)const
{
    Vec sample;
    sample.resize(inputSpaceDim);
    if(transformIdx <0)
        generatePredictedFrom(source,sample);
    else
        generatePredictedFrom(source,sample,transformIdx);
    return sample;
}

//!fill the matrix "samples" with data points obtained from a given center data point
void TransformationLearner::batchGeneratePredictedFrom(const Vec & center,
                                                        Mat & samples)const
{
    PLASSERT(center.length() ==inputSpaceDim);
    PLASSERT(samples.width() ==inputSpaceDim);
    int l = samples.length();
    for(int i=0; i<l; i++)
    {
        Vec v = samples(i);
        generatePredictedFrom(center, v);
    }
}

//!fill the matrix "samples" with data points obtained form a given center data point
//!    - we use a specific transformation
void TransformationLearner::batchGeneratePredictedFrom(const Vec & center,
                                                        Mat & samples,
                                                        int transformIdx)const
{
    PLASSERT(center.length() ==inputSpaceDim);
    PLASSERT(samples.width() ==inputSpaceDim);
    int l = samples.length();
    for(int i=0; i<l; i++)
    {
        Vec v = samples(i);
        generatePredictedFrom(center, v,transformIdx);
    }  
}

//Generates n samples from center and returns them stored in a matrix
//    (generation process = 1) choose a transformation (*),
//                          2) apply it on center
//                          3) add noise)
// - (*) if transformIdx>=0, we always use the corresponding transformation
Mat TransformationLearner::returnGeneratedSamplesFrom(Vec center,
                                                      int n,
                                                      int transformIdx)const
{
    Mat samples = Mat(n,inputSpaceDim);
    if(transformIdx<0)
        batchGeneratePredictedFrom(center,samples);
    else
        batchGeneratePredictedFrom(center,samples,transformIdx);
    return samples;
}

//!select a transformation randomly (with respect to our multinomial distribution)
int TransformationLearner::pickTransformIdx() const
{
    
    Vec probaTransformDistribution ;
    probaTransformDistribution.resize(nbTransforms);
    for(int i=0; i<nbTransforms; i++){
        probaTransformDistribution[i]=PROBA_weight(transformDistribution[i]);
    }
    int w= random_gen->multinomial_sample(probaTransformDistribution);
    return w;
}

//!Select a neighbor in the training set randomly
//!(return his index in the training set)
//!We suppose all data points in the training set are equiprobables
int TransformationLearner::pickNeighborIdx() const
{
    
    return random_gen->uniform_multinomial_sample(trainingSetLength);
}


 //!creates a data set:
//!     equivalent in building a tree with fixed deepness and constant branching factor
//!
//!            0      1        2     ...         
//!  
//!            r -> child1  -> child1  ...       
//!                         -> child2  ...
//!                             ...    ...
//!                         -> childn  ...
//!
//!              -> child2  -> child1  ...
//!                         -> child2  ...
//!                              ...   ...
//!                         -> childn  ...
//!                      ...
//!              -> childn  -> child1  ...
//!                         -> child2  ...
//!                              ...   ...
//!                         -> childn  ... 
//!
//!(where "a -> b" stands for "a generate b")
//!all the child are generated by the same following process:
//! 1) choose a transformation  
//! 2) apply the transformation to the parent
//! 3) add noise to the result 
void TransformationLearner::treeDataSet(const Vec & root,
                                        int deepness,
                                        int branchingFactor,
                                        Mat & dataPoints,
                                        int transformIdx)const
{

    PLASSERT(root.length() == inputSpaceDim);

    //we look at the length of the given matrix dataPoint ;
    int nbDataPoints;
    if(branchingFactor == 1)
        nbDataPoints = deepness + 1;  
    else nbDataPoints = int((1- pow(1.0*branchingFactor,deepness + 1.0))
                            /
                            (1 - branchingFactor));
    dataPoints.resize(nbDataPoints,inputSpaceDim);
    
    //root = first element in the matrix dataPoints
    dataPoints(0) << root;
  
    //generate the other data points 
    int centerIdx=0 ;
    for(int dataIdx=1; dataIdx < nbDataPoints ; dataIdx+=branchingFactor){
        
        Vec v = dataPoints(centerIdx);
        Mat m = dataPoints.subMatRows(dataIdx, branchingFactor);
        if(transformIdx>=0){
            batchGeneratePredictedFrom(v,m,transformIdx);
        }
        else{
            batchGeneratePredictedFrom(v,m);
        } 
        centerIdx ++ ;
    }  
}

Mat TransformationLearner::returnTreeDataSet(Vec root,
                                             int deepness,
                                             int branchingFactor,
                                             int transformIdx)const
{
    Mat dataPoints;
    treeDataSet(root,deepness,branchingFactor, dataPoints);
    return dataPoints;
}


//!create a "sequential" dataset:
//!  start -> second point -> third point ... ->nth point
//! (where "->" stands for : "generate the")
void TransformationLearner::sequenceDataSet(const Vec & start,
                                            int n,
                                            Mat & dataPoints,
                                            int transformIdx)const
{
    treeDataSet(start,n-1,1,dataPoints , transformIdx);
}

Mat TransformationLearner::returnSequenceDataSet(Vec start,
                                                 int n,
                                                 int transformIdx)const
{
    Mat dataPoints;
    sequenceDataSet(start,n,dataPoints,transformIdx);
    return dataPoints;
}




//! COPIES OF THE STRUCTURES


//!returns the "idx"th data point in the training set
Vec TransformationLearner::returnTrainingPoint(int idx)const
{
    
    Vec v,temp;
    real w;
    v.resize(inputSpaceDim);
    train_set->getExample(idx, v, temp, w);
    return v;
    
}
 

//!returns all the reconstructions candidates associated to a given target
TVec<ReconstructionCandidate> TransformationLearner::returnReconstructionCandidates(int targetIdx)const
{
   
    int startIdx = targetIdx * nbTargetReconstructions;  
    return reconstructionSet.subVec(startIdx, 
                                    nbTargetReconstructions).copy();
}


//!returns the reconstructions of the "targetIdx"th data point value in the training set
//!(one reconstruction for each reconstruction candidate)
Mat TransformationLearner::returnReconstructions(int targetIdx)const
{
    Mat reconstructions = Mat(nbTargetReconstructions,inputSpaceDim);
    int candidateIdx = targetIdx*nbTargetReconstructions;
    int neighborIdx, transformIdx;
    for(int i=0; i<nbTargetReconstructions; i++){
        neighborIdx = reconstructionSet[candidateIdx].neighborIdx;
        transformIdx= reconstructionSet[candidateIdx].transformIdx;
        Vec neighbor;
        neighbor.resize(inputSpaceDim);
        seeNeighbor(neighborIdx, neighbor);
        Vec v = reconstructions(i);
        applyTransformationOn(transformIdx, neighbor, v);
        candidateIdx ++;
    }
    return reconstructions; 
}

//!returns the neighbors choosen to reconstruct the target
//!(one choosen neighbor for each reconstruction candidate associated to the target)
Mat TransformationLearner::returnNeighbors(int targetIdx)const
{
    int candidateIdx = targetIdx*nbTargetReconstructions;
    int neighborIdx;
    Mat neighbors = Mat(nbTargetReconstructions, inputSpaceDim);
    for(int i=0; i<nbTargetReconstructions; i++){
        neighborIdx = reconstructionSet[candidateIdx].neighborIdx;
        Vec neighbor;
        neighbor.resize(inputSpaceDim);
        seeNeighbor(neighborIdx, neighbor);
        neighbors(i) << neighbor;
        candidateIdx++;
    }
    return neighbors;
}


//!returns the parameters of the "transformIdx"th transformation
Mat TransformationLearner::returnTransform(int transformIdx)const
{
    return transforms[transformIdx].copy();    
}

//!returns the parameters of each transformation
//!(as an KdXd matrix, K = number of transformations,
//!                    d = dimension of input space)
Mat TransformationLearner::returnAllTransforms()const
{
    return transformsSet.copy();    
}


//! VIEWS ON RECONSTRUCTION SET AND TRAINING SET



//! stores a VIEW on the reconstruction candidates related to the specified
//! target (into the variable "targetReconstructionSet" )
void TransformationLearner::seeTargetReconstructionSet(int targetIdx, 
                                                       TVec<ReconstructionCandidate> & targetReconstructionSet)const
{
    int startIdx = targetIdx *nbTargetReconstructions;
    targetReconstructionSet = reconstructionSet.subVec(startIdx, 
                                                       nbTargetReconstructions); 
}

// stores the "targetIdx"th point in the training set into the variable
// "target"
void TransformationLearner::seeTarget(const int targetIdx, Vec & storage)const
{
    Vec v;
    real w;
    train_set->getExample(targetIdx,storage,v,w);
    
}

// stores the "neighborIdx"th input in the training set into the variable
// "neighbor" 
void TransformationLearner::seeNeighbor(const int neighborIdx, Vec & neighbor)const
{
    Vec v;
    real w;
    train_set->getExample(neighborIdx, neighbor,v,w);
}


//! GENERATE GAMMA RANDOM VARIABLES

//!source of the algorithm: http://oldmill.uchicago.edu/~wilder/Code/random/Papers/Marsaglia_00_SMGGV.pdf
    

//!returns a pseudo-random positive real number x  
//!using the distribution p(x)=Gamma(alpha,beta)
real TransformationLearner::gamma_sample(real alpha, real beta)const
{
  real c,x,u,d,v;
  c = 1.0/3.0;
  d = alpha - c ;
  do{
      x = random_gen->gaussian_01();
      u = random_gen->uniform_sample();    
      v = pow((1 + x/(pow(9*d , 0.5)))  ,3.0);
  }
  while(pl_log(u) < 0.5*pow(x,2) + d - d*v + d*pl_log(v));
  return d*v/beta;   
}




//! GENERATE DIRICHLET RANDOM VARIABLES


 //!source of the algorithm: WIKIPEDIA
    

//!returns a pseudo-random positive real vector x 
//!using the distribution p(x) = Dirichlet(x| all the parameters = alpha)
//!-all the element of the vector are between 0 and 1,
//!-the elements of the vector sum to 1
void TransformationLearner::dirichlet_sample(real alpha, Vec & sample)const{
    int d = sample.length();
    real sum = 0;
    for(int i=0;i<d;i++){
        sample[i]=gamma_sample(alpha);
        sum += sample[i];
    }
    for(int i=0;i<d;i++){
        sample[i]/=sum;
    }
}

Vec TransformationLearner::return_dirichlet_sample(real alpha)const
{
    Vec sample ;
    sample.resize(inputSpaceDim);
    dirichlet_sample(alpha, sample);
    return sample;
}



/*void TransformationLearner::dirichlet_sample(const Vec & alphas,
                                        Vec & samples)
{
    //TODO
}
Vec TransformationLearner::return_dirichlet_sample(Vec alphas)
{
    //TODO
    return Vec();
}
*/



//! OPERATIONS ON WEIGHTS


//!normalizes the reconstruction weights related to a given target.
void TransformationLearner::normalizeTargetWeights(int targetIdx,
                                                   real totalWeight)
{
    real w;
    int startIdx = targetIdx * nbTargetReconstructions;
    int endIdx = startIdx + nbTargetReconstructions;
    for(int candidateIdx =startIdx; candidateIdx<endIdx; candidateIdx++){
        w = reconstructionSet[candidateIdx].weight;
        reconstructionSet[candidateIdx].weight =  DIV_weights(w,totalWeight);
    }
}

//!returns a random weight 
real TransformationLearner::randomWeight()const
{  
    real w = random_gen->uniform_sample();
    return INIT_weight((w + minimumProba)/(1.0 + minimumProba));
}

//!arithmetic operations on  reconstruction weights : CONSTRUCTOR
//!proba->weight
real TransformationLearner::INIT_weight(real initValue)const
{
    return pl_log(initValue);
}

//!arithmetic operations on  reconstruction weights :GET CORRESPONDING PROBABILITY 
//! weight->proba
real TransformationLearner::PROBA_weight(real weight)const
{
    return exp(weight); 
}

//!arithmetic operations on  reconstruction weights : DIVISION
//! In our particular case:
//!  numWeight = log(w1)
//!  denomWeight = log(w2)
//! and we want weight = log(w1/w2) = log(w1) - log(w2) 
//!                             = numweight - denomWeight
real TransformationLearner::DIV_weights(real numWeight,
                                        real denomWeight)const
{
    return numWeight - denomWeight;
}


//!arithmetic operations on  reconstruction weights :MULTIPLICATIVE INVERSE
//! weight = log(p)
//!we want : weight' = log(1/p) = log(1) - log(p)
//!                             =     0 -  log(p)
//!                             = -weight
real TransformationLearner::MULT_INVERSE_weight(real weight)const
{
    
    return -1*weight;
}

//!arithmetic operations on  reconstruction weights: MULTIPLICATION
//! weight1 = log(p1)
//! weight2 = log(p2)
//! we want weight3 = log(p1*p2) = log(p1) + log(p2)
//!                              = weight1 + weight2
real TransformationLearner::MULT_weights(real weight1,real weight2)const
{
    
    return weight1 + weight2 ;
}

//!arithmetic operations on  reconstruction weights : SUM 
//! weight1 = log(p1)
//! weight2 = log(p2)
//! we want : weight3 = log(p1 + p2) = logAdd(weight1, weight2)
real TransformationLearner::SUM_weights(real weight1,
                                        real weight2)const
{
    
    return logadd(weight1,weight2);
}



//!update/compute the weight of a reconstruction candidate with
//!the actual transformation parameters
real TransformationLearner::updateReconstructionWeight(int candidateIdx) 
{
    int targetIdx = reconstructionSet[candidateIdx].targetIdx;
    int neighborIdx = reconstructionSet[candidateIdx].neighborIdx;
    int transformIdx = reconstructionSet[candidateIdx].transformIdx;
    
    real w = computeReconstructionWeight(targetIdx,
                                         neighborIdx,
                                         transformIdx);
    reconstructionSet[candidateIdx].weight = w;
    return w; 
}
real TransformationLearner::computeReconstructionWeight(const ReconstructionCandidate & gc)const
{
    return computeReconstructionWeight(gc.targetIdx,
                                       gc.neighborIdx,
                                       gc.transformIdx);
}
real TransformationLearner::computeReconstructionWeight(int targetIdx,
                                                        int neighborIdx,
                                                        int transformIdx)const
{

    Vec target;
    target.resize(inputSpaceDim);
    seeTarget(targetIdx,target);
    return computeReconstructionWeight(target,
                                       neighborIdx,
                                       transformIdx);
}
real TransformationLearner::computeReconstructionWeight(const Vec & target_,
                                                        int neighborIdx,
                                                        int transformIdx)const
{
    Vec neighbor;
    neighbor.resize(inputSpaceDim);
    seeNeighbor(neighborIdx, neighbor);
    Vec predictedTarget ;
    predictedTarget.resize(inputSpaceDim);
    applyTransformationOn(transformIdx, neighbor, predictedTarget);
    real factor = -1/(2*noiseVariance);
    real w = factor*powdistance(target_, predictedTarget);
    return MULT_weights(w, transformDistribution[transformIdx]); 
}

//!applies "transformIdx"th transformation on data point "src"
void TransformationLearner::applyTransformationOn(int transformIdx,
                                                 const Vec & src,
                                                 Vec & dst)const
{
    if(transformFamily==TRANSFORM_FAMILY_LINEAR){
        Mat m  = transforms[transformIdx];
        transposeProduct(dst,m,src); 
        if(withBias){
            dst += biasSet(transformIdx);
        }
    }
    else{ //transformFamily == TRANSFORM_FAMILY_LINEAR_INCREMENT
        Mat m = transforms[transformIdx];
        transposeProduct(dst,m,src);
        dst += src;
        if(withBias){
            dst += biasSet(transformIdx);
        }
    }
}

//! verify if the multinomial distribution given is well-defined
//! i.e. verify that the weights represent probabilities, and that 
//! those probabilities sum to 1 . 
//!(typical case: the distribution is represented as a set of weights, which are typically
//! log-probabilities)
bool  TransformationLearner::isWellDefined(Vec & distribution)const
{  
    if(nbTransforms != distribution.length()){
        return false;
    }
    real sum = 0;
    real proba;
    for(int i=0; i<nbTransforms;i++){
        proba = PROBA_weight(distribution[i]);
        if(proba < 0 || proba >1){
            return false;
        }
        sum += proba;
    }
    return sum == 1;    
}


//! INITIAL E STEP

//!initialization of the reconstruction set
void TransformationLearner::initEStep(){
    if(initializationMode == INIT_MODE_DEFAULT){
        initEStepA();
    }
    else
        initEStepB();
}

//!initialization of the reconstruction set, version B
//!we suppose that all the parameters to learn are already initialized to some value
//1)for each target,
//  a)find the neighbors
//  b)for each neighbor, consider all the possible transformations
//2)compute the weights of all the reconstruction candidates using 
//  the current value of the parameters to learn 
void TransformationLearner::initEStepB(){
    initEStepA();
    smallEStep();    
}


//!initialization of the reconstruction set, version A
//for each target:
//1)find the neighbors (we use euclidean distance as an heuristic)
//2)for each neighbor, assign a random weight to each possible transformation
void TransformationLearner::initEStepA()
{
   
    priority_queue< pair< real,int > > pq = priority_queue< pair< real,int > >();
    
    real totalWeight;
    int candidateIdx=0,targetStartIdx, neighborIdx;
    
    //for each point in the training set i.e. for each target point,
    for(int targetIdx = 0; targetIdx < trainingSetLength ;targetIdx++){
        
        //finds the nearest neighbors and keep them in a priority queue 
        findNearestNeighbors(targetIdx, pq);
        
        //expands those neighbors in the dataset:
        //(i.e. for each neighbor, creates one entry per transformation and 
        //assignsit a positive random weight)
        
        totalWeight = INIT_weight(0);
        targetStartIdx = candidateIdx;
        for(int k = 0; k < nbNeighbors; k++){
            neighborIdx = pq.top().second;
            pq.pop();
            totalWeight =
                SUM_weights(totalWeight,
                            expandTargetNeighborPairInReconstructionSet(targetIdx, 
                                                                        neighborIdx,
                                                                        candidateIdx));
            candidateIdx += nbTransforms;
        }
        //normalizes the  weights of all the entries created for the target 
        //point
        normalizeTargetWeights(targetIdx,totalWeight);
    }

}


//!auxialiary function of "initEStep" . 
//!    for a given pair (target, neighbor), creates all the  
//!    possible reconstruction candidates. 
//!returns the total weight of the reconstruction candidates created
real TransformationLearner::expandTargetNeighborPairInReconstructionSet(int targetIdx,
                                                                        int neighborIdx,
                                                                        int candidateStartIdx)
{
    int candidateIdx = candidateStartIdx;
    real weight, totalWeight = INIT_weight(0);  
    for(int transformIdx=0; transformIdx<nbTransforms; transformIdx ++){
       
        weight = randomWeight(); 
        totalWeight = SUM_weights(totalWeight,weight);
        reconstructionSet[candidateIdx] = ReconstructionCandidate(targetIdx, 
                                                                  neighborIdx,
                                                                  transformIdx,
                                                                  weight);
    
        candidateIdx ++;
    }
    return totalWeight;    
}


//!auxiliary function of initEStep
//!    stores the nearest neighbors for a given target point in a priority
//!    queue.
void TransformationLearner::findNearestNeighbors(int targetIdx,
                                                 priority_queue< pair< real, int > > & pq)
{
    
    //we want an empty queue
    PLASSERT(pq.empty()); 
  
    //capture the target from his index in the training set
    Vec target;
    target.resize(inputSpaceDim);
    seeTarget(targetIdx, target);
    
    //for each potential neighbor,
    real dist;    
    for(int i=0; i<trainingSetLength; i++){
        if(i != targetIdx){ //(the target cannot be his own neighbor)
            //computes the distance to the target
            Vec neighbor;
            neighbor.resize(inputSpaceDim);
            seeNeighbor(i, neighbor);
            dist = powdistance(target, neighbor); 
            //if the distance is among "nbNeighbors" smallest distances seen,
            //keep it until to see a closer neighbor. 
            if(int(pq.size()) < nbNeighbors){
                pq.push(pair<real,int>(dist,i));
            }
            else if (dist < pq.top().first){
                pq.pop();
                pq.push(pair<real,int>(dist,i));
            }
            else if(dist == pq.top().first){
                if(random_gen->uniform_sample() >0.5){
                    pq.pop();
                    pq.push(pair<real,int>(dist,i));
                }
            }
        }
    }    
}

//! ESTEP 

//!coordination of the different kinds of expectation steps
//!  -which are : largeEStepA, largeEStepB, smallEStep
void TransformationLearner::EStep()
{
    if(largeEStepAPeriod > 0  && stage % largeEStepAPeriod == largeEStepAOffset){
        largeEStepA();
    }
    if(largeEStepBPeriod>0 && stage % largeEStepBPeriod == largeEStepBOffset){
        largeEStepB();
    }
    smallEStep(); 
}


//! LARGE E STEP : VERSION A (expectation step)

//!full update of the reconstruction set
//!for each target, keeps the km most probable <neighbor, transformation> 
//!pairs (k = nb neighbors, m= nb transformations)
void TransformationLearner::largeEStepA()
{
    priority_queue< ReconstructionCandidate > pq =  
        priority_queue< ReconstructionCandidate >();
    real totalWeight= INIT_weight(0);
    int candidateIdx=0;
    
    //for each point in the training set i.e. for each target point,
    for(int targetIdx = 0; targetIdx < trainingSetLength ; targetIdx++){
        
        //finds the best weighted triples and keep them in a priority queue 
        findBestTargetReconstructionCandidates(targetIdx, pq);
        //store those triples in the dataset:
        totalWeight = INIT_weight(0);
        for(int k=0; k < nbTargetReconstructions; k++){
            reconstructionSet[candidateIdx] = pq.top(); 
            totalWeight = SUM_weights(pq.top().weight, totalWeight);
            pq.pop();         
            candidateIdx ++;
        }
        
        //normalizes the  weights of all the entries created for the 
        //target point;
        normalizeTargetWeights(targetIdx,totalWeight);
    } 
}


//!auxiliary function of largeEStepA()
//!   for a given target, stores the km most probable (neighbors,
//!   transformation) pairs in a priority queue 
//!   (k = nb neighbors, m = nb transformations)
void TransformationLearner::findBestTargetReconstructionCandidates(int targetIdx,
                                                                   priority_queue< ReconstructionCandidate > & pq)
{
    //we want an empty queue
    PLASSERT(pq.empty()); 
    
    real weight;

    //for each potential neighbor
    for(int neighborIdx=0; neighborIdx<trainingSetLength; neighborIdx++){
        if(neighborIdx != targetIdx){
            for(int transformIdx=0; transformIdx<nbTransforms; transformIdx++){
                weight = computeReconstructionWeight(targetIdx, 
                                                     neighborIdx, 
                                                     transformIdx);
                
                //if the weight is among "nbEntries" biggest weight seen,
                //keep it until to see a bigger neighbor. 
                if(int(pq.size()) < nbTargetReconstructions){
                    pq.push(ReconstructionCandidate(targetIdx,
                                                    neighborIdx,
                                                    transformIdx,
                                                    weight));  
                }
                else if (weight > pq.top().weight){ 
                    pq.pop();
                    pq.push(ReconstructionCandidate(targetIdx,
                                                    neighborIdx,
                                                    transformIdx,
                                                    weight));
                }
                else if (weight == pq.top().weight){
                    if(random_gen->uniform_sample()>0.5){
                        pq.pop();
                        pq.push(ReconstructionCandidate(targetIdx,
                                                        neighborIdx,
                                                        transformIdx,
                                                        weight));
                    }
                }
            }
        }     
    }
}



//! LARGE E STEP : VERSION B (expectation step)


//!full update of the reconstruction set
//!   for each given pair (target, transformation), find the best
//!   weighted neighbors  
void TransformationLearner::largeEStepB()
{
    priority_queue< ReconstructionCandidate > pq;
    
    real totalWeight , weight;
    int candidateIdx=0 ;
    
    //for each point in the training set i.e. for each target point,
    for(int targetIdx =0; targetIdx<trainingSetLength ;targetIdx++){
        
        totalWeight = INIT_weight(0);
        for(int transformIdx=0; transformIdx < nbTransforms; transformIdx ++){
            //finds the best weighted triples   them in a priority queue 
            findBestWeightedNeighbors(targetIdx,transformIdx, pq);
            //store those neighbors in the dataset
            for(int k=0; k<nbNeighbors; k++){
                reconstructionSet[candidateIdx] = pq.top();
                weight = pq.top().weight;
                totalWeight = SUM_weights( weight, totalWeight);
                pq.pop();
                candidateIdx ++;
            }
        }
      //normalizes the  weights of all the entries created for the target 
      //point;
        normalizeTargetWeights(targetIdx,totalWeight);
    }
}
   

//!auxiliary function of largeEStepB()
//!   for a given target x and a given transformation t , stores the best
//!    weighted triples (x, neighbor, t) in a priority queue .
void TransformationLearner::findBestWeightedNeighbors(int targetIdx,
                                                      int transformIdx,
                                                      priority_queue< ReconstructionCandidate > & pq)
{
    //we want an empty queue
    PLASSERT(pq.empty()); 
    
    real weight; 
    
    //for each potential neighbor
    for(int neighborIdx=0; neighborIdx<trainingSetLength; neighborIdx++){
        if(neighborIdx != targetIdx){ //(the target cannot be his own neighbor)
          
            weight = computeReconstructionWeight(targetIdx, 
                                                 neighborIdx, 
                                                 transformIdx);
            //if the weight of the triple is among the "nbNeighbors" biggest 
            //seen,keep it until see a bigger weight. 
            if(int(pq.size()) < nbNeighbors){
                pq.push(ReconstructionCandidate(targetIdx,
                                                neighborIdx, 
                                                transformIdx,
                                                weight));
            }
            else if (weight > pq.top().weight){
                pq.pop();
                pq.push(ReconstructionCandidate(targetIdx,
                                                neighborIdx,
                                                transformIdx,
                                                weight));
            }
            else if (weight == pq.top().weight){
                if(random_gen->uniform_sample() > 0.5){
                    pq.pop();
                    pq.push(ReconstructionCandidate(targetIdx,
                                                    neighborIdx,
                                                    transformIdx,
                                                    weight));
                }
            }
        }
    }   
}



//! SMALL E STEP (expectation step)


//!updating the weights while keeping the candidate neighbor set fixed
void TransformationLearner::smallEStep()
{
    int candidateIdx =0;
    int  targetIdx = reconstructionSet[candidateIdx].targetIdx;
    real totalWeight = INIT_weight(0);
    
    while(candidateIdx < nbReconstructions){
        
        totalWeight = SUM_weights(totalWeight,
                                  updateReconstructionWeight(candidateIdx));
        candidateIdx ++;
    
        if(candidateIdx == nbReconstructions)
            normalizeTargetWeights(targetIdx,totalWeight);
        else if(targetIdx != reconstructionSet[candidateIdx].targetIdx){
            normalizeTargetWeights(targetIdx, totalWeight);
            totalWeight = INIT_weight(0);
            targetIdx = reconstructionSet[candidateIdx].targetIdx;
        }
    }    
}

// M STEP


//!coordination of the different kinds of maximization step
//!(i.e.: we optimize with respect to which parameter?)
void TransformationLearner::MStep()
{
    if(noiseVariancePeriod > 0 && stage%noiseVariancePeriod == noiseVarianceOffset)
        MStepNoiseVariance();
    if(transformDistributionPeriod > 0 && 
       stage % transformDistributionPeriod == transformDistributionOffset)
        MStepTransformDistribution();
    if(biasPeriod > 0 && stage % biasPeriod == biasOffset)
        MStepBias();
    if(stage % transformsPeriod == transformsOffset)
        MStepTransformations();
    
}

//!maximization step  with respect to  transformation distribution
//!parameters
void TransformationLearner::MStepTransformDistribution()
{
    MStepTransformDistributionMAP(transformDistributionAlpha);
}

//!maximization step  with respect to transformation distribution
//!parameters
//!(MAP version, alpha = dirichlet prior distribution parameter)
//!NOTE :  alpha =1 ->  no regularization
void TransformationLearner::MStepTransformDistributionMAP(real alpha)
{
   
    Vec newDistribution ;
    newDistribution.resize(nbTransforms);
    
    for(int k=0; k<nbTransforms ; k++){
        newDistribution[k] = INIT_weight(0);
    }
    
    int transformIdx;
    real weight;
    for(int idx =0 ;idx < nbReconstructions ; idx ++){
        transformIdx = reconstructionSet[idx].transformIdx;
        weight = reconstructionSet[idx].weight;
        newDistribution[transformIdx] = 
            SUM_weights(newDistribution[transformIdx],
                        weight);
    }

    real addFactor = INIT_weight(alpha - 1);
    real divisionFactor = INIT_weight(nbTransforms*(alpha - 1) + trainingSetLength); 

    for(int k=0; k<nbTransforms ; k++){
        newDistribution[k]= DIV_weights(SUM_weights(addFactor,
                                                    newDistribution[k]),
                                        divisionFactor);
    }
    transformDistribution << newDistribution ;
}

//!maximization step with respect to transformation parameters
//!(MAP version)
void TransformationLearner::MStepTransformations()
{
    
    //set the m dXd matrices Ck and Bk , k in{1, ...,m} to 0.
    B_C.clear();
    
    real lambda = 1.0*noiseVariance/transformsVariance;
    Vec v;
    for(int idx=0 ; idx<nbReconstructions ; idx++){
        
        //catch a view on the next entry of our dataset, that is, a  triple:
        //(target_idx, neighbor_idx, transformation_idx)
        
        real p = PROBA_weight(reconstructionSet[idx].weight);
  
        //catch the target and neighbor points from the training set
        Vec target;
        target.resize(inputSpaceDim);
        seeTarget(reconstructionSet[idx].targetIdx, target);
        Vec neighbor;
        neighbor.resize(inputSpaceDim);
        seeNeighbor(reconstructionSet[idx].neighborIdx, neighbor);
        
        int t = reconstructionSet[idx].transformIdx;
        
        v.resize(inputSpaceDim);
        v << target;
        if(transformFamily == TRANSFORM_FAMILY_LINEAR_INCREMENT){
            v = v - neighbor;
        }
        if(withBias){
            v = v - biasSet(t);
        }
        externalProductScaleAcc(C[t], neighbor, neighbor, p);
        
        externalProductScaleAcc(B[t], neighbor, v,p); 
    }
    for(int t=0; t<nbTransforms; t++){
        addToDiagonal(C[t],lambda);
        transforms[t] << solveLinearSystem(C[t], B[t]);  
    }  
}
 

//TODO
void TransformationLearner::MStepBias(){
    
}


//!maximization step with respect to noise variance
void TransformationLearner::MStepNoiseVariance()
{
    MStepNoiseVarianceMAP(noiseAlpha,noiseBeta);
}

//!maximization step with respect to noise variance
//!(MAP version, alpha and beta = gamma prior distribution parameters)
//!NOTE : alpha=1, beta=0 -> no regularization
void TransformationLearner::MStepNoiseVarianceMAP(real alpha, real beta)
{
    
    Vec total_k;
    total_k.resize(nbTransforms);
    int transformIdx;
    real proba;
    for(int idx=0; idx < nbReconstructions; idx++){
        transformIdx = reconstructionSet[idx].transformIdx;
        proba = PROBA_weight(reconstructionSet[idx].weight);
        total_k[transformIdx]+=(proba * reconstructionEuclideanDistance(idx));
    }
    noiseVariance = (2*beta + sum(total_k))/(2*alpha - 2 + trainingSetLength*inputSpaceDim);  
}
 
//!returns the distance between the reconstruction and the target
//!for the 'candidateIdx'th reconstruction candidate
real TransformationLearner::reconstructionEuclideanDistance(int candidateIdx){
    Vec target;
    target.resize(inputSpaceDim);
    seeTarget(reconstructionSet[candidateIdx].targetIdx, target);
    Vec neighbor;
    neighbor.resize(inputSpaceDim);
    seeNeighbor(reconstructionSet[candidateIdx].neighborIdx,
                neighbor);
    Vec reconstruction;
    reconstruction.resize(inputSpaceDim);
    applyTransformationOn(reconstructionSet[candidateIdx].transformIdx,
                          neighbor,
                          reconstruction);
    return powdistance(target, reconstruction);
}


//!increments the variable 'stage' of 1 
void TransformationLearner::nextStage(){
    stage ++;
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
