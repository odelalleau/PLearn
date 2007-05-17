// -*- C++ -*-

// TransformationLearner.cc
//
//version 5
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

//C++ 
#include <math.h>


//Plearn
#include <plearn/math/TMat_maths.h>
#include <plearn/math/PRandom.h>
#include <plearn/math/plapack.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TransformationLearner,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

//TO TEST : OK
TransformationLearner::TransformationLearner():
/* ### Initialize all fields to their default value here */
    seed(1827),
    transformFamily(TRANSFORM_FAMILY_LINEAR),
    noiseVariance(1.0),
    transformsVariance(0.5),
    nbTransforms(5),
    nbNeighbors(5),
    epsilonInitWeight(0.01)
{
    // ...
    //pout << "TransformationLearner()" << endl;
    random_gen = new PRandom();
    

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
}


//TO TEST
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
    
    declareOption(ol, 
                  "seed", 
                  &TransformationLearner::seed, 
                  OptionBase::buildoption,
                  "seed of the random generator");
    declareOption(ol, 
                  "transformFamily", 
                  &TransformationLearner::transformFamily, 
                  OptionBase::buildoption,
                  "transformation function Family");
    declareOption(ol, 
                  "noiseVariance", 
                  &TransformationLearner::noiseVariance, 
                  OptionBase::buildoption,
                  "variance on the noise r.v. (normaly distributed with mean 0)");
    declareOption(ol,
                  "transformsVariance", 
                  &TransformationLearner::transformsVariance, 
                  OptionBase::buildoption,
                  "variance on the transformation parameters r.vs"
                  "(normaly distributed with mean 0)");
    declareOption(ol,
                  "nbTransforms", 
                  &TransformationLearner::nbTransforms, 
                  OptionBase::buildoption,
                  "number of transformations");
    declareOption(ol,
                  "nbNeighbors", 
                  &TransformationLearner::nbNeighbors, 
                  OptionBase::buildoption,
                  "number of neighbors");
    declareOption(ol,
                  "epsilonInitWeight", 
                  &TransformationLearner::epsilonInitWeight, 
                  OptionBase::buildoption,
                  "smallest amount of weight we can give to a choosen \n"
                  "generation candidate at initialization of the \n "
                  "generation set");

    declareOption(ol,
                  "transformDistribution", 
                  &TransformationLearner::transformDistribution, 
                  OptionBase::buildoption,
                  "a multinomial distribution for the transformations\n"
                  "i.e. p(kth transformation) = transformDistribution[k] \n ");

    declareOption(ol,
                  "inputSpaceDim", 
                  &TransformationLearner::inputSpaceDim, 
                  OptionBase::learntoption,
                  "dimension of the training set input space");
    declareOption(ol,
                  "nbGenerationCandidatesPerTarget", 
                  &TransformationLearner::nbGenerationCandidatesPerTarget, 
                  OptionBase::learntoption,
                  "number of generation candidates per target"); 
    declareOption(ol,
                  "nbGenerationCandidates", 
                  &TransformationLearner::nbGenerationCandidates, 
                  OptionBase::learntoption,
                  "number of generation candidates in the generation set");
    declareOption(ol,
                  "nbTrainingInput", 
                  &TransformationLearner::nbTrainingInput, 
                  OptionBase::learntoption,
                  "number of input given in the training set");  
    declareOption(ol,
                  "trainsformsSet", 
                  &TransformationLearner::transformsSet, 
                  OptionBase::learntoption,
                  "set of transformations");
    declareOption(ol,
                  "transforms", 
                  &TransformationLearner::transforms, 
                  OptionBase::learntoption,
                  "views on the transformation set");
    
    declareOption(ol,
                  "generationSet", 
                  &TransformationLearner::generationSet, 
                  OptionBase::learntoption,
                  "set of generation candidates"
                  "i.e. triples (target, neighbor, transformation)");
    declareOption(ol,
                  "lambda", 
                  &TransformationLearner::lambda, 
                  OptionBase::learntoption,
                  "weight decay");

    declareOption(ol,
                  "noiseVarianceFactor", 
                  &TransformationLearner::noiseVarianceFactor, 
                  OptionBase::learntoption,
                  "factor used in computation of generation weights"
                  " 1/(2*noise variance)");

    declareOption(ol,
                  "noiseStDev", 
                  &TransformationLearner::noiseStDev, 
                  OptionBase::learntoption,
                  "standard deviation on noise distribution");
    declareOption(ol,
                  "transformsStDev", 
                  &TransformationLearner::transformsStDev, 
                  OptionBase::learntoption,
                  "standard deviation on transformation parameters");

    // Now call the parent class' declareOptions
    


    inherited::declareOptions(ol);
}


