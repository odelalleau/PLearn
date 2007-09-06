from plearn.learners.modulelearners import *

zoom_factor = 5
from plearn.learners.modulelearners.sampler import *

import sys, os.path


def view_inputweights(learner, Nim, save_dir):

  save_image=False
  if save_dir<>None and len(save_dir)<>0:
    print "\nDo you want to save learner in the directory "+save_dir+"?"
    print "1.[default] No"
    print "2. Yes"
    c = pause()
    while c not in [0,1,2,EXITCODE]:
          c = pause()
    if c==2:
       save_image=True
    elif c==EXITCODE:
       return
       
  if save_image:
     print "\nChecking/creating directory "+save_dir+"\n"
     os.system('mkdir -p '+save_dir)
  
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
  
  zoom_factor = globals()['zoom_factor']
  
  if 'RBMMatrixConnection' in str(type(image_RBM.connection)):

    screen=init_screen(Nim,zoom_factor)
    for i in range(len(image_RBM.connection.weights)):
        weights=image_RBM.connection.weights[i]
        print str(i+1)+"/"+str(len(image_RBM.connection.weights))
        c = draw_normalized_image( weights, screen, zoom_factor )
	if save_image:
	   fname = save_dir+'/filters-%05d.jpg' % i
	   os.system('import -window "pygame window" ' + fname )
        if c==EXITCODE:
           return
       
  elif 'RBMMixedConnection' in str(type(image_RBM.connection)):

    N_filter = len(image_RBM.connection.sub_connections)
    N_inputim = len(image_RBM.connection.sub_connections[0])
    size_filter = image_RBM.connection.sub_connections[0][0].kernel.shape
    zoom_factor **= 2

#
# to complete.... see all the weights at the same time
#
#    N=math.ceil(math.sqrt(N_filter))
#    print N
#    print (size_filter[0]*N_inputim+(N_inputim-1))*N_filter
#    print size_filter[1]*N_filter
#    print zoom_factor
#    return
#    screen=init_screen( (size_filter[0]*N_inputim*N , size_filter[1]*N) , zoom_factor)
#    for i in range(N_filter):
#        X = math.fmod(N_filter,i)
#	weights = image_RBM.connection.sub_connections[i][0].kernel
#        print str(i+1)+"/"+str(N_filter)
#        for j in range(1,N_inputim):
#	   weights.resize( size_filter[0]*(j+1)*(i+1)+j*(i+1), size_filter[1]*(i+1) )
#	   weights[size_filter[0]*j*i+1:]=image_RBM.connection.sub_connections[i][j].kernel
#    draw_normalized_image( weights, screen, zoom_factor )
#    return

    screen=init_screen( (size_filter[0]*N_inputim+(N_inputim-1) , size_filter[1]) , zoom_factor)
    for i in range(N_filter):
	weights = image_RBM.connection.sub_connections[i][0].kernel
        print str(i+1)+"/"+str(N_filter)
        for j in range(1,N_inputim):
	   weights.resize( size_filter[0]*(j+1)+j, size_filter[1] )
	   weights[size_filter[0]*j]=[0]*size_filter[1]
	   weights[size_filter[0]*j+1:]=image_RBM.connection.sub_connections[i][j].kernel
        c = draw_normalized_image( weights, screen, zoom_factor )
	
	if save_image:
	   fname = save_dir+'/filters-%05d.jpg' % i
	   os.system('import -window "pygame window" ' + fname )
	
        if c==EXITCODE:
           return
	   
  else:
     raise TypeError, "sampler::view_inputweights() not yet implemented for RBM connection of type "+str(type(image_RBM.connection))


def inputweights_man():
     print "\nPlease type:"
     print ":    <ENTER>   : to continue Gibbs Sampling (same gibbs step)"
     print ":      q       : (quit) when you are fed up\n"
     print "Meaning of gray levels (g):"
     print "\tg = 127  <->  w = 0"
     print "\tg > 127  <->  w > 0"
     print "\tg < 127  <->  w < 0"
     print "\tg = 255  <->  w = +max{ -min(w), max(w) }"
     print "\tg = 0    <->  w = -max{ -min(w), max(w) }\n"

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
