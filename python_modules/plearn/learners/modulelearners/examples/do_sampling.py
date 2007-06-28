from plearn.learners.modulelearners import *

def basename_withoutExt(name):
    return '.'.join(os.path.basename(name).split('.')[:-1])


####################################
## Choosing the model to generate ##
####################################


plarg_defaults.gibbs_step                    = 1
gibbs_step                                   = plargs.gibbs_step # Number of Gibbs step between each sampling

##############################
# Characteristics of the data

plarg_defaults.Path                          = '/cluster/pauli/data/babyAI/textual_v3'
plarg_defaults.Encoding                      = '3gram01'
plarg_defaults.Topics                        = 'shape' #, 'color', location', 'size', 
plarg_defaults.Size                          = 32
plarg_defaults.Nobj                          = 1 # 1, 2, 3, 4
plarg_defaults.ImageType                     = 'gray' # 'gray', 'gray_norot' (without rotation of objects)
plarg_defaults.trainNsamples                 = 10000
plarg_defaults.unsupervised_trainNsamples    = 250000 # 10000 250000 1250000
plarg_defaults.testNsamples                  = 5000
plarg_defaults.validNsamples                 = plargs.testNsamples
plarg_defaults.Extension                     = 'vmat'

Path                          = plargs.Path                       # Directory where are the datasets'files
Encoding                      = plargs.Encoding                   # layerType of Encoding (onehot, 1gram, 2gram, 3gram, ...)
Topics                        = plargs.Topics                     # Question topic (color, color-size, color-size-location, color-size-location-shape, ...)
Size                          = plargs.Size                       # Image width/height
Nobj                          = plargs.Nobj                       # Number of objects in the image
ImageType                     = plargs.ImageType                  # Color encoding (gray, color) and other features (no rotation...)
trainNsamples                 = plargs.trainNsamples              # number of training samples
unsupervised_trainNsamples    = plargs.unsupervised_trainNsamples # number of training samples
testNsamples                  = plargs.testNsamples               # number of samples to test
validNsamples                 = plargs.validNsamples              # number of validation samples
Extension                     = plargs.Extension                  # extension of the input files


plarg_defaults.nRBM                     = 3
plarg_defaults.NH1                      = 500
plarg_defaults.NH2                      = 500
plarg_defaults.NH3                      = 500
plarg_defaults.batchSize                = 50
plarg_defaults.layerType                = 'gaussian'
plarg_defaults.unsupervised_nStages     = int(unsupervised_trainNsamples)
plarg_defaults.supervised_nStages       = 100  # /!\ see after: supervised_nStages   *= trainNsamples
plarg_defaults.nStagesStep              = 5
plarg_defaults.LR_CDiv                  = 0.01
plarg_defaults.LR_CDiv1		        = float(plargs.LR_CDiv)
plarg_defaults.LR_CDiv2			= float(plargs.LR_CDiv)
plarg_defaults.LR_CDiv3			= float(plargs.LR_CDiv)
plarg_defaults.LR_GRAD_UNSUP            = 0.003
plarg_defaults.LR_GRAD_UNSUP1		= float(plargs.LR_GRAD_UNSUP) 
plarg_defaults.LR_GRAD_UNSUP2 		= float(plargs.LR_GRAD_UNSUP) 
plarg_defaults.LR_GRAD_UNSUP3 		= float(plargs.LR_GRAD_UNSUP)	
plarg_defaults.LR_SUP                   = 0.
plarg_defaults.L2wd_SUP                 = 1e-5
plarg_defaults.nGibbs                   = 1
plarg_defaults.Tag                      = 'dbn-'+str(plargs.nRBM)+'RBMimage'

# Network structure
nRBM          = int(plargs.nRBM)
NH1           = int(plargs.NH1)                        # num units for the image part
NH2           = int(plargs.NH2)                        # num units, 2nd hid layer (image part)
NH3           = int(plargs.NH3)                        # num units, 2rd hid layer
layerType                = plargs.layerType
#
# Network learning parameters
batchSize                 = int(plargs.batchSize)                # num of samples in the minibatch
unsupervised_nStages      = int(plargs.unsupervised_nStages)                # total number of samples to see (unsupervised phase)
supervised_nStages        = int(plargs.supervised_nStages)                # total number of samples to see (supervised phase)
supervised_nStages       *= trainNsamples
nStagesStep               = int(plargs.nStagesStep)                #
LR_CDiv                   = float(plargs.LR_CDiv)                # unsup. lr
LR_CDiv1                   = float(plargs.LR_CDiv1)                # unsup. lr
LR_CDiv2                   = float(plargs.LR_CDiv2)                # unsup. lr
LR_CDiv3                   = float(plargs.LR_CDiv3)                # unsup. lr
LR_GRAD_UNSUP             = float(plargs.LR_GRAD_UNSUP)         # super. lr
LR_GRAD_UNSUP1             = float(plargs.LR_GRAD_UNSUP1)         # super. lr
LR_GRAD_UNSUP2             = float(plargs.LR_GRAD_UNSUP2)         # super. lr
LR_GRAD_UNSUP3             = float(plargs.LR_GRAD_UNSUP3)         # super. lr
LR_SUP                    = float(plargs.LR_SUP) # super. lr
L2wd_SUP                  = float(plargs.L2wd_SUP)
nGibbs                     = int(plargs.nGibbs)
#
# Other
Tag                     = plargs.Tag