//TO TEST
void TransformationLearner::declareMethods(RemoteMethodMap& rmm)
{
    declareMethod(rmm, "largeEStepA", &TransformationLearner::largeEStepA,
                  (BodyDoc("Performs a large update of the generation set (expectation step)"  
                           "For each target, we take the best generation candidates among all the possibilities \n")));
    
    declareMethod(rmm, "initEStep", &TransformationLearner::initEStep,
                  (BodyDoc("Initialization of the generation set (expectation step)\n" 
                           "For each possible couple (target,neighbor), we take the best transformations to form \n" 
                           "the generation candidates")));
    
    declareMethod(rmm, "smallEStep",&TransformationLearner::smallEStep,
                  (BodyDoc("Update of the generation set (expectation step) \n"
                           "we update the weights of the generation candidates while keeping them fixed")));
    
    declareMethod(rmm, "MStep", &TransformationLearner::MStep,
                  (BodyDoc("Updating the transformation parameters (maximization step)\n")));

    declareMethod(rmm, "largeEStepB", &TransformationLearner::largeEStepB,
                  (BodyDoc("Performs a large update of the")));
 
    declareMethod(rmm, "returnReproductionSources", &TransformationLearner::returnReproductionSources,
                  (BodyDoc("Returns the generation candidates associated to a specific target "),
                   ArgDoc ("targetIdx", "Index of the target data point in the training set"),
                   RetDoc ("A vector of tuples (target index, neighbor index, transformation index, weight )")));

    declareMethod(rmm, "returnReproductions", &TransformationLearner::returnReproductions,
                  (BodyDoc("Computes the reproductions of the target from his generation candidates "),
                   ArgDoc("targetIdx","Index of the target data point in the training set"),
                   RetDoc("A matrix of data points (reproductions of the target)")));

    declareMethod(rmm, "returnTransform", &TransformationLearner::returnTransform,
                  (BodyDoc("Returns the parameters of a transformation"),
                   ArgDoc("transformIdx"," Index of the transformation"),
                   RetDoc("a dXd matrix, d = dimension of input space")));
    declareMethod(rmm, "returnAllTransforms",&TransformationLearner::returnAllTransforms,
                  (BodyDoc("Returns all the transformation parameters"),
                   RetDoc("a kdXd matrix,  k = nb transformations \n" 
                          "                d = dimension of input space")));
    declareMethod(rmm, 
                  "returnGeneratedSamplesFrom", 
                  &TransformationLearner::returnGeneratedSamplesFrom,
                  (BodyDoc("returns samples data point generated from\n"
                           "a center data point"),
                   ArgDoc("Vec center, int n"," center data point"),
                   ArgDoc("int n", " number of sample data points to generate"),
                   RetDoc("a nXd matrix, the center is included in the dataset")));
    declareMethod(rmm,
                  "returnGeneratedDataSet",
                  &TransformationLearner::returnGeneratedDataSet,
                  (BodyDoc("returns a data set with respect to the current\n"
                           "distribution paramaters\n"
                           "We use a tree generation process (see createDataSet)\n"
                           "i.e.: each new data point is used to generate a fix number of data points "),
                   ArgDoc("Vec root","initial data point"),
                   ArgDoc("int nbGenerations","deepness of the tree"),
                   ArgDoc("int GenerationLen","number of child for interior nodes"),
                   RetDoc("a nXd matrix, where n = number of nodes in the tree (root = part of the dataSet)")));
    
    declareMethod(rmm,
                  "returnSequentiallyGeneratedDataSet",
                  &TransformationLearner::returnSequentiallyGeneratedDataSet,
                  (BodyDoc("returns a data set with respect to the current\n"
                           "distribution parameters\n"
                           "We use a sequential generation process\n"
                           "i.e: each new data point is used to generate the next data point"),
                   ArgDoc("Vec root","initial data point"),
                   ArgDoc("int n","number of data points to generate"),  
                   RetDoc("a nXd matrix (the root is included in the data set)")));
    
    inherited::declareMethods(rmm);
}



