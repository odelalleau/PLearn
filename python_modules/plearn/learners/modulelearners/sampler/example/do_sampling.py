#!/usr/bin/env python

from plearn.learners.modulelearners import *

from plearn.learners.modulelearners.sampler.inputweights import *
from plearn.learners.modulelearners.sampler.reconstruct import *
from plearn.learners.modulelearners.sampler.sample_from_visible import *
from plearn.learners.modulelearners.sampler.sample_from_hidden import *


import os 
PLEARNDIR = os.environ.get('PLEARNDIR', os.getcwd())
default_DIRECTORY = os.path.join(PLEARNDIR,'python_modules','plearn','learners','modulelearners','sampler','example')

learner_filename = default_DIRECTORY + '/data/DBN-3RBM.babyAI-1obj.psave'
#learner_filename = '/u/louradoj/PRGM/babyAI/convolution/expes/models/UNSUP_dbn-1RBMimage-handinit-conv_BABYAI_gray_1250000x1obj_32x32.color-size-location-shape.train.3gram01_slope1.0_N1-8-7_LRs1e-05_0.1_ns1250000_ng1/init_learner.psave'


#plarg_defaults.width         = 32
#plarg_defaults.imageSize     = int(plargs.width)**2
#plarg_defaults.data_filename = default_DIRECTORY + '/data/babyAI-1obj.dmat'

#width                    = plargs.width
#imageSize                = plargs.imageSize
#data_filename            = plargs.data_filename

data_filename = default_DIRECTORY + '/data/babyAI-1obj.dmat'
width         = 32
imageSize = width*width

if len(sys.argv)>=2:
  learner_filename = sys.argv[1]
if len(sys.argv)>=3:
  data_filename = sys.argv[2]
if len(sys.argv)>=4:
  imageSize = int(sys.argv[3])

if os.path.isfile(learner_filename) == False:
   learner_filename2=learner_filename.replace(os.path.dirname(os.path.abspath(sys.argv[0])),os.path.abspath(sys.argv[0]))
   if os.path.isfile(learner_filename2) == False:
      raise EOFError, "Cannot find file "+learner_filename
   else:
      learner_filename=learner_filename2
print " loading... "+learner_filename
learner = loadObject(learner_filename)
if 'HyperLearner' in str(type(learner)):
   learner=learner.learner

if os.path.isfile(data_filename) == False and os.path.isdir(data_filename) == False:
   data_filename2=data_filename.replace(os.path.dirname(os.path.abspath(sys.argv[0])),os.path.abspath(sys.argv[0]))
   if os.path.isfile(data_filename) == False and os.path.isdir(data_filename) == False:
      raise EOFError, "Cannot find file or directory "+data_filename
   else:
      data_filename=data_filename2
print " loading... "+data_filename
dataSet = pl.AutoVMatrix( specification = data_filename )



def check_choice(c):
    try:
       if int(c) in [1,2,3,4,EXITCODE]:
          return True
    except: pass
    return False
    
while True:

   c=None
   while check_choice(c)==False:
      print "\n---------------"
      print "-- MAIN MENU --"
      print "---------------"
      print "1. *Sample visible units*:\n   - initialization of bottom RBM visible units with (randomly picked) real input image"
      print "2. *Sample visible units*:\n   - initialization of top RBM hidden units (random binary vector)"
      print "3. *Reconstruct* some input image"
      print "4. *Visualize weights* of the 1st RBM (input)"
      print "(to quit, type 'q' or 'Q')\n"
      c = pause()

   # NOT to save the images...
   #
   save_dir=None
   #   
   # to save the images...
   #
   save_dir=None
   save_dir='~/PRGM/babyAI/pres/'+os.path.basename(os.path.dirname(learner_filename))

   if c == 1:
      view_sample_from_visible(learner, imageSize, dataSet, 1, save_dir)
#      os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/sample_from_visible.py '+' '.join([ learner_filename, str(imageSize), data_filename, 'gibbs_step=1' ]))
   elif c == 2:
      view_sample_from_hidden(learner, imageSize, 1)
#      os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/sample_from_hidden.py '+' '.join([ learner_filename, str(imageSize), 'gibbs_step=1' ]))
   elif c == 3:
      view_reconstruct( learner, imageSize , dataSet)
#      os.system('python '+os.path.dirname(os.path.abspath(sys.argv[0]))+'/reconstruct.py '+' '.join([ learner_filename, str(imageSize), data_filename]))
   elif c == 4:
      view_inputweights(learner, imageSize, save_dir)
   elif c == EXITCODE:
      break
