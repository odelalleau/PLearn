// -*- C++ -*-

// TransformationLearner.h
//
//version 5 
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

/*! \file TransformationLearner.h */


#ifndef TransformationLearner_INC
#define TransformationLearner_INC

//C++
#include <utility>
#include <queue>

//Plearn
#include <plearn_learners/generic/PLearner.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/PRandom.h>
#include <plearn/base/tuple.h>


#define TRANSFORM_FAMILY_LINEAR 0
#define TRANSFORM_FAMILY_LINEAR_INCREMENT 1



namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */

/***** GENERATION CANDIDATE ***********************************************/


//Generate Candidate objects are basically 4-tuples with the following format: 
//         nKm x4 matrix 
//    ------C1----------|---- C2------------|-- C3-----------|-- C4-----------|
//    index i  in the   | index j in the    | index t of a   | positive weight
//    training set of a | training set of a | transformation | 
//    target point      | a neighbour       |                |
//                      | candidate         |                | 


class GenerationCandidate
{
public:
    int targetIdx, neighborIdx, transformIdx;
    real weight;

    GenerationCandidate(int targetIdx_=-1, int neighborIdx_=-1, int transformIdx_=-1, real weight_=0){
        targetIdx =  targetIdx_;
        neighborIdx =  neighborIdx_;
        transformIdx =  transformIdx_ ;
        weight =  weight_;            
    }
};
inline bool operator<(const GenerationCandidate& o1 , const GenerationCandidate& o2)
{
    // we need the inverse comparison for the priority queue
    return o2.weight>o1.weight;
}

inline bool operator==(const GenerationCandidate& o1, const GenerationCandidate& o2)
{
        return o1.weight==o2.weight;
}



inline PStream& operator<<(PStream& out, const GenerationCandidate& x)
    {
        out << tuple<int, int, int, real>(x.targetIdx, x.neighborIdx, x.transformIdx, x.weight);
        return out;
    }

inline PStream& operator>>(PStream& in, GenerationCandidate& x)
    {
        tuple<int, int, int, real> t;
        in >> t;
        tie(x.targetIdx, x.neighborIdx, x.transformIdx, x.weight) = t;
        return in;
    }

/********* END , GENERATION CANDIDATE *************************************/


class TransformationLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! for random generators
    long seed; 
    
    //! set the family of transformation functions we are interested in
    int transformFamily ;
    
    //HYPER-PARAMETERS OF THE ALGORITHM
    
    //!variance of the NOISE, considered here as a random variable normaly 
    //!distributed, with mean 0
    real noiseVariance;
    
    //!variance on the transformation parameters. (prior distribution = normal with
    //!mean 0).
    real transformsVariance;

    //number of transformations
    int nbTransforms;
    
    //number of neighbors
    int nbNeighbors;

    //minimum random weight to give to a  chosen generation candidate at
    //initialization step
    real epsilonInitWeight;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    TransformationLearner();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    // (PLEASE IMPLEMENT IN .cc)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    // (PLEASE IMPLEMENT IN .cc)
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

    //! Computes the output from the input.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    /*********GENERATION MODULE *****************************************/

    //The object is the representation of a learned distribution
    //Are are methods to ensure the "generative behavior" of the object
    //(Once the distribution is learned, we might be able to generate
    // samples from it)

    //Chooses the transformation parameters using a 
    //normal distribution with mean 0 and variance "transformsVariance"
    //(call generatorBuild() after)
    void buildTransformationParametersNormal();
   
    //set the transformation parameters to the specified values
    //(call generatorBuild() after)
    void setTransformationParameters(TVec<Mat> & transforms);

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
    void createDataSet(Vec & root,
                       int nbGenerations,
                       int generationLength,
                       Mat & dataPoints);
    
    //create a dataset using the same tree generation process as
    //createDataSet, except the number of child per parent is fixed to 1,
    //   root -> 1st point -> 2nd point ... -> nth point 
    void createDataSetSequentially(Vec & root,
                                   int n,
                                   Mat & dataPoints);

    //Select a transformation randomly (with respect ot our transformation
    //distribution)
    int pickTransformIdx();
    
    
    //here is the generation process for a given center data point 
    //  1) choose a transformation
    //  2) apply it on the center data point
    //  3) add noise

    //generates a sample data point  from a  given center data point 
    void generateFrom(Vec & center, Vec & sample);
    //generates a sample data point from a given center data point
    void generateFrom(Vec & center, Vec & sample, int transformIdx);
    //fill the matrix "samples" with sample data points obtained from
    // a given center data point.
    void batchGenerateFrom( Vec & center, Mat & samples); 
  
   
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(TransformationLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    //---EXTERIOR ACCESS ON LEARNED VARIABLES -------------------------------
    
    //returns all the entries in the generation set with the 
    //TARGET_IDX field fixed to "targetIdx"
    //those entries represents ways to reproduct the 
    //"targetIdx"th data point in the training set
    TVec<GenerationCandidate> returnReproductionSources(int targetIdx); 
  
    //returns the reproductions of the "targetIdx"th data point in the
    //training set
    //(one reproduction by reproduction source)
    Mat returnReproductions(int targetIdx);

    //returns the parameter of the "transformIdx"th transformation
    Mat returnTransform(int transformIdx);

    
    //returns the paramter of each transformation
    //(as an tdXd matrix, t = number of transformation,
    //                    d = dimension of input space)
    Mat returnAllTransforms();
    
    //Generates n samples from center and returns them
    //    (generation process = 1) choose a transformation,
    //                          2) apply it on center
    //                          3) add noise)
    Mat returnGeneratedSamplesFrom(Vec center, int n);
    
    //Creates a data set and returns it
    //(see createDataSet for more details on the generation process)
    Mat returnGeneratedDataSet(Vec root,
                               int nbGenerations,
                               int generationLength);

    //Generates a data set and returns it
    //(sequential generation process: see createDataSetSequentially for more details)
    Mat returnSequentiallyGeneratedDataSet(Vec root,int n);


protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

public:    
    
    //DIMENSION VARIABLES 
  
    //dimension of the input space;
    int inputSpaceDim;
    
    //number of generation candidates related to a specific target in the 
    //generation set. 
    int nbGenerationCandidatesPerTarget;

    //total number of generation candidates in the generation set
    int nbGenerationCandidates;

    //number of samples given in the training set
    int nbTrainingInput;

    //multinomial probability ditribution for the transformations
    //(i.e. probability of kth transformation = transformDistribution[k])
    Vec transformDistribution;
    

    //LEARNED MODEL PARAMETERS

    //set of transformations:
    //mdxd matrix :  -where m = number of transformation,
    //                      d = dimensionality of the input space
    //               -rows kd to kd + d (exclusively) = sub-matrix = parameters of the
    //                                                               kth transformation
    //                                                               (0<=k<m)
    Mat transformsSet ; 
    TVec< Mat > transforms; //views on sub-matrices of the matrix transformsSet 
    
    //a generationSet D : 
    //implemented as a vector of "GenerationCandidate" objects.
    TVec< GenerationCandidate > generationSet; 


    //OTHER VARIABLES

    //the weight decay
    real lambda;
    

    //factor used in the computation of generation weights :
    // 1/2(noiseVariance)y
    real noiseVarianceFactor;

    //standard deviation for the noise distribution, and transformation
    //parameters distributions:
    real noiseStDev,transformsStDev;

    //will be used to store a view on the generation set:
    //that is, all the entries related to a specific target . 
    TVec<GenerationCandidate>  targetGenerationSet; 
    
    //Storage space that will be used to update the transformation
    //parameters. It represents a set of sub-matrices. There are exactly 2 
    //sub-matrices by transformation.   
    Mat B_C ;

    //Vectors of matrices that will be used to update the transformation 
    //parameters. Each matrix is a view on a sub-matrix in B_C. 
    TVec<Mat> C , B ;

    //to retrieve easily an input point from the training set 
    Vec target, neighbor;

    

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);


    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

    //general building operations 
    void build_();
    
    //do the building operations related to training
    //warning: we suppose the training set has been transmitted
    //         before calling the method
    void trainInit();
    //do the building operations related to the generation process
    //warning: we suppose the transformation parameters are set 
    void generatorInit();

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
 