//TO TEST
//do the building operations related to the generation process
//warning: we suppose the transformation parameters are set 
void TransformationLearner::generatorInit(){
    
    inputSpaceDim = transformsSet.width();
    

}


//TO TEST
//do the building operations related to training
//warning: we suppose the training set has been transmitted
//         before calling the method
void TransformationLearner::trainInit(){
   
    //DIMENSION VARIABLES
    
    //dimension of the input space
    inputSpaceDim = train_set->inputsize();
      
    //number of samples given in the training set
    nbTrainingInput = train_set->length();

    
    //number of generation candidates related to a specific target in the 
    //generation set.   
    nbGenerationCandidatesPerTarget = nbNeighbors * nbTransforms;

   //total number of generation candidates in the generation set
    nbGenerationCandidates = nbTrainingInput * nbGenerationCandidatesPerTarget;

    
    //LEARNED MODEL PARAMETERS
    
    //set of transformations (represented as a single matrix)
    transformsSet = Mat(nbTransforms * inputSpaceDim, inputSpaceDim);
    
    //view on the set of transformations (vector)
    //    each transformation = one matrix 
    transforms.resize(nbTransforms);
    for(int k = 0; k< nbTransforms; k++){
        transforms[k] = transformsSet.subMatRows(k * inputSpaceDim, inputSpaceDim);       
    }
    
     //generation set and weights of the entries in the generation set
    generationSet.resize(nbGenerationCandidates);

    //OTHER VARIABLES
    
    //weight decay
    lambda = noiseVariance/transformsVariance;
   
    

    //factor used in the computation of the generation weights
    noiseVarianceFactor = 1/(2*noiseVariance);
    

    //to store a view on the generation set 
    //   (entries related to a specific target)
    targetGenerationSet.resize(nbGenerationCandidatesPerTarget);

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
    
    
    target.resize(inputSpaceDim);
    neighbor.resize(inputSpaceDim);

}


//TO TEST 
void TransformationLearner::build_(){

  
    if(transformDistribution.length() == 0){
        transformDistribution.resize(nbTransforms);
        transformDistribution.fill(1.0/nbTransforms);
    }
    else{
        PLASSERT(transformDistribution.length() == nbTransforms);
        real sum =0;
        for(int i=0; i<nbTransforms; i++){
            sum += transformDistribution[i];
        }
        PLASSERT(sum == 1);  
    }
    
   

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
}

//TO TEST
// ### Nothing to add here, simply calls build_
void TransformationLearner::build()
{

    // pout << "build()" << endl;
    inherited::build();
    build_(); 
}


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

/******** LEARNING MODULE ******************************************************/


//TO DO
int TransformationLearner::outputsize() const
{
    return 0;
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
}


//TO DO
void TransformationLearner::forget()
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
}

//TO DO
void TransformationLearner::train()
{

    trainInit();

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
}

//TO DO
void TransformationLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
}

//TO DO
void TransformationLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output.
// ...
}

//TO DO
TVec<string> TransformationLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ..
    
    return TVec<string>();
}

//TO DO
TVec<string> TransformationLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    // ...
    return TVec<string>();
}

 /*********GENERATION MODULE *****************************************/

    //The object is the representation of a learned distribution
    //Are are methods to ensure the "generative behavior" of the object
    //(Once the distribution is learned, we might be able to generate
    // samples from it)



// TO TEST
//Chooses the transformation parameters using a 
//normal distribution with mean 0 and variance "transformsVariance"
//(call generatorBuild() after)
void TransformationLearner::buildTransformationParametersNormal(){
    transformsSet.resize(nbTransforms*inputSpaceDim, inputSpaceDim);
    transforms.resize(nbTransforms);
    for(int t=0; t<nbTransforms ; t++){
        random_gen->fill_random_normal(transforms[t], 0 , transformsStDev);
    }
}


