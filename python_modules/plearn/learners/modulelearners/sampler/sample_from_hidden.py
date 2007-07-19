from plearn.learners.modulelearners import *

zoom_factor = 5
from plearn.learners.modulelearners.sampler import *

import random

def view_sample_from_hidden(learner, Nim, init_gibbs_step):


  print "analyzing learner..."
  #
  # Getting the RBMmodule which sees the image (looking at size of the down layer)
  #
  nRBM=0 # The number of RBMs
  modules=getModules(learner)
  for i in range(len(modules)):
     module = modules[i]
     if isModule(module,'RBM'):
        nRBM += 1
        if module.connection.down_size == Nim:
           image_RBM=learner.module.modules[i]
           break
  image_RBM_name=image_RBM.name
  #
  # Getting the top RBMmodule
  #

  top_RBM = getTopRBMModule( learner )
  top_RBM_name = top_RBM.name
  
  NH=top_RBM.connection.up_size

  if nRBM == 1: MeanField=False
  else:
     print "\nChoose betweem these options:"
     print "1.[default] Gibbs sampling in the top RBM + mean field"
     print "2.                   ''                   + sample hidden->visible"
     c = pause()
     while c not in [0,1,2,EXITCODE]:
           c = pause()
     MeanField = False
     if c==1:
        MeanField = True
     elif c==EXITCODE:
        return

  if MeanField:
     ports = [ ('input', top_RBM_name+'.hidden_sample' ),
               ('output', image_RBM_name+'.visible_reconstruction.state')
             ]

  else:
     ports = [ ('input', top_RBM_name+'.hidden_sample' ),
               ('output', image_RBM_name+'.visible_expectation')
               ]

  #
  # Removing useless connections for sampling
  #
  old_connections_list = copy.copy(learner.module.connections)
  conn_toremove=[]
  connections_list_down=[]
  connections_list_up=[]
  for connection in old_connections_list:
      source_module = getModule( learner, port2moduleName( connection.source ))
      dest_module   = getModule( learner, port2moduleName( connection.destination ))
      if isModule( source_module, 'RBM') and isModule( dest_module,'RBM'):
         if MeanField:
            if dest_module.name == top_RBM_name:
               connections_list_down.append ( pl.NetworkConnection(source = port2moduleName( connection.destination )+'.visible_sample',
                                                                destination = port2moduleName( connection.source )+'.hidden.state',
                                                                propagate_gradient = 0) )
            else:
               connections_list_down.append ( pl.NetworkConnection(source = port2moduleName( connection.destination )+'.visible_reconstruction.state',
                                                                destination = port2moduleName( connection.source )+'.hidden.state',
                                                                propagate_gradient = 0) )
	 else:
            connections_list_down.append ( pl.NetworkConnection(source = port2moduleName( connection.destination )+'.visible_sample',
                                                                destination = port2moduleName( connection.source )+'.hidden_sample',
                                                                propagate_gradient = 0) )
  
  #
  # Removing useless modules for sampling
  #
  modules_list = getModules(learner)
  mod_toremove=[]
  for module in modules_list:
      if isModule( module, 'RBM') == False:
         mod_toremove.append(module)
  for module in mod_toremove:
      modules_list.remove(module)
  
  
  RBMnetwork = pl.NetworkModule(
                          modules = modules_list,
                          connections = connections_list_down,
                          ports = ports,
                          # to avoid calling the forget() method in ModuleLearner                          
                          random_gen = pl.PRandom( seed = 1827 ),
                          # Hack from Olivier
                          save_states = 0
                         )

  
  RBMmodel = pl.ModuleLearner(
                              cost_ports = [],
                              target_ports = [],
                              module = RBMnetwork
                           )


  RBMmodel.setTrainingSet(pl.AutoVMatrix(inputsize=NH, targetsize=0, weightsize=0), False)

  screen=init_screen(Nim,zoom_factor)
  random.seed(1969)

  for i in range(len(RBMmodel.module.modules)):
    module = RBMmodel.module.modules[i]
    if isModule( module, 'RBM'):
       RBMmodel.module.modules[i].compute_contrastive_divergence = False
       if module.name == top_RBM_name:
          RBMmodel.module.modules[i].n_Gibbs_steps_per_generated_sample = init_gibbs_step
          top_RBM = RBMmodel.module.modules[i]

  sample_from_hidden_man()
  while True:
 
   init_hidden=[random.randint(0,1) for i in range(NH)]
   
   c = draw_image( RBMmodel.computeOutput(init_hidden), screen, zoom_factor )
   if c==NEXTCODE:
      continue
   elif c==EXITCODE:
      return
   elif c>0:
      top_RBM.n_Gibbs_steps_per_generated_sample = c
      
   while True:
       c = draw_image( RBMmodel.computeOutput([]) , screen, zoom_factor )
       if c==NEXTCODE:
          break
       elif c==EXITCODE:
          return
       elif c>0:
          top_RBM.n_Gibbs_steps_per_generated_sample = c
    

def sample_from_hidden_man():
     print "\nPlease type:"
     print ":    <ENTER>   : to continue Gibbs Sampling (same gibbs step)"
     print ": <an integer> : to change the gibbs step (ex: 10 will set the number of gibbs step between each example to 10)"
     print ":      n       : (next) to try another hidden state for initialization"
     print ":      q       : (quit) when you are fed up\n"

if __name__ == "__main__":
  import sys, os.path

  if len(sys.argv) < 2:
     print "Usage:\n\t" + sys.argv[0] + " <ModuleLearner_filename> <Image_size> [init_gibbs_step=<init_gibbs_step>]\n"
     print "Purpose:\n\tSee consecutive Gibbs sample"
     print "\twhen top hidden units are initalized randomly"
     sample_from_hidden_man()
     sys.exit()

  learner_filename = sys.argv[1]
  Nim = int(sys.argv[2])
     
  plarg_defaults.init_gibbs_step                    = 10
  init_gibbs_step                                   = plargs.init_gibbs_step # Number of Gibbs step between each sampling

     
  if os.path.isfile(learner_filename) == False:
     raise TypeError, "Cannot find file "+learner_filename
  print " loading... "+learner_filename
  learner = loadObject(learner_filename)
  if 'HyperLearner' in str(type(learner)):
     learner=learner.learner

  view_sample_from_hidden(learner, Nim, init_gibbs_step)
