import os, os.path
import sys
import time
import datetime
import math

#from pdb import *

#from plearn.pyplearn import *
#from plearn.learners.autolr import *
from plearn.learners.modulelearners import *
   

def basename_withoutExt(name):
    return '.'.join(os.path.basename(name).split('.')[:-1])

   

##############################
# Characteristics of the data

# Default parameters values
# (see below for description)

plarg_defaults.databasePath                  = '/cluster/pauli/data/babyAI/textual_v3'
plarg_defaults.Encoding                      = '3gram01'
plarg_defaults.Size                          = 32
plarg_defaults.Nobj                          = 1 # 1, 2, 3, 4
plarg_defaults.ImageType                     = 'gray' # 'gray', 'gray_norot' (without rotation of objects)
plarg_defaults.trainNsamples                 = 10000
plarg_defaults.unsupervised_trainNsamples    = 1250000 # 10000 250000 1250000
plarg_defaults.testNsamples                  = 5000
plarg_defaults.validNsamples                 = plargs.testNsamples
plarg_defaults.Extension                     = 'vmat'

databasePath                  = plargs.databasePath                       # Directory where are the datasets'files
Encoding                      = plargs.Encoding                   # layerType of Encoding (onehot, 1gram, 2gram, 3gram, ...)
Size                          = plargs.Size                       # Image width/height
Nobj                          = plargs.Nobj                       # Number of objects in the image
ImageType                     = plargs.ImageType                  # Color encoding (gray, color) and other features (no rotation...)
trainNsamples                 = plargs.trainNsamples              # number of training samples
unsupervised_trainNsamples    = plargs.unsupervised_trainNsamples # number of training samples
testNsamples                  = plargs.testNsamples               # number of samples to test
validNsamples                 = plargs.validNsamples              # number of validation samples
Extension                     = plargs.Extension                  # extension of the input files


# Datasets filenames

unsupervised_trainFilename = databasePath+'/BABYAI_'+ImageType+'_'+str(unsupervised_trainNsamples)+'x'+str(Nobj)+'obj_'+str(Size)+'x'+str(Size)+'.color-size-location-shape.train.'+Encoding+'.'+Extension   
if os.path.isfile(unsupervised_trainFilename) == False:
   raise EOFError, "CANNOT find "+unsupervised_trainFilename

unsupervised_trainSet = pl.AutoVMatrix(
            specification = unsupervised_trainFilename
            )
validSet = unsupervised_trainSet

sys.path.append(databasePath) # the module to read sizes is in the same directory as data...
from read_sizes import *
imageSize, textSize, nClasses = read_sizes(unsupervised_trainFilename)
inputSize = imageSize + textSize

##############################

## default values, see below for descriptions

plarg_defaults.NH1                   = 500
plarg_defaults.NH2                   = 500
plarg_defaults.NH3                   = 500
plarg_defaults.batchSize             = 50
plarg_defaults.layerType             = 'gaussian'
plarg_defaults.unsupervised_nStages  = int(plargs.unsupervised_trainNsamples)
plarg_defaults.supervised_nStages    = 100  # /!\ see after: supervised_nStages   *= trainNsamples
plarg_defaults.nStagesStep           = 5
plarg_defaults.MDS                   = 10
plarg_defaults.LR_CDiv               = 0.01
plarg_defaults.LR_CDiv1              = float(plargs.LR_CDiv)
plarg_defaults.LR_CDiv12             = float(plargs.LR_CDiv)
plarg_defaults.LR_CDiv2              = float(plargs.LR_CDiv)
plarg_defaults.LR_CDiv3              = float(plargs.LR_CDiv)
plarg_defaults.LR_GRAD_UNSUP         = 0.003
plarg_defaults.LR_GRAD_UNSUP1        = float(plargs.LR_GRAD_UNSUP)
plarg_defaults.LR_GRAD_UNSUP2        = float(plargs.LR_GRAD_UNSUP)
plarg_defaults.LR_GRAD_UNSUP3        = float(plargs.LR_GRAD_UNSUP)
plarg_defaults.LR_SUP                = 0.01
plarg_defaults.L2wd_SUP              = 1e-7
plarg_defaults.seed                  = 6343
plarg_defaults.nGibbs                = 1