//TO TEST
//set the transformation parameters to the specified values
//(call generatorBuild() after)
void TransformationLearner::setTransformationParameters(TVec<Mat> & transforms_){
     
    PLASSERT(transforms_.length() == nbTransforms);
    inputSpaceDim = transforms_[0].width();
    
    int nbRows = inputSpaceDim*nbTransforms;
    transformsSet = Mat(nbRows,inputSpaceDim);
    transforms.resize(nbTransforms);
  
    int rowIdx = 0;
    for(int t=0; t<nbTransforms; t++){
        transformsSet.subMatRows(rowIdx,inputSpaceDim) << transforms_[t];
        transforms[t]= transformsSet.subMatRows(rowIdx,inputSpaceDim);
        rowIdx += inputSpaceDim;
    }
}

//TO TEST
//creates a data set
//
//     Consists in building a tree of deepness d = "nbGenerations" and
//     constant branch factor n = "generationLength"
//
//            0      1        2     ...         
//  
//            r - child1  - child1  ...       
//                        - child2  ...
//                            ...   ...
//                        - childn  ...
//
//              - child2  - child1  ...
//                        - child2  ...
//                            ...   ...
//                        - childn  ...
//                     ...
//             - childn   - child1  ...
//                        - child2  ...
//                            ...   ...
//                        - childn  ... 
//

// all the childs are choosen following the same process:
// 1) choose a transformation  
// 2) apply the transformation to the parent
// 3) add noise to the result 
void TransformationLearner::createDataSet(Vec & root,
                                          int nbGenerations,
                                          int generationLength,
                                          Mat & dataPoints){
   
    PLASSERT(root.length() == inputSpaceDim);
 
    //we look at the length of the given matrix dataPoint ;.  
    int nbDataPoints = int(pow(1.0*nbGenerations,1.0*generationLength)) + 1;
    dataPoints.resize(nbDataPoints,inputSpaceDim);
    
    //root = first element in the matrix dataPoints
    dataPoints(0) << root;
  
    //generate the other data points 
    int centerIdx=0 ;
    for(int dataIdx=1; dataIdx < nbDataPoints ; dataIdx+=generationLength){

        Vec v = dataPoints(centerIdx);
        Mat m = dataPoints.subMatRows(dataIdx, generationLength);
        batchGenerateFrom(v,m); 
        centerIdx ++ ;
    }
}


//TO TEST
//create a dataset using the same tree generation process as
//createDataSet, except the number of child per parent is fixed to 1,
//   root -> 1st point -> 2nd point ... -> nth point 
void TransformationLearner::createDataSetSequentially(Vec & root,
                                                      int n,
                                                      Mat & dataPoints){
    createDataSet(root, n-1, 1, dataPoints);
}


//TO TEST
//Select a transformation randomly (with respect ot our transformation
//distribution)
int TransformationLearner::pickTransformIdx(){
     return random_gen->multinomial_sample(transformDistribution);
}

  
//here is the generation process for a given center data point 
//  1) choose a transformation
//  2) apply it on the center data point
//  3) add noise

//TO TEST
//generates a sample data point  from a  given center data point 
void  TransformationLearner::generateFrom(Vec & center, Vec & sample){
    int transformIdx = pickTransformIdx();
    generateFrom(center, sample, transformIdx);
}

//TO TEST
//generates a sample data point from a given center data point
void TransformationLearner::generateFrom(Vec & center,
                                         Vec & sample, 
                                         int transformIdx){
    int d = center.length();
    PLASSERT(d == inputSpaceDim);
    
    sample.resize(inputSpaceDim);
    
    //apply the transformation
    applyTransformationOn(transformIdx,center,sample);
    
    //add noise
    for(int i=0; i<d; i++){
        sample[i] += random_gen->gaussian_mu_sigma(0, noiseStDev);
    } 
}

//TO TEST
//fill the matrix "samples" with sample data points obtained from
// a given center data point.
void TransformationLearner::batchGenerateFrom(Vec & center, Mat & samples){

    PLASSERT(center.length() ==inputSpaceDim);
    PLASSERT(samples.width()==inputSpaceDim);
    int l = samples.length();
    for(int i=0; i<l; i++)
    {
        Vec v = samples(i);
        generateFrom(center, v);
    }
}



//-------------EXTERNAL ACCESS ---------------------------