public:

    // ----------GENERAL USE--------------------------------------------------
    

    //REFERENCE OPERATIONS ON GENERATION SET AND TRAINING SET  

    // stores a view on the subset of generation set related to the specified
    // target (into the variable "targetGenerationSet" )
    void getViewOnTargetGenerationCandidates(int targetIdx);
    // stores the "targetIdx"th input in the training set into the variable
    // "target"
    void getTargetFromTrainingSet(int targetIdx);
    // stores the "neighborIdx"th input in the training set into the variable
    // "neighbor" 
    void getNeighborFromTrainingSet(int neighborIdx);
    
    
    //OPERATIONS RELATED TO GENERATION WEIGHTS
    
    //normalizes the generation weights related to a given target. 
    void normalizeTargetGenerationWeights(int targetIdx, real totalWeight);
    
    //returns a random positive weight 
    real randomPositiveGenerationWeight();
  
    //arithmetic operations on  generation weights
    real DIV_weights(real numWeight, real denomWeight); //DIVISION
    real MULT_INVERSE_weight(real weight);//MULTIPLICATIVE INVERSE
    real MULT_weights(real weight1, real weight2); //MULTIPLICATION
    real SUM_weights(real weight1, real weight2); //SUM 
    
    //update/compute the weight of a generation candidate with
    //the actual transformation parameters
    real updateGenerationWeight(int candidateIdx);
    real computeGenerationWeight(GenerationCandidate & gc);
    real computeGenerationWeight(int targetIdx, 
                                 int neighborIdx, 
                                 int transformIdx);
    
    //applies "transformIdx"th transformation on data point "src"
    void applyTransformationOn(int transformIdx, Vec & src , Vec & dst);
  

   //-------- INITIAL E STEP --------------------------------------------

    //initialization of the generation set 
    void initEStep();
    
    //auxialiary function of "initEStep" . 
    //    for a given pair (target, neighbor), creates all the associated 
    //    generation candidates (entries) in the data set. 
    //returns the total weight of the generation candidates created
    real expandTargetNeighborPairInGenerationSet(int targetIdx,
                                                 int neighborIdx,
                                                 int candidateStartIdx);
    
    //auxiliary function of initEStep
    //    keeps the nearest neighbors for a given target point in a priority
    //    queue.
    void findNearestNeighbors(int targetIdx,
                              priority_queue< pair< real, int > > & pq);
    

    //-------- LARGE E STEP : VERSION A --------------------------------

    //full update of the generation set
    //for each target, keeps the top km most probable <neighbor, transformation> 
    //pairs (k = nb neighbors, m= nb transformations)
    void largeEStepA();

    //auxiliary function of largeEStepA()
    //   for a given target, keeps the top km most probable neighbors,
    //   transformation pairs in a priority queue 
    //   (k = nb neighbors, m = nb transformations)
    void findBestTargetCandidates
    (int targetIdx,
     priority_queue< GenerationCandidate > & pq);
    

    //-------- LARGE E STEP : VERSION B --------------------------------

    //full update of the generation set
    //   for each given pair (target, transformation), find the best
    //   weighted neighbors  
    void largeEStepB();

    
    //auxiliary function of largeEStepB()
    //   for a given target x and a given transformationt , keeps the best
    //   weighted triples (x, neighbor, t) in a priority queue .
    void findBestWeightedNeighbors
    (int targetIdx,
     int transformIdx,
     priority_queue< GenerationCandidate > & pq);



    //-------- SMALL E STEP --------------------------------------------- 

    
    //updating the weights while keeping the candidate neighbor set fixed
    void smallEStep();
    

    //-------- M STEP ---------------------------------------------   
    

    //updating the transformation parameters
    void MStep();



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
