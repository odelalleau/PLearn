from plearn.learners.modulelearners import *

zoom_factor = 5
from plearn.learners.modulelearners.sampler import *

import sys, os.path


def view_inputweights(learner, Nim):
  
  inputweights_man()
  #
  # Getting the RBMmodule which sees the image (looking at size of the down layer)
  #
  modules=getModules(learner)
  for i in range(len(modules)):
     module = modules[i]
     if isModule(module,'RBM') and module.connection.down_size == Nim:
        image_RBM=learner.module.modules[i]
        break
  image_RBM_name=image_RBM.name

  screen=init_screen(Nim,zoom_factor)
  
  
#  if 'RBMMatrixConnection' in str(type(image_RBM.connection)):

  for i in range(len(image_RBM.connection.weights)):
    weights=image_RBM.connection.weights[i]
    print str(i+1)+"/"+str(len(image_RBM.connection.weights))
    c = draw_normalized_image( weights, screen, zoom_factor )
    if c==EXITCODE:
       return
       
#  elif 'RBMMixedConnection' in str(type(image_RBM.connection)):

def inputweights_man():
     print "\nPlease type:"
     print ":    <ENTER>   : to continue Gibbs Sampling (same gibbs step)"
     print ":      q       : (quit) when you are fed up\n"

if __name__ == "__main__":

  if len(sys.argv) < 2:
     print "Usage:\n\t" + sys.argv[0] + " <ModuleLearner_filename> <Image_size>\n"
     print "Purpose:\n\tSee weights of the RBM which sees image\n\t(i.e. with visible_size=Image_size)"
     inputweights_man()
     sys.exit()

  learner_filename = sys.argv[1]
  Nim = int(sys.argv[2])

  if os.path.isfile(learner_filename) == False:
     raise TypeError, "Cannot find file "+learner_filename
  print " loading... "+learner_filename
  learner = loadObject(learner_filename)
  if 'HyperLearner' in str(type(learner)):
     learner=learner.learner

  view_inputweights(learner, Nim)
