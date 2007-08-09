// -*- C++ -*-

// TransformationLearner.h
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

// Authors: Lysiane Bouchard, Pascal Vincent

/*! \file TransformationLearner.h */


#ifndef TransformationLearner_INC
#define TransformationLearner_INC

//PLEARN
#include <plearn_learners/distributions/PDistribution.h> 
#include <plearn/math/plapack.h>
#include <plearn/math/pl_math.h>
#include <plearn_learners/distributions/PDistribution.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/PRandom.h>
#include <plearn/base/tuple.h>

//C++
#include <utility>
#include <queue>
#include <math.h>

#define TRANSFORM_FAMILY_LINEAR 0
#define TRANSFORM_FAMILY_LINEAR_INCREMENT 1
#define UNDEFINED -1
#define INIT_MODE_DEFAULT 0
#define INIT_MODE_RANDOM 1
#define NOISE_ALPHA_NO_REG 1
#define NOISE_BETA_NO_REG 0
#define TRANSFORM_DISTRIBUTION_ALPHA_NO_REG 1
#define BEHAVIOR_LEARNER 0
#define BEHAVIOR_GENERATOR 1

namespace PLearn {



/**
description of the main class: TransformationLearner

GENERATION PROCESS

We suppose a new point v is obtained from a point x by :
1) choosing a transformation t among a set of transformations T
  (with probability p(t))
2) applying the choosen transformation t on x
3) add some noise in all the directions (noise = normally distributed random variable) 

UNDERLYING PROBABILITY SPACE

variables of the distribution : X : real vector
                                V : real vector, neighbor of X
                                T : transformation 
P(x,v,t) = P(v is obtained by applying transformation t on x) 
         = P(x,v|t)P(t)
         = N( T(t)(x)|v,sigma)P(t)

LEARNING BEHAVIOR
 
The parameters of the distributions are learned using a variant
of E.M.algorithm
- learns  a finite set of transformations 
- possibly:  learns the parameter sigma describing the noise distribution,
             learns p(t), the transformation distribution.   

 */




/****************************************************************************
 *AUXILIARY CLASS RECONSTRUCTION CANDIDATE : 
 *
 *
 *
 *Reconstruction Candidate objects are basically 4-tuples with the following format: 
 *         nKm x4 matrix 
 *    ------C1----------|---- C2------------|-- C3-----------|-- C4-----------|
 *    index i  in the   | index j in the    | index t of a   | positive weight
 *    training set of a | training set of a | transformation | 
 *    target point      | a neighbour       |                |
 *                      | candidate         |                | 
 *
 ***************************************************************************/ 
class ReconstructionCandidate
{
public:
    int targetIdx, neighborIdx, transformIdx;
    real weight;
    
    ReconstructionCandidate(int targetIdx_=-1, int neighborIdx_=-1, int transformIdx_=-1, real weight_=0){
        targetIdx =  targetIdx_;
        neighborIdx =  neighborIdx_;
        transformIdx =  transformIdx_ ;
        weight =  weight_;            
    }
};

//Comparisons between ReconstructionCandidate objects 
inline bool operator<(const ReconstructionCandidate& o1 ,
                      const ReconstructionCandidate& o2)
{
    //  Will be used in storage process, in a priority queue.
    //  With the following  definitions, priority measure increases when weight
    //  field decreases.
    //  That is, we want to keep ReconstructionCandidate objects with lower 
    //  weights on top of the priority queue 
    return o2.weight<o1.weight;
}
inline bool operator==(const ReconstructionCandidate& o1,
                       const ReconstructionCandidate& o2)
{
    return o1.weight==o2.weight;
}


//print/read ReconstructionCandidate objects
inline PStream& operator<<(PStream& out, 
                           const ReconstructionCandidate& x)
{
    out << tuple<int, int, int, real>(x.targetIdx, x.neighborIdx, x.transformIdx, x.weight);
    return out;
}
inline PStream& operator>>(PStream& in, ReconstructionCandidate& x)
{
    tuple<int, int, int, real> t;
    in >> t;
    tie(x.targetIdx, x.neighborIdx, x.transformIdx, x.weight) = t;
    return in;
}



/***************************************************************************
 * main class: TRANSFORMATION LEARNER
 *
 *
 *
 * Learns a finite set of linear transformations. That is, learns how to move from 
 * one point to another.  
 */
class TransformationLearner : public PDistribution
{
    typedef PDistribution inherited;

public:
    //#####  Public Build Options  ############################################

