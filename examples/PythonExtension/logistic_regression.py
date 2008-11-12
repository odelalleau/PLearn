# logistic_regression.py
# Author: Pascal Vincent (2008)

# Simple demo using the plearn python extension module to train a simple logistic regression
# (uses NNet with no hidden layer)
# For this to work, the plearn python extension module must have been compiled:
# make_plearn_python_ext


import plearn.pyext.plext as pl
from numpy import array
from math import exp

def sigmoid(x):
    return exp(x)/(1+exp(x))


traindata = array([
    [.1,   1],
    [-2.5, 1],
    [2,  0],
    [2.5,  0]])

learner = pl.NNet(output_transfer_func = "sigmoid",
                  cost_funcs = ["stable_cross_entropy"],
                  noutputs = 1,
                  batch_size = 0,
                  optimizer = pl.ConjGradientOptimizer(),
                  nstages = 1000,
                  verbosity = 3)
trainset = pl.MemoryVMatrix(data = traindata, inputsize=1, targetsize=1, weightsize=0)
#learner.setExperimentDirectory('myexp')
learner.setTrainingSet(trainset, True)
learner.train()
params = learner.wout.value
print "Learnt parameters:",params
b, w = params.flat
print "----------"
x = 1.2
print "Computation using computeOutput:", learner.computeOutput(array([x]))
print "Computation by hand using learnt parameters:",sigmoid(w*x+b)
print "Outputs:"
print learner.computeOutputs(traindata[:,0:1])
outputs, costs = learner.computeOutputsAndCosts(traindata[:,0:1],traindata[:,1:2])
print "Costs:",costs

