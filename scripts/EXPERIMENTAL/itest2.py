import os ,sys, time, matplotlib, math, copy

from matplotlib.pylab import *
from matplotlib.colors import *
from numpy.numarray import *
# from numarray import *
#from numarray.linear_algebra import *

from plearn.io.server import *
from plearn.pyplearn import *
from plearn.plotting import *
from copy import *

from iTraining import *
from generators import *
from TLTester import *
#exec open("iTraining.py").read()
#exec open("generators.py").read()
#exec open("TLTester.py").read()

server_command = 'plearn_exp server'
serv = launch_plearn_server(command = server_command)

LINEAR = 0
LINEAR_INCREMENT = 1
UNDEFINED = -1
DEFAULT = 0
BEHAVIOR_LEARNER = 0
BEHAVIOR_GENERATOR = 1

DEFAULT_DIM = 2


generator_TRANSFORMS = []
generator_BIAS_SET = array([])
generator_NOISE_VARIANCE = 0.0001
generator_TRANSFORM_DISTRIBUTION = []

#parameters of a tree generator
generator_MODE_TREE = 0
generator_ROOT = [] #[1,1]
generator_DEEPNESS = 3
generator_BRANCHING_FACTOR = 3
generator_TREE_TRANSFORMS = []
generator_TREE_BIAS_SET = array([])
generator_TREE_NOISE_VARIANCE = 0.00001
generator_TREE_TRANSFORM_DISTRIBUTION = []

#parameters of a sequential generator
generator_MODE_SEQUENTIAL = 1
generator_START = [1,1]
generator_SEQUENCE_LENGTH = 40


#parameters of a circle generator
generator_MODE_CIRCLE = 2
generator_CENTER = [0,0]
generator_NB_CIRCLE_POINTS = 40
generator_RAY = 10

#parameters of a spiral generator
generator_MODE_SPIRAL = 3
generator_SPIRAL_ROOT = [1,1]
generator_ALPHA = 1.01
generator_THETA = 0.1
generator_NB_SPIRAL_POINTS = 40



#We suppose that the noisePrecision follows a gamma distribution
#with parameters alpha, and beta
#      (reminds that noisePrecision = 1/noiseVariance)
#
#Accorging to that distribution:
#          E(noisePrecision) = alpha/beta
#          Var(noisePrecision)=alpha/(beta^2)
#
#Given those 2 last values, we can deduce the value of alpha and beta:
#
#          beta = mean/var
#          alpha = (mean^2)/var
#
#It is what the following procedure is doing:
# -find alpha and beta,
# and then returns them in a pair
def computeNoiseVarianceParameters(mean,var):
    beta = (1.0*mean)/var
    alpha = mean*beta
    return [alpha,beta]




ALPHA = 0
BETA = 1
#learner_NOISE_PRECISION_MEAN = 1.0/0.0001
#learner_NOISE_PRECISION_VAR = 1.0
#learner_NOISE_DISTRIBUTION_PARAMETERS = (UNDEFINED,UNDEFINED)
#if(learner_NOISE_PRECISION_MEAN > 0 and learner_NOISE_PRECISION_VAR > 0):
#   learner_NOISE_DISTRIBUTION_PARAMETERS = computeNoiseVarianceParameters(learner_NOISE_PRECISION_MEAN,
#                                                                           learner_NOISE_PRECISION_VAR)
#print "noise distribution parameters: "
#print learner_NOISE_DISTRIBUTION_PARAMETERS
learnerSpec = pl.TransformationLearner(
    behavior = BEHAVIOR_LEARNER,
    transformFamily = LINEAR_INCREMENT,
    withBias = False,
    learnNoiseVariance = True,
    regOnNoiseVariance = False,
    noiseAlpha =  1,
    noiseBeta = 1,
    learnTransformDistribution = False,
    regOnTransformDistribution = False,
    transformDistributionAlpha = 2,
    noiseVariance =3.,
    transformsVariance =4.0 ,
    nbTransforms = 2,
    nbNeighbors = 2,
    initializationMode = DEFAULT)
learner = serv.new(learnerSpec)
iLearner = IterativeLearner(learner)


generatorSpec = pl.TransformationLearner(
    behavior = BEHAVIOR_GENERATOR,
    transformFamily = LINEAR_INCREMENT,
    withBias = False,
    noiseAlpha = UNDEFINED,
    noiseBeta = UNDEFINED,
    transformDistributionAlpha = UNDEFINED,
    noiseVariance = generator_NOISE_VARIANCE,
    transformsVariance = 1,
    nbTransforms = 1,
    nbNeighbors = 1)

gen = serv.new(generatorSpec)


generatorMode = generator_MODE_CIRCLE

#generator = TreeGenerator(gen,
 #                         DEFAULT_DIM,
  #                        False,
   #                       generator_DEEPNESS,
    #                      generator_BRANCHING_FACTOR,
     #                     generator_ROOT)
#generator= SequentialGenerator(gen,
 #                              DEFAULT_DIM,
  #                             False,
   #                            generator_SEQUENCE_LENGTH,
    #                           generator_START
     #                          )

generator = CircleGenerator(gen,
                            False,
                            generator_NB_CIRCLE_POINTS,
                            generator_CENTER,
                            generator_RAY)
#generator = SpiralGenerator(gen,
 #                           False,
  #                          generator_NB_SPIRAL_POINTS,
   #                         generator_SPIRAL_ROOT,
    #                        generator_ALPHA,
     #                       generator_THETA)

tester = TLTester(iLearner,generator)
if(generator_NOISE_VARIANCE > 0):
    generator.setNoiseVariance(generator_NOISE_VARIANCE)
tester.run()