    //!A transformation learner might behave as a learner,as well as a generator
    int behavior;


    //!The following variable will be used to ensure p(x,v,t )>0 
    //!at the beginning (see implantation of randomReconstuctionWeight()
    //!for more details) 
    real minimumProba;
    
    
    //WHICH KIND OF TRANSFORMATION FUNCTIONS ... 
    
    //! what is the global form of the transformation functions used?
    int transformFamily;
    //! add a bias to the transformation function ?
    bool withBias;
    
    //LEARNING MODE ...

    //!is the variance(precision) of the noise random variable learned or fixed ? 
    //!(recall that the precision = 1/variance) 
    bool learnNoiseVariance;
   
    //!if we learn the noise variance, do we use the MAP estimator ? 
    bool regOnNoiseVariance;
    
    //!is the transformation distribution learned or fixed?
    bool learnTransformDistribution;
    
    //!if we learn the transformation distribution, do we use the MAP estimator ?
    bool regOnTransformDistribution;

    //!how the initial values of the parameters to learn are choosen?
    int initializationMode;

    //!For a given training point, we do not consider all the possibilities for the hidden variables.
    //!We approximate EM by using only the hidden variables with higher probability.
    //!That is, for each point in the training set, we keep a fixed number of
    //!hidden variables combinations, the most probable ones.
    //!We call that selection "large expection step".
    //!There are 2 versions, A and B. The following variables tells us when to
    //!perform each one. (see EStep() for more details)
    int largeEStepAPeriod;
    int largeEStepAOffset;
    int largeEStepBPeriod;
    int largeEStepBOffset;
    
    //!If the noise variance (precision) is learned, the following variables
    //!tells us when to update the noise variance in the maximization steps:
    //!(see MStep() for more details)
    int noiseVariancePeriod;
    int noiseVarianceOffset;                    
    
    //!These 2 parameters have to be defined if the noise variance is learned
    //!using a MAP procedure.
    //!We suppose that the prior distribution for the noise variance is a gamma
    //!distribution with parameters alpha and beta:
    //! p(x|alpha,beta)= x^(alpha-1)beta^(alpha)exp(-beta*x)/gamma(alpha)
    //!Note : if alpha = 1, beta=0, all the possibilities are equiprobable 
    //!      (no regularization effect)      
    real noiseAlpha;
    real noiseBeta;
    
    //!If the transformation distribution is learned, the following variables
    //!tells us when to update it in the maximization steps:
    //!(see MStep() for more details)
    int transformDistributionPeriod;
    int transformDistributionOffset;
    
    //!This parameter have to be defined if the transformation distribution
    //!is learned using a MAP procedure. We suppose that this distribution have a a multinomial form
    //(u1,u2,...,uK) with dirichlet prior probability : 
    // p(u1,...,uK) = NormalisationCoeff(alpha)*u1^(alpha -1)*u2^(alpha -1)...*uK^(alpha  - 1)
    //Note: if alpha = 1, it means all possibilities are equiprobable
    //      (no regularization effect)  
    real transformDistributionAlpha;


    //!tells us when to update the transformation parameters
    int transformsPeriod;
    int transformsOffset;
    int biasPeriod;
    int biasOffset;

    //PARAMETERS OF THE DISTRIBUTION

    //! variance of the NOISE random variable. 
    //!(recall that this r.v. is normally distributed with mean 0).
    //! -if it is a learned parameter, will be considered as the initial value 
    //!  of the noise variance parameter.
    //! -if it is not well defined (<=0), it will be redefined using its
    //!  prior distribution (Gamma).
    real noiseVariance;
    
    //! variance on the transformation parameters (prior distribution = normal with mean 0)
    real transformsVariance;
    
    //!number of transformations
    int nbTransforms;
    
    //!number of neighbors
    int nbNeighbors;
    
    //!multinomial distribution for the transformation:
    //!(i.e. probabilit of kth transformation = transformDistriibution[k])
    //!(might be learned or fixed)
    //!-if it is a learned parameter, will be considered as the initial value
    //! or the transformation distribution
    //!-if it is not well defined (size, positivity, sum to 1), it will be 
    //! redefined using its prior distribution (Dirichlet).
    Vec transformDistribution; 
   
    

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    TransformationLearner();