NH1                     = int(plargs.NH1)                        # num units for the image part
NH2                     = int(plargs.NH2)                        # num units, 2nd hid layer (image part)
NH3                     = int(plargs.NH3)                        # num units, 2rd hid layer
batchSize               = int(plargs.batchSize)                # num of samples in the minibatch
unsupervised_nStages    = int(plargs.unsupervised_nStages)                # total number of samples to see (unsupervised phase)
supervised_nStages      = int(plargs.supervised_nStages)                # total number of samples to see (supervised phase)
nStagesStep             = int(plargs.nStagesStep)                #
layerType               = plargs.layerType
LR_CDiv                 = float(plargs.LR_CDiv)                # unsup. lr
# lotsa optional learning rates that one can specify for each layer
LR_CDiv1                = float(plargs.LR_CDiv1)        # unsup. lr
LR_CDiv12               = float(plargs.LR_CDiv12)        # unsup. lr
LR_CDiv2                = float(plargs.LR_CDiv2)                # unsup. lr
LR_CDiv3                = float(plargs.LR_CDiv3)                # unsup. lr
LR_GRAD_UNSUP           = float(plargs.LR_GRAD_UNSUP)         # super. lr
# same thing with the supervised ones
LR_GRAD_UNSUP1          = float(plargs.LR_GRAD_UNSUP1) # super. lr
LR_GRAD_UNSUP2          = float(plargs.LR_GRAD_UNSUP2)         # super. lr
LR_GRAD_UNSUP3          = float(plargs.LR_GRAD_UNSUP3)        # super. lr
LR_SUP                  = float(plargs.LR_SUP) # super. lr
MDS                     = int(plargs.MDS)                        # minim. # of non-decreas. steps
seed                    = int(plargs.seed)                        
L2wd_SUP                = float(plargs.L2wd_SUP)
LR_batchfactor          = math.sqrt(int(plargs.batchSize))
nGibbs                  = int(plargs.nGibbs)
supervised_nStages *= trainNsamples

##############################

expdir =  os.environ.get('PLEARNDIR', os.getcwd())+'/python_modules/plearn/learners/modulelearners/sampler/example/data'
init_DBN_filename = expdir+'/DBN-3RBM.babyAI-1obj.psave'

##############################

def rbm_layer(layer_type,nunits):
        if layer_type=='gaussian':
                return pl.RBMGaussianLayer(        
                                        size = nunits
                                )
        else:
                return pl.RBMBinomialLayer(
                                             size = nunits
                                )

def rbm_module(name,vis_size,hid_size,lr_GRAD_UNSUP,lr_CDiv,ng,layer_type,compute_cd):
        x =  pl.RBMMatrixConnection(
                                down_size = vis_size,
                                up_size = hid_size
                        )

        return pl.RBMModule(
                        name = name,
                        visible_layer = rbm_layer(layer_type,vis_size),
                        hidden_layer = rbm_layer('binomial',hid_size),
                        connection = x,
                        reconstruction_connection = pl.RBMMatrixTransposeConnection(rbm_matrix_connection = x),
                        n_Gibbs_steps_CD = ng,
                        grad_learning_rate = lr_GRAD_UNSUP,
                        cd_learning_rate = lr_CDiv,
                        compute_contrastive_divergence = compute_cd
                )
        
        
unsupervised_modules = []
unsupervised_connections = []

##########################
unsupervised_modules.append(pl.SplitModule(
                                                        name = 'split',
                                                        down_port_name = 'input',
                                                        up_port_names = ['out1', 'out2'],
                                                        up_port_sizes = [imageSize, textSize]
                                                ))
unsupervised_modules.append( rbm_module('rbm_gaussian_inputs', imageSize, NH1, LR_GRAD_UNSUP1 * LR_batchfactor, LR_CDiv1 * LR_batchfactor, nGibbs, layerType, True ) )

unsupervised_connections.append(pl.NetworkConnection(source = 'split.out1',
                                        destination = 'rbm_gaussian_inputs.visible'))

##########################

unsupervised_modules.append( rbm_module('rbm_binomial',        NH1, NH2,   LR_GRAD_UNSUP2 * LR_batchfactor, LR_CDiv2 * LR_batchfactor, nGibbs, 'binomial', True))
unsupervised_connections.append(pl.NetworkConnection(source = 'rbm_gaussian_inputs.hidden.state',
                                        destination = 'rbm_binomial.visible'))

unsupervised_modules.append( rbm_module('rbm_binomial_top',        NH2, NH3,   LR_GRAD_UNSUP3 * LR_batchfactor, LR_CDiv3 * LR_batchfactor, nGibbs, 'binomial', True))
unsupervised_connections.append(pl.NetworkConnection(source = 'rbm_binomial.hidden.state',
                                        destination = 'rbm_binomial_top.visible'))

