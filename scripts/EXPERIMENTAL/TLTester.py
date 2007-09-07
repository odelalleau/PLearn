import os ,sys, time, matplotlib, math, copy
from numpy import *
from matplotlib.pylab import *
from matplotlib.colors import *

from plearn.io.server import *
from plearn.pyplearn import *
from plearn.plotting import *
from copy import *

from generators import *

UNDEFINED = -1
EMPTY_MAT = TMat()

class TLTester(object):
    
    #iLearner = UNDEFINED
    #generator = UNDEFINED
    #dim = 2
    
    def __init__(self,
                 iLearner,
                 generator):
        assert(2*generator.nbTransforms == iLearner.nbTransforms)
        assert(generator.dim == iLearner.dim)
        self.dim = generator.dim
        self.iLearner = iLearner
        self.generator = generator


    def biasAreNull(self,
                    biasSet):
        if(type(biasSet)==type(EMPTY_MAT)):
            if(biasSet.nrows == 0 and biasSet.ncols == 0):
                return True
            else:
                return False
        elif(len(biasSet)==0):
            return True
        else:
            for i in range(biasSet.shape[0]):
                for j in range(biasSet.shape[1]):
                    if(biasSet[i][j] != 0):
                        return false
            return True


            
        
    def linearToLinearIncrement(self,transform,bias):
        newTransform = copy(transform)
        newBias = copy(bias)
        for i in range(newTransform.shape[0]):
            newTransform[i][i] = newTransform[i][i] - 1;
        return (newTransform, newBias)

    def linearIncrementToLinear(self,transform,bias):
        newTransform = copy(transform)
        newBias = copy(bias)
        for i in range(newTransform.shape[0]):
            newTransform[i][i] = newTransform[i][i] + 1;
        return (newTransform,newBias)
    
    
    def inverseTransformation(self,transform,bias,transformFamily):
        if(transformFamily == LINEAR):
            invTransform = self.zeroMatrix(self.dim,self.dim)
            inv = inverse(transform)
            for i in range(self.dim):
                for j in range(self.dim):
                    invTransform[i,j]=inv[i,j]
            if(len(bias) >0):
                invBias = -1*dot(invTransform ,bias)
            else:
                invBias = []
            return (invTransform,invBias)
        else:
            id = identity(len(transform))
            invTransform = zeros((self.dim,self.dim))
            inv = inverse(transform + id)
            for i in range(self.dim):
                for j in range(self.dim):
                    invTransform[i][j]=inv[i][j]
            if(len(bias) > 0):
                invBias = -1*dot(invTransform,bias)
            else:
                invBias = []
            invTransform = invTransform - id
            return (invTransform,invBias)


    def conversionGeneratorToLearner(self,transform,bias):
        if(self.generator.transformFamily == self.iLearner.transformFamily):
            newTransform = copy(transform)
            newBias = copy(bias)
            return(newTransform,newBias)
        elif(self.generator.transformFamily == LINEAR):
            return self.linearToLinearIncrement(transform,bias)
        else:
            return self.linearIncrementToLinear(transform,bias)


    def prepareILearner(self):
        self.transmitParametersToLearn()
        self.transmitDataSet()

        
    def transmitDataSet(self):
        self.iLearner.setTrainingSet(self.generator.newDataSet())


         
    

    def transmitParametersToLearn(self):
        #transformations: 
        transformsToLearn = []
        temp = []
        biasToLearn = array(zeros(self.iLearner.nbTransforms*self.dim), 'd')
        biasToLearn.resize(self.iLearner.nbTransforms,self.dim)
        
        if(not self.iLearner.withBias and self.generator.withBias):
            assert(self.biasAreNull(self.generator.biasSet))
        K = self.generator.nbTransforms
        for i in range(K):
            if(self.generator.withBias):
                biasToLearn[i] = copy(self.generator.biasSet[i,:])
            (t,b)=self.conversionGeneratorToLearner(self.generator.transforms[i],biasToLearn[i])
            transformsToLearn.append(t.copy())
            biasToLearn[i]=copy(b)
            (t,b)= self.inverseTransformation(t,                                       
                                              b,
                                              self.iLearner.transformFamily)
            temp.append(t.copy())
            biasToLearn[i + K]=copy(b)
        for i in range(K):
            transformsToLearn.append(temp[i])
            
    
        
        #noise variance
        noiseVariance = UNDEFINED
        if(self.iLearner.learnNoiseVariance):
            noiseVariance=self.generator.noiseVariance
        #transform distribution
        transformDistribution = []
        if(self.iLearner.learnTransformDistribution):
            transformDistribution = [0.0,0.0]
            for i in range(self.generator.nbTransforms):
                p = 0.5*exp(self.generator.transformDistribution[i])
                transformDistribution[i]=log(p)
                transformDistribution[i + self.generator.nbTransforms] = log(p)
        #transmission
        self.iLearner.setParametersToLearn(transformsToLearn,
                                      biasToLearn,
                                      noiseVariance,
                                      transformDistribution)
            
    def run(self):
        self.prepareILearner()
        self.iLearner.run()
        
            
            
