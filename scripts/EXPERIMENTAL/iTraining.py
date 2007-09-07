#iTraining2.py
#to control interactively the training process

import os, sys, time, matplotlib, math, numpy ,copy

from matplotlib.pylab import plot,show,draw,close,text,scatter,colorbar
from matplotlib.colors import *

from plearn.io.server import *
from plearn.pyplearn import *
from plearn.plotting import *
from numpy import *
from pickle import *
from copy import *




#RECONSTRUCTION CANDIDATE

#consists in a 4-element tuple:
#   (target, target<s neighbor, transformation, weight)
TARGET_IDX = 0
NEIGHBOR_IDX = 1
TRANSFORM_IDX = 2
WEIGHT = 3


#OTHER CONSTANTS
DEFAULT = 0
UNDEFINED = -1
EMPTY_BIAS = TMat()

#ITERATIVE LEARNER ----------------------------------------------------------
class IterativeLearner(object):


    #GRAPHICAL OUTPUT FORMAT (constants)

    #a data point is represented graphically by a dot
    #    his size/shape/color might change according to his nature:
    #          -target ?
    #          -neighbor ?
    #          -ordinary training point ?
    #          -reconstruction ?
    #we also need some formats to draw the transformations

    #sizes
    MIN_SIZE = 10.0
    MAX_SIZE = 100.0
    TARGET_SIZE = MAX_SIZE
    NEIGHBOR_SIZE = MIN_SIZE
    DEFAULT_SIZE = MIN_SIZE
    RECONSTRUCTION_SIZE_FACTOR = 1

    #colors
    TARGET_COLOR = 'b'
    NEIGHBOR_COLOR = 'k'
    DEFAULT_COLOR = 'w'
    RECONSTRUCTION_COLOR = 'y'
    TRANSFORMATION_COLOR  = 'k'

    #shapes
    TARGET_SHAPE = 'o'
    NEIGHBOR_SHAPE = 'o'
    DEFAULT_SHAPE = 'o'
    RECONSTRUCTION_SHAPE = 'd'

    #index of a transformation : size of the police
    TRANSFORMATION_DIGIT_SIZE = 12

    #LEARNER
    
    learner = UNDEFINED                    #TransformationLearner object
    dim = 2                                #dimension of input space
    nbTransforms = UNDEFINED               #number  of transforms to learn
    withBias = False                       #includes a bias addition in the transformation function?
    transformsToLearn = []                 #the transformations matrices to learn
    biasToLearn = EMPTY_BIAS                #the transformations bias to learn, if any
    learnedTransforms = []                 #learned transformations matrices
    learnedBias = EMPTY_BIAS                #learned transformations bias, if any
    learnNoiseVariance = False             #noise variance = learned parameter ?
    noiseVariance = UNDEFINED              #noise variance (fixed or learned)
    noiseVarianceToLearn=UNDEFINED         #noise variance to learn (if any)
    learnTransformDistribution = False     #transformation distribution = learned parameter ?
    transformDistribution = []             #transformation distribution (fixed or learned)
    transformDistributionToLearn = []      #transformation distribution to learn (if any)
    data = array([])                       #training data points
    transformFamily=0                      #type of transformation functions used
    

    #TARGET AND CORRESPONDING RECONSTRUCTION CANDIDATES
    testTargetIdx = 0
    target = []
    neighbors = array([])
    reconstructions= array([])
    weights = array([])
    recSizes = array([])
    recColors = array([])
    choosenTransforms = array([])


    #INITIALIZATION PROCEDURES

    #constructor
    def __init__(self,
                 learner,
                 dim=2,
                 testTargetIdx=0,
                 transformsToLearn=[],
                 bias=array([]),
                 noiseVarianceToLearn=UNDEFINED,
                 transformDistributionToLearn=[],
                 data=array([])):
        self.learner = learner
        self.dim = dim
        self.nbTransforms = self.learner.getOption("nbTransforms")
        self.withBias = self.learner.getOption("withBias")
        self.transformFamily = self.learner.getOption("transformFamily")
        self.testTargetIdx = testTargetIdx
        self.learnNoiseVariance = self.learner.getOption("learnNoiseVariance")
        if(self.learnNoiseVariance and noiseVarianceToLearn>0):
            self.setNoiseVarianceToLearn(noiseVarianceToLearn)
        else:
            self.updateNoiseVariance()
        self.learnTransformDistribution = self.learner.getOption("learnTransformDistribution")
        if(self.learnTransformDistribution and len(transformDistributionToLearn) >0):
            self.setTransformDistributionToLearn(transformDistributionToLearn)        
        if(len(transformsToLearn) != 0):
            self.setTransformsToLearn(transformsToLearn, bias)
        if(len(data)!=0):
            self.setTrainingSet(data)


    def setNoiseVarianceToLearn(self,
                                noiseVarianceToLearn):
        assert(noiseVarianceToLearn>0)
        self.noiseVarianceToLearn = noiseVarianceToLearn

    def setTransformDistributionToLearn(self,
                                        transformDistributionToLearn):
        assert(len(transformDistributionToLearn) == self.nbTransforms)
        sum = 0
        for i in range(self.nbTransforms):
            p = exp(transformDistributionToLearn[i])
            assert( 0<= p <=1)
            sum = sum + p
        assert(sum == 1)
        self.transformDistributionToLearn = copy(transformDistributionToLearn)
        
        

    #specifies the set of transformation functions that might be learned        
    def setTransformsToLearn(self,
                             transformsToLearn,
                             bias=EMPTY_BIAS):
        assert(len(transformsToLearn)==self.nbTransforms)
        if(self.withBias):
            assert(bias.shape[0] == self.nbTransforms)
            assert(bias.shape[1] == self.dim)
            self.biasToLearn= bias.copy()
        else:
            self.biasToLearn = EMPTY_BIAS
        for i in range(self.nbTransforms):
            assert(self.dim == transformsToLearn[i].shape[0])
            assert(self.dim == transformsToLearn[i].shape[1])
        self.transformsToLearn = copy(transformsToLearn)


    #defines the training set of the learner with the given datas
    def setTrainingSet(self,
                       datas):
        assert(self.dim == datas.shape[1])
        self.data = copy(datas)
        trainset = pl.MemoryVMatrix(data=self.data,
                                    inputsize = self.dim,
                                    targetsize = 0,
                                    weightsize = 0,
                                    length = datas.shape[0],
                                    width = datas.shape[1])
        self.learner.setTrainingSet(trainset,True)
        self.updateLearnedParameters()
        if(not self.learnTransformDistribution):
            self.updateTransformDistribution()
        


    #gets the current value of the learner's transformations , noise variance(optional)
    #and transformation distribution(optional)
    def updateLearnedParameters(self):
        self.updateTransforms()
        if(self.learnNoiseVariance):
            self.updateNoiseVariance()
        if(self.learnTransformDistribution):
            self.updateTransformDistribution()

    
    #registers the "real values" of the parameters to learn
    def setParametersToLearn(self,
                             transformsToLearn,
                             biasToLearn=array([]),
                             noiseVarianceToLearn = -1,
                             transformDistributionToLearn = []):
        self.setTransformsToLearn(transformsToLearn, biasToLearn)
        if(self.learnNoiseVariance):
            self.setNoiseVarianceToLearn(noiseVarianceToLearn)
        if(self.learnTransformDistribution):
            self.setTransformDistributionToLearn(transformDistributionToLearn)
        
        

    #updates the variables 'learnedTransforms' and 'learnedBias' to ensure that
    #they correspond to the learner 's transformation matrices and bias 
    def updateTransforms(self):
        self.learnedTransforms = self.learner.getOption("transforms")
        if(self.withBias):
            self.learnedBias = self.learner.getOption("biasSet")


    #updates the variable 'noiseVariance' to ensure it correspond to the learner's noise variance
    def updateNoiseVariance(self):
        self.noiseVariance = self.learner.getOption("noiseVariance")


    #updates the variable 'transformDistribution' to ensure it correspond to the learner's transformation distribution
    def updateTransformDistribution(self):
        self.transformDistribution = self.learner.getOption("transformDistribution")


    #updates reconstruction datas 
    def updateReconstructionDatas(self):
        self.target = self.learner.returnTrainingPoint(self.testTargetIdx)
        temp = self.learner.returnReconstructionCandidates(self.testTargetIdx)
        reconstructionCandidates = array(temp, 'd')
        reconstructionCandidates.resize(len(temp),4)
        self.weights = copy(reconstructionCandidates[:,WEIGHT])
        for i in range(self.weights.shape[0]):
            self.weights[i]=exp(self.weights[i])
            self.recSizes = (multiply(self.weights,self.MAX_SIZE - self.MIN_SIZE)
                             +
                             self.MIN_SIZE)
        self.choosenTransforms = copy(reconstructionCandidates[:,TRANSFORM_IDX])
        self.neighbors = self.learner.returnNeighbors(self.testTargetIdx)
        self.reconstructions = self.learner.returnReconstructions(self.testTargetIdx)
        

    #extracts the data points from a file and returns them
    #(in a matricial form)
    def load_data(self,
                  filename):
        data_in = open(filename)
        (inputsize, targetsize, weightsize) = [int(x) for x in data_in.readline().split()]
        assert(targetsize == 0)
        assert(weightsize == 0)
        data = []
        n_samples = 0
        for line in data_in.readlines():
            data +=[float(x) for x in line.split()]
            n_samples = n_samples + 1
        data_in.close()
        data = array(data, 'd')
        data.resize(n_samples,inputsize)
        return data


    #sets the training set (extracts first the data points from a file)
    def setTrainingSetFromFile(self,
                               filename):
        self.setTrainingSet(self.load_data(filename))


    #re-initializes the present object    
    def reset(self,
              dim = 2,
              testTargetIdx = 0,
              transformsToLearn=[],
              biasToLearn=array([]),
              noiseVarianceToLearn=UNDEFINED,
              transformDistributionToLearn =[],
              data=array([])):
        self.__init__(self.learner,
                      dim,
                      testTargetIdx,
                      transformsToLearn,
                      biasToLearn,
                      noiseVarianceToLearn,
                      transformDistributionToLearn,
                      data)


    #GRAPHICAL PROCEDURES
    
    #draws the target test point
    #(big blue dot)
    def drawTarget(self):
        scatter([self.target[0]],
                [self.target[1]],
                [self.TARGET_SIZE],
                c=self.TARGET_COLOR,
                marker = self.TARGET_SHAPE)


    #draws the neighbors associated to the reconstruction candidates
    #(small red dots)
    def drawNeighbors(self):
        scatter(self.neighbors[:,0].tolist(),
                self.neighbors[:,1].tolist(),
                self.NEIGHBOR_SIZE,
                c =  self.NEIGHBOR_COLOR,
                marker=self.NEIGHBOR_SHAPE)

    #draws all the training data points
    #(small black circles)
    def drawTrainingSet(self):
        scatter(self.data[:,0].tolist(),
                self.data[:,1].tolist(),
                self.DEFAULT_SIZE,
                c = self.DEFAULT_COLOR,
                marker = self.DEFAULT_SHAPE)

    #draws the reconstructions of the test target point
    #(yellow diamonds, the most probable the reconstruction, the bigger the diamond shape)
    def drawReconstructions(self):
        scatter(self.reconstructions[:,0].tolist(),
                self.reconstructions[:,1].tolist(),
                self.recSizes.tolist(),
                c=self.RECONSTRUCTION_COLOR,
                marker=self.RECONSTRUCTION_SHAPE)

        
    #draws the transformations associated to the reconstruction candidates
    #   -each transformation is represented as an arrow, and an integer
    #    (the transformation index)
    def drawChoosenTransforms(self):
        for i in range(self.reconstructions.shape[0]):
            mid_X = 0.5*(self.neighbors[i][0] + self.reconstructions[i][0])
            mid_Y = 0.5*(self.neighbors[i][1] + self.reconstructions[i][1])
            label = str(int(self.choosenTransforms[i]))
            text(mid_X,
                 mid_Y,
                 label,
                 fontsize=self.TRANSFORMATION_DIGIT_SIZE)
            plot([self.neighbors[i][0],self.reconstructions[i][0]],
                 [self.neighbors[i][1],self.reconstructions[i][1]],
                 c=self.TRANSFORMATION_COLOR)



    # draws the graph representing the different reconstruction candidates
    # of the test  target point
    def drawLearningGraph(self):
        clf()
        self.updateReconstructionDatas()
        self.drawTrainingSet()
        self.drawTarget()
        self.drawNeighbors()
        self.drawChoosenTransforms()
        self.drawReconstructions()
        draw()
      

    #CONTROL PROCEDURES


    #prints the list of control keys
    def help(self):
        print "CONTROL PROCEDURES AND CORRESPONDING KEYS:\n"
        
        print "printLearnState() ........... 'p'"
        print "initEStep() ................. 'i'"
        print "smallEStep() ................ 's'"
        print "largeEStepA() ............... 'a'"
        print "largeEStepB() ............... 'b'"
        print "MStep() ..................... 'm'"
        print "MStepTransformations() ...... 't'"
        print "MStepTransformationsDiv() ... 'y'"
        print "MStepNoiseVariance() ........ 'n'"
        print "MStepTransformDistribution()  'd'"
        print "printReconstructionsProbas()  'r'"
        print "printTransforms() ........... 'z'"
        print "printNoiseVariance() ........ 'x'"
        print "printDistribution() ......... 'c'"
        print "nextStage() ................. ' '"
        print "routine1() .................. '1'"
        print "help() ...................... 'h'"
        



    r1_NB_ITERATIONS = 1
    def routine1(self):
        for i in range(self.r1_NB_ITERATIONS):
            self.MStep()
            self.smallEStep()
            self.nextStage()
        self.printLearnState()
    

    #prints the learned parameters and the values of the parameters to learn
    #(control key == 'p')
    def printLearnState(self): 
        self.updateLearnedParameters()
        n = len(self.transformsToLearn)
        print "transformations to learn:"
        for i in range(n):
            print "\n"
            print self.transformsToLearn[i]
            if(self.withBias):
                print "bias: ", self.biasToLearn[i,:]
        print "\n"
        print "learned transformations:"
        for i in range(n):
            print "\n"
            print self.learnedTransforms[i]
            if(self.withBias):
                print "bias: ", self.learnedBias[i,:]
        print "\n"
        if(self.learnNoiseVariance):
            print "noise variance to learn:  ", self.noiseVarianceToLearn
            print "learned noise variance: " , self.noiseVariance
            print "\n"
        if(self.learnTransformDistribution):
            print "transformation distribution to learn, format =(log,proba):"
            print [["(",x," ", exp(x),")"] for x in self.transformDistributionToLearn]
            print "learned transformation distribution (log/proba) :"
            print [(x, exp(x)) for x in self.transformDistribution]
            print "\n"
            


    #initEStep  (control key == 'i')
    def initEStep(self):
        assert(len(self.data!= 0))
        #print "** initEStep **"
        self.learner.initEStep()
        self.updateTransforms()
    
        
    #smallEStep   (control key == 's')
    def smallEStep(self):   
        #print "** smallEStep **"
        self.learner.smallEStep()
        self.printReconstructionsProbas()


    #largeEStepA   (control key == 'a')
    def largeEStepA(self):
        #print "** largeEStepA **"
        self.learner.largeEStepA()
        self.printReconstructionsProbas()
     

    #largeEStepB   (control key == 'b')
    def largeEStepB(self):
        #print "** largeEStepB **"
        self.learner.largeEStepB()
        self.printReconstructionsProbas()


    #MStep (control key == 'm')
    def MStep(self):
        #print "** MStep **"
        self.learner.MStep()
        #self.printLearnState()

    #MStepTransformations (control key == 'T')
    def MStepTransformations(self):
        #print "** MStepTransformations **"
        self.learner.MStepTransformations()
        self.printTransforms()

    mstd_t = 0
    #MStepTransformationDiv (control key == 'H')
    def MStepTransformationDiv(self):
        #print "** MStepTransformations**"
        #print "transform: ", mstd_t
        self.learner.MStepTransformationDiv(self.mstd_t)
        self.mstd_t = (self.mstd_t + 1) % self.nbTransforms

    #MStepNoiseVariance (control key == 'N')
    def MStepNoiseVariance(self):
        #print "** MStepNoiseVariance **"
        self.learner.MStepNoiseVariance()
        self.printNoiseVariance()

    #MStepDistribution (control key == 'D')
    def MStepTransformDistribution(self):
        #print "** MStepDistribution **"
        self.learner.MStepTransformDistribution()
        self.printLearnState()

    #prints the  probabilities of the reconstructions associated to the present test target point
    #(control key == r)
    def printReconstructionsProbas(self):
        self.updateReconstructionDatas()
        print "reconstructions and their weights (weight format : (log,proba))"
        for i in range(self.reconstructions.shape[0]):
            print  self.reconstructions[i], " (" , log(self.weights[i]),", ", self.weights[i], ")"
        print "\n"
    
    #prints the current learned transformations (control key == 't')
    def printTransforms(self):
        self.updateLearnedParameters()
        print "current transformations:\n"
        for i in range(self.nbTransforms):
            print self.learnedTransforms[i]
            if(self.withBias):
                print self.learnedBias[i,:]
            print "\n"
        
    #prints the value of the learner's noise variance (control key == 'n')
    def printNoiseVariance(self):
        self.updateLearnedParameters()
        print "noise variance: " , self.noiseVariance , "\n"
        

    #prints the value of the learner's transformation distribution
    #(control key == 'd')
    def printDistribution(self):
        self.updateLearnedParameters()
        print "transformation distribution (log, proba): \n"
        print [(x, exp(x)) for x in self.transformDistribution ]
        print "\n"


    #increment the learner variable 'stages' of 1 (control key == 'n')
    def nextStage(self):
        self.learner.nextStage()


    #GENERAL USE PROCEDURES


    #returns the square euclidean distance between data points x and y 
    def squareEuclideanDistance(self,x,y):
        return pow(x[0] - y[0],2) + pow(x[1] - y[1],2)   
    

    #RUN PROCEDURE (main)

    i=1
    def run(self):
        self.learner.buildLearnedParameters()
        self.initEStep()
        self.drawLearningGraph()
        self.i = 1
        def mouse_press(event):
            if(event.button == 2):
                p = [event.xdata,event.ydata]
                min_idx = 0
                min_d = self.squareEuclideanDistance(self.data[0],p)
                for i in range(1,len(self.data)):
                    d = self.squareEuclideanDistance(self.data[i],p)
                    if(d< min_d):
                        min_d = d
                        min_idx = i
                self.testTargetIdx = min_idx
                self.target = self.data[min_idx]
                print "new target: ", min_idx, "\n"
                self.drawLearningGraph()
        
        def key_press(event):
            if(event.key == '1'):
                self.routine1()
            if(event.key == 'p' ):
                self.printLearnState()
            if(event.key == 'i'):
                self.initEStep()
            if(event.key == 's'):
                self.smallEStep()
            if(event.key == 'a'):
                self.largeEStepA()
            if(event.key == 'b'):

                self.largeEStepB()
            if(event.key == 'm'):
                self.MStep()
            if(event.key == 't' ):
                self.MStepTransformations()
            if(event.key == 'n'):
                self.MStepNoiseVariance()
            if(event.key == 'd'):
                self.MStepTransformDistribution()
            if(event.key == 'y'):
                self.MStepTransformationDiv()
            if(event.key == 'r'):
                self.printReconstructionsProbas()
            if(event.key == 'z'):
                self.printTransforms()
            if(event.key == 'x'):
                self.printNoiseVariance()
            if(event.key == 'c' ):
                self.printDistribution()
            if(event.key == 'h'):
                self.help()
            if(event.key == ' '):
                self.nextStage()
            self.drawLearningGraph()
            
        connect('button_press_event',mouse_press)
        connect('key_press_event', key_press)
        show()   
