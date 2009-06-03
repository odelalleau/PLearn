import sys
from plearn.pyplearn import *
from plearn.var.Var import *

datadir = "DBDIR:"

# Format: [ inputsize, targetsize, nclasses, trainset, validset, testset ]


def listDatasets():
    """Returns the list of all classification tasks available with getDatasets(datasetname)"""
    return all_data_sets.keys()

def getDatasets(datasetname, ntrain=None, nvalid=None, ntest=None):
    """Ex: getDataset("mnist_bg")
    will return a tuple: inputsize, targetsize, nclasses, trainset, validset, testset
    where the sets are VMatrix specifications.
    If ntrain, nvalid, ntest are left unspecified (or None) then the full standard sets
    will be used. If they are specified, then a subset with that many examples will be used.
    """
    inputsize, targetsize, nclasses, trainset, validset, testset = all_data_sets[datasetname]

    if ntrain is not None:
        trainset = pl.RowsSubVMatrix(source=trainset, startrow=0, length=ntrain)
    if nvalid is not None:
        validset = pl.RowsSubVMatrix(source=validset, startrow=0, length=nvalid)
    if ntest is not None:
        testset = pl.RowsSubVMatrix(source=testset, startrow=0, length=ntest)
        
    return inputsize, targetsize, nclasses, trainset, validset, testset


mnist_small = [ 784, 1, 10,
                pl.FileVMatrix(filename=datadir+"mnist/mnist_small/mnist_basic2_train.pmat",
                               inputsize = 784,
                               targetsize = 1,
                               weightsize = 0),
                pl.FileVMatrix(filename=datadir+"mnist/mnist_small/mnist_basic2_valid.pmat",
                               inputsize = 784,
                               targetsize = 1,
                               weightsize = 0),
                pl.FileVMatrix(filename=datadir+"mnist/mnist_small/mnist_basic2_test.pmat",
                               inputsize = 784,
                               targetsize = 1,
                               weightsize = 0)
                ]

mnist_bg = [ 784, 1, 10, 
             pl.AutoVMatrix(filename=datadir+"icml07data/mnist_background_images/plearn/mnist_all_background_images_train.amat",
                            inputsize=784,
                            targetsize = 1,
                            weightsize = 0),
             pl.AutoVMatrix(filename=datadir+"icml07data/mnist_background_images/plearn/mnist_all_background_images_valid.amat",
                              inputsize=784,
                              targetsize = 1,
                              weightsize = 0),
             pl.AutoVMatrix(filename=datadir+"icml07data/mnist_background_images/plearn/mnist_all_background_images_big_test.amat",
                              inputsize=784,
                              targetsize = 1,
                              weightsize = 0)
             ]


mnist_rot = [ 784, 1, 10,
              pl.AutoVMatrix(filename=datadir+"icml07data/mnist_rotations/plearn/mnist_all_rotation_normalized_float_train.amat",
                              inputsize=784,
                              targetsize = 1,
                              weightsize = 0),
              pl.AutoVMatrix(filename=datadir+"icml07data/mnist_rotations/plearn/mnist_all_rotation_normalized_float_valid.amat",
                              inputsize=784,
                              targetsize = 1,
                              weightsize = 0),
              pl.AutoVMatrix(filename=datadir+"icml07data/mnist_rotations/plearn/mnist_all_rotation_normalized_float_test.amat",
                              inputsize=784,
                              targetsize = 1,
                              weightsize = 0)
              ]


# mnist_full

mnist_all_examples = pl.FileVMatrix(filename=datadir+"mnist/mnist_all.pmat",
                                    inputsize = 784,
                                    targetsize = 1,
                                    weightsize = 0)

mnist_full = [ 784, 1, 10,
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=0, length=50000),
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=50000, length=10000),
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=60000, length=10000) ]


# mnist_tiny

mnist_tiny = [ 784, 1, 10,
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=0, length=100),
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=100, length=30),
               pl.RowsSubVMatrix(source=mnist_all_examples, startrow=130, length=30) ]

# babyAI shape

babyAIshape = [ 1024, 1, 3,
                pl.AutoVMatrix(filename=datadir+"babyAI/curriculum/shapeset1_1cspo_2_3.10000.train.shape.vmat",
                               inputsize=1024,
                               targetsize = 1,
                               weightsize = 0),
                pl.AutoVMatrix(filename=datadir+"babyAI/curriculum/shapeset1_1cspo_2_3.5000.valid.shape.vmat",
                               inputsize=1024,
                               targetsize = 1,
                               weightsize = 0),
                pl.AutoVMatrix(filename=datadir+"babyAI/curriculum/shapeset1_1cspo_2_3.5000.test.shape.vmat",
                               inputsize=1024,
                               targetsize = 1,
                               weightsize = 0)
                ]


# babyAIshape normalized

babyAIshape_norm_all_examples = pl.AutoVMatrix(filename=datadir+"babyAI/curriculum/shapeset1_1cspo_2_3.20000.all.shape.vmat",
                                      inputsize=1024,
                                      targetsize = 1,
                                      weightsize = 0)

babyAIshape_norm = [ 1024, 1, 3,
                     pl.RowsSubVMatrix(source=babyAIshape_norm_all_examples, startrow=0, length=10000),
                     pl.RowsSubVMatrix(source=babyAIshape_norm_all_examples, startrow=10000, length=5000),
                     pl.RowsSubVMatrix(source=babyAIshape_norm_all_examples, startrow=15000, length=5000) ]


all_data_sets = {
    "mnist_small" : mnist_small,
    "mnist_bg" : mnist_bg,
    "mnist_rot" : mnist_rot,
    "mnist_full" : mnist_full,
    "mnist_tiny" : mnist_tiny,
    "babyAIshape" : babyAIshape,
    "babyAIshape_norm" : babyAIshape_norm,    
    }