//TO TEST
//Returns a copy of the generation candidates associated to a given target
TVec<GenerationCandidate> TransformationLearner::returnReproductionSources
(int targetIdx){
    int startIdx = targetIdx * nbGenerationCandidatesPerTarget;
    int endIdx = startIdx + nbGenerationCandidatesPerTarget;
    return generationSet.subVec(startIdx, endIdx).copy();
}

//TO TEST
//Returns the parameters of a given transformation
Mat TransformationLearner::returnTransform(int transformIdx){
    return transforms[transformIdx].copy();   
}

//TO TEST
//Returns all the transformation parameters
Mat TransformationLearner::returnAllTransforms(){
    return transformsSet;
}


//TO TEST
//From the subset ofgeneration candidate associated to the target,
//builds and returns the corresponding subset of generated data points . 
Mat TransformationLearner::returnReproductions(int targetIdx){
    Mat reproductions = Mat(nbGenerationCandidatesPerTarget,inputSpaceDim);
    int candidateIdx = targetIdx*nbGenerationCandidatesPerTarget;
    int neighborIdx, transformIdx;
    for(int i=0; i<nbGenerationCandidatesPerTarget; i++){
        neighborIdx = generationSet[candidateIdx].neighborIdx;
        transformIdx= generationSet[candidateIdx].transformIdx;
        getNeighborFromTrainingSet(neighborIdx);
        Vec v = reproductions(i);
        applyTransformationOn(transformIdx, neighbor, v);
        candidateIdx ++;
    }
    return reproductions;
}


//TO TEST
//Generates n samples from center and returns them
//    (generation process = 1) choose a transformation,
//                          2) apply it on center
//                          3) add noise)
Mat TransformationLearner::returnGeneratedSamplesFrom(Vec center, int n){
    int d = center.length();
    PLASSERT(d == inputSpaceDim);
    Mat m = Mat(n,d);
    batchGenerateFrom(center, m);
    return m;
}

//TO TEST
//Generates a data set and returns it
//(tree generation process: see createDataSet for more details)
Mat TransformationLearner::returnGeneratedDataSet(Vec root,
                                                  int nbGenerations,
                                                  int generationLength){
 
    int n = int(pow(1.0*nbGenerations, 1.0*generationLength)) + 1;
    int d = root.length();
    PLASSERT(d == inputSpaceDim);

    Mat dataSet = Mat(n,d);
    createDataSet(root,nbGenerations,generationLength,dataSet);
    return dataSet;
}

//TO TEST
//Generates a data set and returns it
//(sequential generation process: see createDataSetSequentially for more details)
Mat TransformationLearner::returnSequentiallyGeneratedDataSet(Vec root,int n){ 
    return returnGeneratedDataSet(root, n-1,1);
}


// ----------GENERAL USE--------------------------------------------------



//REFERENCE OPERATIONS ON GENERATION SET AND TRAINING SET  


//TO TEST
// stores a view on the subset of generation set related to the specified
// target (into the variable "targetGenerationSet" )
void TransformationLearner::getViewOnTargetGenerationCandidates(int targetIdx){
    int startIdx = targetIdx * nbGenerationCandidatesPerTarget;
    int endIdx = startIdx + nbGenerationCandidatesPerTarget;
    targetGenerationSet = generationSet.subVec(startIdx, 
                                               endIdx);
    
}





// stores the "targetIdx"th input in the training set into the variable
// "target"
void TransformationLearner::getTargetFromTrainingSet(int targetIdx){
    Vec v;
    real w;
    train_set->getExample(targetIdx,target,v,w);

    //TO TEST : OK
}

// stores the "neighborIdx"th input in the training set into the variable
// "neighbor" 
void TransformationLearner::getNeighborFromTrainingSet(int neighborIdx){
    Vec v;
    real w;
    train_set->getExample(neighborIdx,neighbor,v,w);
    
    //TO TEST : OK
}


//OPERATIONS RELATED TO GENERATION WEIGHTS

//normalizes the generation weights related to a given target. 
void TransformationLearner::normalizeTargetGenerationWeights(int targetIdx, 
                                                             real totalWeight){
    real w;
    int startIdx = targetIdx * nbGenerationCandidatesPerTarget;
    int endIdx = startIdx + nbGenerationCandidatesPerTarget;
    for(int candidateIdx =startIdx; candidateIdx<endIdx; candidateIdx++){
        w = generationSet[candidateIdx].weight;
        generationSet[candidateIdx].weight =  DIV_weights(w,totalWeight);
    }

    //TO TEST : OK
}
    