    //#####  PDistribution Member Functions  ##################################
 

    // virtual TVec<string> getTrainCostNames() const;
    
    
    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& y) const;

    //! Return a pseudo-random sample generated from the conditional
    //! distribution, of density p(y | x).
    virtual void generate(Vec& y) const ;

    //### Override this method if you need it (and if your distribution can
    //### handle it. Default version calls PLERROR.
    //! Generates a pseudo-random sample x from the reversed conditional
    //! distribution, of density p(x | y) (and NOT p(y | x)).
    //! i.e., generates a "predictor" part given a "predicted" part, regardless
    //! of any previously set predictor.
    // virtual void generatePredictorGivenPredicted(Vec& x, const Vec& y);

    //### Override this method if you need it. Default version calls
    //### random_gen->manual_seed(g_seed) if g_seed !=0
    //! Reset the random number generator used by generate() using the
    //! given seed.
    // virtual void resetGenerator(long g_seed) const;

    // ### These methods may be overridden for efficiency purpose:
    /*
    //### Default version calls exp(log_density(y))
    //! Return probability density p(y | x)
    virtual real density(const Vec& y) const;

    //### Default version calls setPredictorPredictedSises(0,-1) and generate
    //! Generates a pseudo-random sample (x,y) from the JOINT distribution,
    //! of density p(x, y)
    //! i.e., generates a predictor and a predicted part, regardless of any
    //! previously set predictor.
    virtual void generateJoint(Vec& xy);

    //### Default version calls generateJoint and discards y
    //! Generates a pseudo-random sample x from the marginal distribution of
    //! predictors, of density p(x),
    //! i.e., generates a predictor part, regardless of any previously set
    //! predictor.
    virtual void generatePredictor(Vec& x);

    //### Default version calls generateJoint and discards x
    //! Generates a pseudo-random sample y from the marginal distribution of
    //! predicted parts, of density p(y) (and NOT p(y | x)).
    //! i.e., generates a predicted part, regardless of any previously set
    //! predictor.
    virtual void generatePredicted(Vec& y);
    */


    //#####  PLearner Member Functions  #######################################

    // ### Default version of inputsize returns learner->inputsize()
    // ### If this is not appropriate, you should uncomment this and define
    // ### it properly in the .cc
    virtual int inputsize() const;

    /**
     * (Re-)initializes the PDistribution in its fresh state (that state may
     * depend on the 'seed' option).  And sets 'stage' back to 0 (this is the
     * stage of a fresh learner!).
     * ### You may remove this method if your distribution does not
     * ### implement it.
     */
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage == nstages, updating the train_stats collector with training
    //! costs measured on-line in the process.
    // ### You may remove this method if your distribution does not
    // ### implement it.
    virtual void train();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(TransformationLearner);


    //!INITIAL VALUES OF  THE PARAMETERS TO LEARN
    
    //!initializes the transformation parameters randomly 
    //!(prior distribution= Normal(0,transformsVariance))
    void initTransformsParameters();

    //!initializes the transformation parameters to the given values
    //!(bias are set to 0)
    void setTransformsParameters(TVec<Mat>  transforms, Mat bias=Mat());
    
   


    //!initializes the noise variance randomly
    //!(gamma distribution)
    void initNoiseVariance();
    
    //!initializes the noise variance with the given value
    void setNoiseVariance(real nv);
    
    //!initializes the transformation distribution randomly
    //!(dirichlet distribution)
    void initTransformDistribution();
    
    //!initializes the transformation distribution with the given values
    void setTransformDistribution(Vec td);
    
    
    //!GENERATION FUNCTIONS
    
    //!generates a sample data point from a source data point
    void generatePredictedFrom(const Vec & source, Vec & sample)const;
    
    //!generates a sample data point from a source data point with a specific transformation
    void generatePredictedFrom(const Vec & source, Vec & sample, int transformIdx)const;

    //!generates a sample data point from a source data point and returns it
    //! (if transformIdx >= 0 , we use the corresponding transformation )
    Vec returnPredictedFrom(Vec source, int transformIdx=-1)const ;
    

