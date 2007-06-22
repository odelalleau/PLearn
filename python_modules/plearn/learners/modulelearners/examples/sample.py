from plearn.learners.modulelearners import *
import random, sys, os.path

from pygame import *
from math import *


zoom_factor = 5

def init_screen(Nim):
    init()
    width = int(sqrt(Nim*1.0))
    if width**2 != Nim:
       width = int(sqrt(Nim*1.0))+1
#       raise TypeError, "This code only deals with square images\n(and image size "+str(Nim)+" is not the square of an integer)"
    width *= zoom_factor
    return display.set_mode([width, width])


def draw_image(visible,screen):
    Nim=len(visible)
    width = int(sqrt(Nim*1.0))
#    if width**2 != Nim:
#       raise TypeError, "This code only deals with square images\n(and image size "+str(Nim)+" is not the square of an integer)"
    width *= zoom_factor
    surface = Surface((width, width),0,8)
    surface.set_palette([(i,i,i) for i in range(2**8)])
    for x in range(width/zoom_factor):
       for y in range(width/zoom_factor):
           graycol = max(min(255,int(255.0*visible[x*width/zoom_factor+y])),0)
           for i in range(zoom_factor):
               for j in range(zoom_factor):
                   surface.set_at((x*zoom_factor+i,y*zoom_factor+j),(graycol,graycol,graycol,255))
    screen.blit(surface, (0,0))
    display.update()
    return pause()
  
def pause():
   c = sys.stdin.readline()
   if c.strip() == 'q' or  c.strip() == 'x':
      sys.exit(0)
   if c.strip() == 'n' :
      return -1
   try: return int(c.strip())
   except: return 0


if __name__ == "__main__":

  if len(sys.argv) < 2:
     print "Usage:\n\t" + sys.argv[0] + " <ModuleLearner_filename> <Image_size> <dataSet_filename> [gibbs_step=<gibbs_step>]\n"
     print "Purpose:\n\tSee consecutive Gibbs sample"
     print "\twhen input visible units are initalized with real images"
     print "Tips:\n\tOnce you can see an image type"
     print "\t:    <ENTER>   : to continue Gibbs Sampling (same gibbs step)"
     print "\t: <an integer> : to change the gibbs step (10 will set the number of gibbs step between each example to 10)"
     print "\t:      n       : (next) to try with another image as initialization"
     print "\t:      q       : (quit) to stop the massacre\n"
     sys.exit()

  learner_filename = sys.argv[1]
  Nim = int(sys.argv[2])
  data_filename = sys.argv[3]
     
  plarg_defaults.gibbs_step                    = 10
  gibbs_step                                   = plargs.gibbs_step # Number of Gibbs step between each sampling

     
  if os.path.isfile(learner_filename) == False:
     raise TypeError, "Cannot find file "+learner_filename
  print " loading... "+learner_filename
  learner = loadObject(learner_filename)
  if 'HyperLearner' in str(type(learner)):
     learner=learner.learner
  
  if os.path.isfile(data_filename) == False:
     raise TypeError, "Cannot find file "+data_filename
  print " loading... "+data_filename
  dataSet = pl.AutoVMatrix( specification = data_filename )

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
  #
  # Getting the top RBMmodule
  #

  top_RBM = getTopRBMModule( learner )
  top_RBM_name = top_RBM.name
  
  NH=top_RBM.connection.up_size


  init_ports = [ ('input',  image_RBM_name+'.visible'),
                 ('output', top_RBM_name+'.hidden_sample')
               ]
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
         connections_list_up.append ( pl.NetworkConnection(source = port2moduleName( connection.source )+'.hidden_sample',
                                                           destination = port2moduleName( connection.destination )+'.visible_sample',
                                                           propagate_gradient = 0) )
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
  RBMnetworkInit = pl.NetworkModule(
                          modules = modules_list,
                          connections = connections_list_up,
                          ports = init_ports,
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
  RBMmodelInit = pl.ModuleLearner(
                              cost_ports = [],
                              target_ports = [],
                              module = RBMnetworkInit
                           )



  RBMmodelInit.setTrainingSet(pl.AutoVMatrix(inputsize=Nim, targetsize=0, weightsize=0), False)
  RBMmodel.setTrainingSet(pl.AutoVMatrix(inputsize=NH, targetsize=0, weightsize=0), False)


  screen=init_screen(Nim)
  random.seed(1969)

  for i in range(len(RBMmodel.module.modules)):
    module = RBMmodel.module.modules[i]
    if isModule( module, 'RBM'):
       RBMmodel.module.modules[i].compute_contrastive_divergence = False
       if module.name == top_RBM_name:
          RBMmodel.module.modules[i].n_Gibbs_steps_per_generated_sample = gibbs_step
          top_RBM = RBMmodel.module.modules[i]

  while True:
 
   random_index=random.randint(0,dataSet.length)
   init_image=[dataSet.getRow(random_index)[i] for i in range(Nim)]
   
   c = draw_image( init_image, screen )
   if c<0:
      continue
   elif c>0:
      top_RBM.n_Gibbs_steps_per_generated_sample = c
   
   init_hidden = RBMmodelInit.computeOutput(init_image)
   c = draw_image( RBMmodel.computeOutput(init_hidden), screen )
   if c<0:
      break
   elif c>0:
      top_RBM.n_Gibbs_steps_per_generated_sample = c
   
      
   while True:
       c = draw_image( RBMmodel.computeOutput([]) , screen)
       if c<0:
          break
       elif c>0:
          top_RBM.n_Gibbs_steps_per_generated_sample = c
    