//returns a random positive weight 
real TransformationLearner::randomPositiveGenerationWeight(){
    return  random_gen->uniform_sample() + epsilonInitWeight;

    //TO TEST : OK
}
  

//arithmetic operations on  generation weights
real TransformationLearner::DIV_weights(real numWeight,    //DIVISION
                                        real denomWeight){ 
    return numWeight/denomWeight;

    //TO TEST : OK
}
real TransformationLearner::MULT_INVERSE_weight(real weight){//MULTIPLICATIVE INVERSE
    return -weight;

    //TO TEST : OK
}
real TransformationLearner::MULT_weights(real weight1, real weight2){ //MULTIPLICATION
    return weight1*weight2;
    
    //TO TEST : OK
}
real TransformationLearner::SUM_weights(real weight1, real weight2){ //SUM
    return weight1 + weight2;

    //TO TEST : OK
} 

//TO TEST
//update/compute the weight of a generation candidate with
//the actual transformation parameters
real TransformationLearner::updateGenerationWeight(int candidateIdx){
    
    GenerationCandidate * gc = & generationSet[candidateIdx];
    
    real w = computeGenerationWeight(gc->targetIdx,
                                     gc->neighborIdx,
                                     gc->transformIdx);
    pout << "weigth:"<< w <<endl;
    gc->weight = w;
    return w;
}

//TO TEST
real TransformationLearner::computeGenerationWeight(GenerationCandidate & gc){
    return computeGenerationWeight(gc.targetIdx,
                                   gc.neighborIdx,
                                   gc.transformIdx);

   
}

//TO TEST
real TransformationLearner::computeGenerationWeight(int targetIdx, 
                                                    int neighborIdx, 
                                                    int transformIdx){
  

    getTargetFromTrainingSet(targetIdx);
    getNeighborFromTrainingSet(neighborIdx);
    Vec predictedTarget ;
    predictedTarget.resize(inputSpaceDim);
    applyTransformationOn(transformIdx, neighbor, predictedTarget);
    return exp(noiseVarianceFactor * powdistance(target, predictedTarget));

    
}


//applies "transformIdx"th transformation on data point "src"
void TransformationLearner::applyTransformationOn(int transformIdx, Vec & src, Vec &  dst){
    if(transformFamily == TRANSFORM_FAMILY_LINEAR){
        Mat m  = transforms[transformIdx];
        product(dst,m,src);
    }
    else{
        Mat m = transforms[transformIdx];
        product(dst, m, src);
        dst += src;
    }
}

//-------- INITIAL E STEP --------------------------------------------

//initialization of the generation set 
void TransformationLearner::initEStep(){
    priority_queue< pair< real,int > > pq = priority_queue< pair< real,int > >();
    
    real totalWeight;
    int candidateIdx=0,targetStartIdx, neighborIdx;
    
    //for each point in the training set i.e. for each target point,
    for(int targetIdx = 0; targetIdx < nbTrainingInput ;targetIdx++){
    
        //finds the nearest neighbors and keep them in a priority queue 
        findNearestNeighbors(targetIdx, pq);
        
        //expands those neighbors in the dataset:
        //(i.e. for each neighbor, creates one entry per transformation and 
        //assignsit a positive random weight)
        
        totalWeight =0;
        targetStartIdx = candidateIdx;
        for(int k = 0; k < nbNeighbors; k++){
            neighborIdx = pq.top().second;
            pq.pop();
            totalWeight =SUM_weights(expandTargetNeighborPairInGenerationSet(targetIdx, 
                                                                             neighborIdx,
                                                                             candidateIdx),
                                     totalWeight);
            candidateIdx += nbTransforms;
        }
        //normalizes the  weights of all the entries created for the target 
        //point
        normalizeTargetGenerationWeights(targetIdx,totalWeight);
    }

    //TO TEST
}
    