##########################                
unsupervised_modules.append(pl.LinearCombinationModule(
                                                        name = 'total_cost',
                                                        weights = [
                                                                   1.0 / getModule(unsupervised_modules,'rbm_gaussian_inputs').connection.down_size,
                                                                   1.0 / getModule(unsupervised_modules,'rbm_binomial').connection.down_size,
                                                                   1.0 / getModule(unsupervised_modules,'rbm_binomial_top').connection.down_size
                                                                   ]
                                                ))
unsupervised_connections.append(pl.NetworkConnection(source = 'rbm_gaussian_inputs.reconstruction_error.state',
                                        destination = 'total_cost.in_1',
                                        propagate_gradient = 1))
unsupervised_connections.append(pl.NetworkConnection(source = 'rbm_binomial.reconstruction_error.state',
                                        destination = 'total_cost.in_2',
                                        propagate_gradient = 1))
unsupervised_connections.append(pl.NetworkConnection(source = 'rbm_binomial_top.reconstruction_error.state',
                                        destination = 'total_cost.in_3',
                                        propagate_gradient = 1))

##########################
                
unsupervised_ports = [
          ('input', 'split.input'),
          ('output', 'rbm_binomial_top.hidden.state'),
          ('total_cost', 'total_cost.output'),
          ('reconstruction_error_1','rbm_gaussian_inputs.reconstruction_error.state'),
          ('reconstruction_error_2','rbm_binomial.reconstruction_error.state'),
          ('reconstruction_error_3','rbm_binomial_top.reconstruction_error.state'),
          ('contrastive_divergence_1','rbm_gaussian_inputs.contrastive_divergence'),
          ('contrastive_divergence_2','rbm_binomial.contrastive_divergence'),
          ('contrastive_divergence_3','rbm_binomial_top.contrastive_divergence')
        ]


unsupervised_module = pl.NetworkModule(
                          modules = unsupervised_modules,
                          connections = unsupervised_connections,
                          ports = unsupervised_ports
                         )
                         
unsupervised_learner = pl.ModuleLearner(
                              module = unsupervised_module,
                              cost_ports = ['total_cost', 
                                                   'reconstruction_error_1','reconstruction_error_2', 'reconstruction_error_3',
                                            'contrastive_divergence_1', 'contrastive_divergence_2', 'contrastive_divergence_3' 
                                           ],
                              target_ports = [],
                              batch_size = batchSize,
                              nstages = unsupervised_nStages,
                              expdir = expdir
                             )

unsupervised_statnames = [
                        'E[test1.E[reconstruction_error_1]]',
                        'E[test1.E[contrastive_divergence_1]]',
                        'E[test1.E[reconstruction_error_2]]',
                        'E[test1.E[contrastive_divergence_2]]',
                        'E[test1.E[reconstruction_error_3]]',
                        'E[test1.E[contrastive_divergence_3]]',
                        'E[test1.E[total_cost]]'
           ]

unsupervised_cost = unsupervised_statnames.index('E[test1.E[total_cost]]')

oracle = pl.EarlyStoppingOracle(
                      option = 'nstages',
                     values = [ str(unsupervised_nStages)], # range = [ unsupervised_nStages, unsupervised_nStages+1 ],
)
                        
unsupervised_strategy = [
        pl.HyperOptimize(
                  which_cost = str(unsupervised_cost),
                  provide_tester_expdir = 1,
                  oracle = oracle
          )
       ]
unsupervised_hyperlearner = pl.HyperLearner(
   tester = pl.PTester(
       splitter = pl.ExplicitSplitter(splitsets = TMat(1,2,[ unsupervised_trainSet, validSet])),
       statnames = unsupervised_statnames,
       save_learners = 0,
       save_initial_tester = 1,
       provide_learner_expdir = 1),
   option_fields = [ 'nstages' ],
   dont_restart_upon_change = [ 'nstages' ],
   learner = unsupervised_learner,
   strategy = unsupervised_strategy,
   provide_strategy_expdir = 1,
   save_final_learner = 0,
   expdir = expdir
   )



print "Training UNSUPERVISED hyperlearner... (seeing "+str(int(unsupervised_nStages))+" examples)\n\tin"+init_DBN_filename

if os.path.isfile(init_DBN_filename) == False:
   unsupervised_hyperlearner.train()
   print "... OK. Saving in "+init_DBN_filename
   unsupervised_learner.save(init_DBN_filename,'plearn_binary')
else:
   unsupervised_learner=loadObject(init_DBN_filename)
   print "\nWARNING: unsupervised learner already trained in:\n\t"+expdir+"\n\n"