    //!fill the matrix "samples" with data points obtained from a given center data point
    void batchGeneratePredictedFrom(const Vec & center,
                                     Mat & samples)const;
    
    //!fill the matrix "samples" with data points obtained form a given center data point
    //!    - we use a specific transformation
    void batchGeneratePredictedFrom(const Vec & center,
                                     Mat & samples,
                                     int transformIdx)const ;

    //Generates n samples from center and returns them stored in a matrix
    //    (generation process = 1) choose a transformation (*),
    //                          2) apply it on center
    //                          3) add noise)
    // - (*) if transformIdx>=0, we always use the corresponding transformation
    Mat returnGeneratedSamplesFrom(Vec center, int n, int transformIdx=-1)const;
    
    
    //!select a transformation randomly (with respect to our multinomial distribution)
    int pickTransformIdx() const;

    //!Select a neighbor in the training set randomly
    //!(return his index in the training set)
    //!We suppose all data points in the training set are equiprobables
    int pickNeighborIdx() const;

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
    void treeDataSet(const Vec &root,
                     int deepness,
                     int branchingFactor,
                     Mat & dataPoints,
                     int transformIdx = -1)const;
    Mat returnTreeDataSet(Vec root,
                          int deepness,
                          int branchingFactor,
                          int transformIdx =-1)const;
    

    //!create a "sequential" dataset:
    //!  start -> first point -> second point ... ->nth point
    //! (where "->" stands for : "generate the")
    void sequenceDataSet(const Vec & start,
                         int n,
                         Mat & dataPoints,
                         int transformIdx=-1)const;

    Mat returnSequenceDataSet(Vec start,int n, int transformIdx=-1)const;

  

    



    //!COPIES OF THE STRUCTURES

    //!returns the "idx"th data point in the training set
    Vec returnTrainingPoint(int idx)const;

    //!returns all the reconstructions candidates associated to a given target
    TVec<ReconstructionCandidate> returnReconstructionCandidates(int targetIdx)const;

    //!returns the reconstructions of the "targetIdx"th data point value in the training set
    //!(one reconstruction for each reconstruction candidate)
    Mat returnReconstructions(int targetIdx)const;

    //!returns the neighbors choosen to reconstruct the target
    //!(one choosen neighbor for each reconstruction candidate associated to the target)
    Mat returnNeighbors(int targetIdx)const;

    //!returns the parameters of the "transformIdx"th transformation
    Mat returnTransform(int transformIdx)const;

    //!returns the parameters of each transformation
    //!(as an KdXd matrix, K = number of transformations,
    //!                    d = dimension of input space)
    Mat returnAllTransforms()const;


    //OTHER BUILDING/INITIALIZATION METHODS 

    // Simply calls inherited::build() then build_()
    virtual void build();
    
    //! main initialization operations that have to be done before any training phase
    void mainLearnerBuild();
    
    void buildLearnedParameters();
    

    //! initialization operations that have to be done before a generation process
    //! (all the undefined parameters will be initialized  randomly)
    void generatorBuild(int inputSpaceDim_=2,
                        TVec<Mat> transforms_ =TVec<Mat>(),  
                        Mat biasSet_ =Mat(),  
                        real noiseVariance_ =-1.0,
                        Vec transformDistribution_ =Vec());
    
    


    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    

    //TRANSFORMATIONS
    
    //!set of transformations:
    //!mdxd matrix :  -m = number of transformation,
    //!               -d = dimensionality of the input space
    //!               -rows kd to kd + d (exclusively) = sub-matrix = parameters of the
    //!                                                               kth transformation
    //!                                                               (0<=k<m)
    Mat transformsSet;
    TVec<Mat> transforms; //!views on sub-matrices of the matrix transformsSet
    
    //!set of bias (one by transformation)
    //!-might be used only if the flag "withBias" is turned on
    Mat biasSet;
    
    //SELECTED HIDDEN VARIABLES COMBINATIONS

    //!a reconstruction set:
    //!-choosen hidden variables combinations for each point in the training set 
    //!-implemented as a vector of "ReconstructionCandidate" objects.
    TVec< ReconstructionCandidate > reconstructionSet; 
    
    
    //DIMENSION VARIABLES

    //!dimension of the input space
    int inputSpaceDim;