//auxialiary function of "initEStep" . 
//    for a given pair (target, neighbor), creates all the associated 
//    generation candidates (entries) in the data set. 
//returns the total weight of the generation candidates created
real TransformationLearner::expandTargetNeighborPairInGenerationSet(int targetIdx,
                                                                    int neighborIdx,
                                                                    int candidateIdx){
    real weight, totalWeight = 0;  
    for(int transformIdx=0; transformIdx<nbTransforms; transformIdx ++){
        //choose a random positive weight
        weight = randomPositiveGenerationWeight(); 
        totalWeight = SUM_weights(weight,totalWeight);
        generationSet[candidateIdx] = GenerationCandidate(targetIdx, 
                                                          neighborIdx,
                                                          transformIdx,
                                                          weight);
    
        candidateIdx ++;
    }
    return totalWeight;    

    //TO TEST
}

//auxiliary function of initEStep
//    keeps the nearest neighbors for a given target point in a priority
//    queue.
void TransformationLearner::findNearestNeighbors (int targetIdx,
                                                  priority_queue< pair< real, int > > & pq){
    
    //we want an empty queue
    PLASSERT(pq.empty()); 
  
    //capture the target from his index in the training set
    getTargetFromTrainingSet(targetIdx);
     
    //for each potential neighbor,
    real dist;    
    for(int i=0; i<nbTrainingInput; i++){
        if(i != targetIdx){ //(the target cannot be his own neighbor)
            //computes the distance to the target
            getNeighborFromTrainingSet(i);
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
        }
    }    

    //TO TEST
}


//-------- LARGE E STEP : VERSION A --------------------------------

//full update of the generation set
//for each target, keeps the top km most probable <neighbor, transformation> 
//pairs (k = nb neighbors, m= nb transformations)
void TransformationLearner::largeEStepA(){
    
    priority_queue< GenerationCandidate > pq =  priority_queue< GenerationCandidate >();
    real totalWeight=0;
    int candidateIdx=0;
    
    //for each point in the training set i.e. for each target point,
    for(int targetIdx = 0; targetIdx < nbTrainingInput ; targetIdx++){
        
        //finds the best weighted triples and keep them in a priority queue 
        findBestTargetCandidates(targetIdx, pq);
        
        //store those triples in the dataset:
        totalWeight = 0;
        for(int k=0; k < nbGenerationCandidatesPerTarget; k++){
            generationSet[candidateIdx] = pq.top(); 
            totalWeight = SUM_weights(pq.top().weight, totalWeight);
            pq.pop();         
            candidateIdx ++;
        }
        
        //normalizes the  weights of all the entries created for the 
        //target point;
        normalizeTargetGenerationWeights(targetIdx,totalWeight);
    }

    //TO TEST
}

//auxiliary function of largeEStepA()
//   for a given target, keeps the top km most probable neighbors,
//   transformation pairs in a priority queue 
//   (k = nb neighbors, m = nb transformations)
void  TransformationLearner::findBestTargetCandidates
(int targetIdx,
 priority_queue< GenerationCandidate > & pq){
    
    //we want an empty queue
    PLASSERT(pq.empty()); 
    real weight;

    //for each potential neighbor
    for(int neighborIdx=0; neighborIdx<nbTrainingInput; neighborIdx++){
        if(neighborIdx != targetIdx){
            for(int transformIdx=0; transformIdx<nbTransforms; transformIdx++){
                weight = computeGenerationWeight(targetIdx, 
                                                 neighborIdx, 
                                                 transformIdx);
                
                //if the weight is among "nbEntries" biggest weight seen,
                //keep it until to see a bigger neighbor. 
                if(int(pq.size()) < nbGenerationCandidatesPerTarget){
                    pq.push(GenerationCandidate(targetIdx,
                                                neighborIdx,
                                                transformIdx,
                                                weight));  
                }
                else if (weight > pq.top().weight){
                    pq.pop();
                    pq.push(GenerationCandidate(targetIdx,
                                                neighborIdx,
                                                transformIdx,
                                                weight));
                }
            }
        }     
    }

    //TO TEST
}


//-------- LARGE E STEP : VERSION B --------------------------------