BaseDir =  '/u/louradoj/PRGM/blocksworld/res/'+os.path.basename(Path)
data_filename = Path+'/BABYAI_'+ImageType+'_'+str(unsupervised_trainNsamples)+'x'+str(Nobj)+'obj_'+str(Size)+'x'+str(Size)+'.'+Topics+'.train.'+Encoding+'.'+Extension
BaseTag = Tag+'_'+layerType+'_'+basename_withoutExt(data_filename)


unsupervised_expdir = BaseDir + '/greedyDBN/UNSUP_' + BaseTag + '_'+layerType+''.join(['_N'+str(i)+'-'+str(globals()['NH'+str(i)]) for i in range(1,nRBM+1)])+'_LRs'+'-'.join([str(globals()['LR_CDiv'+str(i)]) for i in range(1,nRBM+1)])+'_'+'-'.join([str(globals()['LR_GRAD_UNSUP'+str(i)]) for i in range(1,nRBM+1)])+'_ns'+str(unsupervised_nStages)+'_ng'+str(nGibbs) 
finetuning_expdir = BaseDir + '/greedyDBN/FINETUNE_' + BaseTag + '_'+layerType+''.join(['_N'+str(i)+'-'+str(globals()['NH'+str(i)]) for i in range(1,nRBM+1)])+'_LRs'+'-'.join([str(globals()['LR_CDiv'+str(i)]) for i in range(1,nRBM+1)])+'_'+'-'.join([str(globals()['LR_GRAD_UNSUP'+str(i)]) for i in range(1,nRBM+1)])+'-'+str(LR_SUP)+'_WD'+str(L2wd_SUP)+'_ns'+str(unsupervised_nStages)+'-'+str(supervised_nStages)+'_ng'+str(nGibbs)



if len(sys.argv) == 1:
   learner_filename = '/u/louradoj/PRGM/blocksworld/res/textual_v3/greedyDBN/FINETUNE_dbn-3RBMimage_gaussian_BABYAI_gray_1250000x1obj_32x32.color-size-location-shape.train.3gram01_gaussian_N1-500_N2-500_N3-500_LRs0.01-0.01-0.01_0.003-0.003-0.003-0.03_WD1e-05_ns2500000-1000000_ng1'+"/Split0/final_learner.psave"
   learner_filename = '/u/louradoj/PRGM/blocksworld/res/textual_v3/greedyDBN/UNSUP_dbn-3RBMimage_gaussian_BABYAI_gray_1250000x1obj_32x32.color-size-location-shape.train.3gram01_gaussian_N1-500_N2-500_N3-500_LRs0.01-0.01-0.01_0.003-0.003-0.003_ns1250000_ng1'+"/init_learner.psave"
   learner_filename = '/u/louradoj/PRGM/blocksworld/res/textual_v3/greedyDBN/FINETUNE_dbn-3RBMimage_gaussian_BABYAI_gray_1250000x1obj_32x32.color-size-location-shape.train.3gram01_gaussian_N1-500_N2-500_N3-500_LRs0.01-0.01-0.01_0.01-0.01-0.01-0.03_WD1e-05_ns5000000-1000000_ng1'+"/Split0/final_learner.psave"
   learner_filename = '/u/louradoj/PRGM/blocksworld/res/textual_v3/greedyDBN/UNSUP_dbn-3RBMimage_gaussian_BABYAI_gray_1250000x1obj_32x32.color-size-location-shape.train.3gram01_gaussian_N1-500_N2-500_N3-500_LRs0.01-0.01-0.01_0.01-0.01-0.01_ns5000000_ng1'+"/init_learner.psave"
elif len(sys.argv) == 2:
   basename=os.path.basename(sys.argv[1])
   if basename[0:5] == 'UNSUP':
      print "UNSUPERVISED-way trained model"
      learner_filename = sys.argv[1]+"/init_learner.psave"
   elif basename[0:5] == 'FINET':
      print "supervised FINETUNED model"
      learner_filename = sys.argv[1]+"/Split0/final_learner.psave"
   else:
      raise TypeError, "Cannot recognize "+basename[0:5]+" in "+sys.argv[1]
elif LR_SUP==None or LR_SUP == 0:
   print "UNSUPERVISED-way trained model"
   learner_filename = unsupervised_expdir+"/init_learner.psave"
else:
   print "supervised FINETUNED model"
   learner_filename = finetuning_expdir+"/Split0/final_learner.psave"


def check_choice(c):
    try:
       if int(c) in [1,2,3,4]:
          return True
    except: pass
    return False


c=None
while check_choice(c)==False:
   print "Type the number corresponding to your choice :"
   print "1. Sample after having initialized input visible units with (randomly picked) real image"
   print "2. Sample after having initialized top hidden units with a random binary vector"
   print "3. Reconstruct the input image"
   print "4. Visualize input weights"
   c = sys.stdin.readline()

if int(c) == 1:
   os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/sample.py '+' '.join([ learner_filename, str(Size*Size), data_filename, 'gibbs_step='+str(gibbs_step) ]))
elif int(c) == 2:
   os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/sample_from_hidden.py '+' '.join([ learner_filename, str(Size*Size), 'gibbs_step='+str(gibbs_step) ]))
elif int(c) == 3:
   os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/reconstruct.py '+' '.join([ learner_filename, str(Size*Size), data_filename]))
elif int(c) == 4:
   os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/inputweights.py '+' '.join([ learner_filename, str(Size*Size)]))