    //!number of hidden variables combinations keeped for a specific target 
    //!in the reconstruction set.
    //!(Those combinations might be seen like reconstructions of the target)
    int nbTargetReconstructions;
    
    //!total number of combinations (x,v,t) keeped in the reconstruction set
    int nbReconstructions;
    
    //!number of samples given in the training set
    int trainingSetLength;
    
    //USEFUL CONSTANTS
    
    
    //!standard deviations for the transformation parameters:
    real transformsSD;
    

    //OTHERS

    
    //! Will be used to store a view on the reconstructionSet.
    //! The view will consist in all the entries related to a specific target
    TVec<ReconstructionCandidate> targetReconstructionSet;
    
    //!Storage space that will be used in the maximization step, in transformation parameters
    //! updating process.
    //!It represents a set of sub-matrices.There are exactly 2 sub-matrices by transformation.
    Mat B_C;
    //!Vectors of matrices that will be used in transformations parameters updating process.
    //!Each matrix is a view on a sub-matrix in th bigger matrix "B_C" described above.
    TVec<Mat> B,C;
    
   
    
protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);
    //! Declares the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);


private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();


    //!VIEWS ON RECONSTRUCTION SET AND TRAINING SET
    
    //! stores a VIEW on the reconstruction candidates related to the specified
    //! target (into the variable "targetReconstructionSet" )
    void seeTargetReconstructionSet(int targetIdx,
                                    TVec<ReconstructionCandidate> & targetReconstructionSet)const ;
 

    //! stores the "idx"th training data point into the variable 'dst'
    inline void seeTrainingPoint(const int idx, Vec & dst) const ;


    //!GENERATE GAMMA RANDOM VARIABLES
    
    //!source of the algorithm: http://oldmill.uchicago.edu/~wilder/Code/random/Papers/Marsaglia_00_SMGGV.pdf
    
    //!returns a pseudo-random positive real number x  
    //!using the distribution p(x)=Gamma(alpha,beta)
    real gamma_sample(real alpha,real beta=1)const;
    
    
    //!GENERATE DIRICHLET RANDOM VARIABLES
    //!source of the algorithm: WIKIPEDIA
    
    //!returns a pseudo-random positive real vector x 
    //!using the distribution p(x) = Dirichlet(x| all the parameters = alpha)
    //!-all the element of the vector are between 0 and 1,
    //!-the elements of the vector sum to 1
    void dirichlet_sample(real alpha, Vec & sample)const;
    Vec return_dirichlet_sample(real alpha)const;

    
  
    

    //!OPERATIONS ON WEIGHTS 
    
     //!normalizes the reconstruction weights related to a given target. 
    inline void normalizeTargetWeights(int targetIdx, real totalWeight);
    
    //!returns a random weight 
    inline real randomWeight() const;
    
    //!arithmetic operations on  reconstruction weights
    inline real INIT_weight(real initValue) const; //!CONSTRUCTOR
    inline real PROBA_weight(real weight) const; //!GET CORRESPONDING PROBABILITY 
    inline real DIV_weights(real numWeight, real denomWeight) const; //!DIVISION
    inline real MULT_INVERSE_weight(real weight) const ;//!MULTIPLICATIVE INVERSE
    inline real MULT_weights(real weight1, real weight2) const ; //!MULTIPLICATION
    inline real SUM_weights(real weight1, real weight2) const ; //!SUM 
    
    //!update/compute the weight of a reconstruction candidate with
    //!the actual transformation parameters
    inline real updateReconstructionWeight(int candidateIdx);
    inline real updateReconstructionWeight(int candidateIdx,
                                           const Vec & target,
                                           const Vec & neighbor,
                                           int transformIdx,
                                           Vec & predictedTarget);
    inline real computeReconstructionWeight(const ReconstructionCandidate & gc) const;
    inline real computeReconstructionWeight(int targetIdx, 
                                            int neighborIdx, 
                                            int transformIdx) const;
    inline real computeReconstructionWeight(const Vec & target,
                                            int neighborIdx,
                                            int transformIdx) const;
    inline real computeReconstructionWeight(const Vec & target,
                                            const Vec & neighbor,
                                            int transformIdx)const;
    inline real computeReconstructionWeight(const Vec & target,
                                            const Vec & neighbor,
                                            int transformIdx,
                                            Vec & predictedTarget)const;
    
    //!applies "transformIdx"th transformation on data point "src"
    inline void applyTransformationOn(int transformIdx, const Vec & src , Vec & dst) const ;

    
    //! verify if the multinomial distribution given is well-defined
    //! i.e. verify that the weights represent probabilities, and that 
    //! those probabilities sum to 1 . 
    //!(the distribution is represented as a set of weights, which are typically
    //! log-probabilities)
    bool isWellDefined(Vec & distribution)const;

    //!INITIAL E STEP 
    
    //!initialization of the reconstruction set
    void initEStep();

    //!initialization of the reconstruction set, version A
    //for each target:
    // 1)find the neighbors (we use euclidean distance as an heuristic)
    // 2)for each neighbor, assign a random weight to each possible transformation
    void initEStepA();

    //!initialization of the reconstruction set, version B

    void initEStepB();
    

    
    //!auxialiary function of "initEStep" . 
    //!    for a given pair (target, neighbor), creates all the  
    //!    possible reconstruction candidates. 
    //!returns the total weight of the reconstruction candidates created
    real expandTargetNeighborPairInReconstructionSet(int targetIdx,
                                                     int neighborIdx,
                                                     int candidateStartIdx);
    
    //!auxiliary function of initEStep
    //!    stores the nearest neighbors for a given target point in a priority
    //!    queue.
    void findNearestNeighbors(int targetIdx,
                              priority_queue< pair< real, int > > & pq);
    
    
    //!E STEP

    //!coordination of the different kinds of expectation steps
    //!  -which are : largeEStepA, largeEStepB, smallEStep
    void EStep();
    
    //!LARGE E STEP : VERSION A (expectation step)

    //!full update of the reconstruction set
    //!for each target, keeps the km most probable <neighbor, transformation> 
    //!pairs (k = nb neighbors, m= nb transformations)
    void largeEStepA();

    //!auxiliary function of largeEStepA()
    //!   for a given target, stores the km most probable (neighbors,
    //!   transformation) pairs in a priority queue 
    //!   (k = nb neighbors, m = nb transformations)
    void findBestTargetReconstructionCandidates
    (int targetIdx,
     priority_queue< ReconstructionCandidate > & pq);
    
    
    //!LARGE E STEP : VERSION B (expectation step)
    
    //!full update of the reconstruction set
    //!   for each given pair (target, transformation), find the best
    //!   weighted neighbors  
    void largeEStepB();
    
    
    //!auxiliary function of largeEStepB()
    //!   for a given target x and a given transformation t , stores the best
    //!    weighted triples (x, neighbor, t) in a priority queue .
    void findBestWeightedNeighbors
    (int targetIdx,
     int transformIdx,
     priority_queue< ReconstructionCandidate > & pq);

    //!SMALL E STEP (expectation step)

    //!updating the weights while keeping the candidate neighbor set fixed
    void smallEStep();
   
    //!M STEP
    
    //!coordination of the different kinds of maximization step
    //!(i.e.: we optimize with respect to which parameter?)
    void MStep();

    //!maximization step  with respect to  transformation distribution
    //!parameters
    void MStepTransformDistribution();
    
    //!maximization step  with respect to transformation distribution
    //!parameters
    //!(MAP version, alpha = dirichlet prior distribution parameter)
    //!NOTE :  alpha =1 ->  no regularization
    void MStepTransformDistributionMAP(real alpha);

    //!maximization step with respect to transformation matrices
    //!(MAP version)
    void MStepTransformations();
  
    //!maximization step with respect to transformation bias
    //!(MAP version)
    void MStepBias();

    //!maximization step with respect to noise variance
    void MStepNoiseVariance();
    
    //!maximization step with respect to noise variance
    //!(MAP version, alpha and beta = gamma prior distribution parameters)
    //!NOTE : alpha=1, beta=0 -> no regularization   
    void MStepNoiseVarianceMAP(real alpha, real beta);    
    
    //!returns the distance between the reconstruction and the target
    //!for the 'candidateIdx'th reconstruction candidate
    inline real reconstructionEuclideanDistance(int candidateIdx);
    
    //increment the variable 'stage' of 1
    void nextStage();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
   
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TransformationLearner);

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