//full update of the generation set
//   for each given pair (target, transformation), find the best
//   weighted neighbors  
void  TransformationLearner::largeEStepB(){
    priority_queue< GenerationCandidate > pq;
    
  real totalWeight=0 , weight;
  int candidateIdx=0 ;
  
  //for each point in the training set i.e. for each target point,
  for(int targetIdx =0; targetIdx<nbTrainingInput ;targetIdx++){
  
      totalWeight = 0;
      for(int transformIdx=0; transformIdx < nbTransforms; transformIdx ++){
          //finds the best weighted triples   them in a priority queue 
          findBestWeightedNeighbors(targetIdx,transformIdx, pq);
          
          //store those neighbors in the dataset
          for(int k=0; k<nbNeighbors; k++){
              generationSet[candidateIdx] = pq.top();
              weight = pq.top().weight;
              totalWeight = SUM_weights( weight, totalWeight);
              pq.pop();
              candidateIdx ++;
          }
      }
      //normalizes the  weights of all the entries created for the target 
      //point;
      normalizeTargetGenerationWeights(targetIdx,totalWeight);
  }

  // TO TEST
}

    
//auxiliary function of largeEStepB()
//   for a given target x and a given transformationt , keeps the best
//   weighted triples (x, neighbor, t) in a priority queue .
void  TransformationLearner::findBestWeightedNeighbors
(int targetIdx,
 int transformIdx,
 priority_queue< GenerationCandidate > & pq){
 
    //we want an empty queue
    PLASSERT(pq.empty()); 

    real weight; 
    
    //for each potential neighbor
    for(int neighborIdx=0; neighborIdx<nbTrainingInput; neighborIdx++){
        if(neighborIdx != targetIdx){ //(the target cannot be his own neighbor)
            
	  weight=  computeGenerationWeight(targetIdx, 
                                           neighborIdx, 
                                           transformIdx);
	  //if the weight of the triple is among the "nbNeighbors" biggest 
	  //seen,keep it until see a bigger weight. 
            if(int(pq.size()) < nbNeighbors){
                pq.push(GenerationCandidate(targetIdx,
                                            neighborIdx, 
                                            transformIdx,
                                            weight));
            }
            else if (weight >  pq.top().weight){
                pq.pop();
                pq.push(GenerationCandidate(targetIdx,
                                            neighborIdx,
                                            transformIdx,
                                            weight));
            }
        }
    } 

    //TO TEST
}


//-------- SMALL E STEP --------------------------------------------- 


//updating the weights while keeping the candidate neighbor set fixed
void TransformationLearner::smallEStep(){
    
    int candidateIdx =0, startCandidateIdx=0;
    int startTargetIdx = generationSet[startCandidateIdx].targetIdx;
    int  targetIdx;
    real totalWeight = 0;
  
    while(candidateIdx < nbGenerationCandidates){
    
        totalWeight = SUM_weights(updateGenerationWeight(candidateIdx),
                                  totalWeight);
        candidateIdx ++;
    
        targetIdx = generationSet[candidateIdx].targetIdx; 
    
        if(candidateIdx > nbGenerationCandidates || targetIdx != startTargetIdx){
            normalizeTargetGenerationWeights(startTargetIdx, totalWeight);
            totalWeight = 0;
            startTargetIdx = targetIdx;
        }
    }    

    //TO TEST
}
    

//-------- M STEP ---------------------------------------------   
    

//updating the transformation parameters
void TransformationLearner::MStep(){
    //set the m dXd matrices Ck and Bk , k in{1, ...,m} to 0.
    B_C.clear();
  
    for(int idx=0 ; idx<nbGenerationCandidates ; idx++){
    
        //catch a view on the next entry of our dataset, that is, a  triple:
        //(target_idx, neighbor_idx, transformation_idx)
        GenerationCandidate * gc = &generationSet[idx];
        
        real w = gc->weight;
  
        //catch the target and neighbor points from the training set
        getTargetFromTrainingSet(gc->targetIdx);
        getNeighborFromTrainingSet(gc->neighborIdx);
        
        int t = gc->transformIdx;
        
        externalProductScaleAcc(C[t], target, target, w);
        if(transformFamily == TRANSFORM_FAMILY_LINEAR){
            externalProductScaleAcc(B[t], target, neighbor,w);
        }
        else
            externalProductScaleAcc(B[t], target, (target - neighbor), w); 
    }
    for(int t=0; t<nbTransforms; t++){
        addToDiagonal(C[t],lambda);
        transforms[t] << solveLinearSystem(C[t], B[t]);
    }

    //TO TEST

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
