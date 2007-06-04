import sys, os, time
#from numarray import *
from math import *
from libsvm import *

	     
class SVM_expert(object):
      __attributes__ = ['kernel_type',
			'parameters_names',
			'tried_parameters',
			'best_parameters',
			'accuracy'
			]
			
      def __init__( self ):
	  self.parameters_names = ['C']
	  self.tried_parameters = {}
	  self.best_parameters  = None
	  self.accuracy         = 0

      def reset(self):
          self.tried_parameters = {}
	  #if self.best_parameters != None:
	  #   self.add_parameter_to_tried_list(self.getBestValue('C'), self.best_parameters[1:])
	  self.accuracy         = 0
	  
      def get_svm_parameter( self, parameters ):
          s= ', '.join([ self.parameters_names[i]+' = '+str(parameters[i]) for i in range(len(self.parameters_names)) ])
	  return eval('svm_parameter( svm_type = C_SVC, kernel_type = '+self.kernel_type+', '+s+')')

      def add_parameter_to_tried_list(self, C, kernel_parameters):
          if self.tried_parameters.has_key(kernel_parameters):
	     self.tried_parameters[kernel_parameters]+=[C]
	  else:
	     self.tried_parameters[kernel_parameters] =[C] 
	     
      def init_C(self, C_value):
          return [C_value/10., C_value, C_value*10.]

      def init_parameters(self, kernel_parameters_list):
	  C_base = self.init_C(10.)
          table  = []
          for C in C_base:
	      for prm in kernel_parameters_list:
	          table.append( parameters2list(C, prm) )
		  self.add_parameter_to_tried_list(C, prm)
          return table

      def choose_new_parameters(self, kernel_parameters):
          if self.tried_parameters.has_key(kernel_parameters):
             C_table = choose_new_parameters_geom( self.tried_parameters[kernel_parameters], self.getBestValue('C') )
	  else:
	     C_table = self.init_C( self.best_parameters[0] )
          table  = []
          for C in C_table:
	          table.append( parameters2list(C, kernel_parameters) )
		  self.add_parameter_to_tried_list(C, kernel_parameters)
          return table
	  
      def getBestValue(self, name_prm):
          return self.best_parameters[ self.parameters_names.index(name_prm) ]
	  

class RBF_expert(SVM_expert):
      def __init__( self ):
          SVM_expert.__init__( self )
          self.kernel_type       = 'RBF'
	  self.parameters_names += ['gamma']

      def init_gamma(self, gamma):
          return [gamma/9., gamma, gamma*9.]
	  
      def init_parameters( self, samples ):
          dim = len(samples[0])
	  std = mean_std(samples)
	  rho=sqrt(dim)*std
          gamma0 = 1/(2*rho**2)
	  gamma_base = self.init_gamma(gamma0)
	  return SVM_expert.init_parameters( self, gamma_base )
	  
      def choose_new_parameters( self ):
	  best_gamma = self.getBestValue('gamma') 
	  tried_gamma = self.tried_parameters
	  if tried_gamma.has_key(best_gamma):
	     if self.getBestValue('C') == self.tried_parameters[best_gamma][0] or self.getBestValue('C') == self.tried_parameters[best_gamma][len(self.tried_parameters[best_gamma])-1]:
	        return SVM_expert.choose_new_parameters(self, best_gamma)
	     else:
	        proposed_gammas = choose_new_parameters_geom( tried_gamma, best_gamma)
	  else:
	     proposed_gammas = self.init_gamma(best_gamma)
          return SVM_expert.init_parameters(self, proposed_gammas)

class POLY_expert(SVM_expert):
      def __init__( self ):
          SVM_expert.__init__( self )
          self.kernel_type       = 'POLY'
	  self.parameters_names += ['degree','coef0']

      def init_degree(self, degree):
          return [  (degree-1,1), (degree,1), (degree+1,1) ]

      def init_parameters( self, samples ):
          #return SVM_expert.init_parameters(self, self.init_degree(3) )
          return SVM_expert.init_parameters(self, [ (2,1), (3,1), (4,1) ] )
	  
      def choose_new_parameters( self ):
          best_degree = self.getBestValue('degree')
          tried_degrees = [prms[0] for prms in self.tried_parameters]
	  if self.tried_parameters.has_key(best_degree):
             if best_degree == max(tried_degrees):
	        return SVM_expert.init_parameters(self, [(best_degree+1,1)])
	     else:
                return SVM_expert.choose_new_parameters(self, (self.best_parameters[1], self.best_parameters[2])  )
          else:
	     return SVM_expert.init_parameters(self, self.init_degree(best_degree) )
	     
class LINEAR_expert(SVM_expert):
      def __init__( self ):
          SVM_expert.__init__( self )
          self.kernel_type = 'LINEAR'

      def init_parameters( self, samples ):
          return SVM_expert.init_parameters(self, [None])

      def choose_new_parameters( self ):
          return SVM_expert.choose_new_parameters(self, None)


class discr_power_SVM_eval(object):

      __attributes__ = ['accuracy',
                        'valid_accuracy',
			'best_parameters',
			'tried_parameters'
			]
       
      def __init__( self ):
      
	  self.accuracy       = 0
	  self.valid_accuracy = 0
	  
	  self.LINEAR_expert  = LINEAR_expert()
	  self.RBF_expert     = RBF_expert()
	  self.POLY_expert    = POLY_expert()
	  
	  self.best_parameters      = None  
	  self.tried_parameters     = {}
	  
          # For cross-validation
	  self.nr_fold        = 5

      def reset( self ):
          self.LINEAR_expert.reset()
	  self.RBF_expert.reset()
	  self.POLY_expert.reset()
	  
          self.tried_parameters = {}
	  #if self.best_parameters != None:
	  #   self.add_parameter_to_tried_list(self.best_parameters[0], self.best_parameters[1:])
	  self.accuracy       = 0
	  self.valid_accuracy = 0

      def add_parameter_to_tried_list(self, kernel, kernel_parameters):
          if self.tried_parameters.has_key(kernel):
	     self.tried_parameters[kernel]+=[kernel_parameters]
	  else:
	     self.tried_parameters[kernel] =[kernel_parameters] 


      def valid_and_compute_accuracy(self, kernel_type, samples_target_list):
	  check_samples_target_list(samples_target_list)
	  
	  expert = eval( 'self.'+kernel_type+'_expert' )
	  
	  if len(expert.tried_parameters) == 0:
	     recompute_best = True
	  else:
	     recompute_best = False

          if expert.best_parameters  == None:
	     parameters_to_try = expert.init_parameters( samples_target_list[0][0] )
	  else:
	     parameters_to_try = expert.choose_new_parameters()
	  
	  best_parameters = expert.best_parameters
	  best_accuracy   = expert.accuracy

          for parameters in parameters_to_try:
	      if parameters != expert.best_parameters or recompute_best:
	      
		  self.add_parameter_to_tried_list(kernel_type, parameters)
	          param = expert.get_svm_parameter( parameters )
		  
	          if len(samples_target_list) == 1: # cross-validation
		     accuracy = do_cross_validation(samples_target_list[0][0], samples_target_list[0][1], param, self.nr_fold)
		  else:
		     train_problem = svm_problem( samples_target_list[0][1] , samples_target_list[0][0] )
		     model = svm_model(train_problem, param)
		     accuracy = do_simple_validation(model, samples_target_list[1][0], samples_target_list[1][1], param)
		     	     
		  if accuracy > best_accuracy:
		         best_parameters = parameters
		         best_accuracy = accuracy
			 if len(samples_target_list) == 3:
			    best_model = model

          if best_accuracy > expert.accuracy:
	     expert.best_parameters = best_parameters
	     expert.accuracy = best_accuracy
	     
	     if best_accuracy > self.valid_accuracy:
		self.best_parameters = [kernel_type, best_parameters]
		self.valid_accuracy = best_accuracy
	        if len(samples_target_list) == 3: # train-valid-test
	           best_param = expert.get_svm_parameter( best_parameters )  
                   self.accuracy = do_simple_validation(best_model, samples_target_list[2][0], samples_target_list[2][1], best_param)
	        else:
		   self.accuracy = self.valid_accuracy
	  
	  return self.accuracy
	  
	  #self.clerror  = 100 - self.accuracy

      def compute_accuracy(self, samples_target_list):
	  best_expert = eval( 'self.'+self.best_parameters[0]+'_expert' )
	  best_parameters = best_expert.best_parameters[1:]
	  param = best_expert.get_svm_parameter( best_parameters )
	  if len(samples_target_list) == 1: # cross-validation
	     accuracy = do_cross_validation(samples_target_list[0][0], samples_target_list[0][1], param, self.nr_fold)
	  else:
	     train_problem = svm_problem( samples_target_list[0][1], samples_target_list[0][0] )
	     model = svm_model(train_problem, param)
	     accuracy = do_simple_validation(model, samples_target_list[1][0], samples_target_list[1][1], param)
	  return accuracy
    
def mean_std(data):
    stds=[get_std_cmp(data,i) for i in range(len(data[0]))]
    return sum(stds)/len(stds)
def get_std_cmp(data,i):
    values=[vec[i] for vec in data]
    tot = sum(values)
    avg = tot*1.0/len(values)
    sdsq = sum([(i-avg)**2 for i in values])
    return (sdsq*1.0/(len(values)-1 or 1))**.5

def arithm_mean(data):
    if type(data[0]) == list:
       return [sum( [data[i][coor] for i in range(len(data))] )*1.0/len(data) for coor in range(len(data[0]))]
    else:
       return sum(data)*1.0/len(data)
def geom_mean(data):
    if type(data[0]) == list:
       res=[]
       for coor in range(len(data[0])):
           res.append( geom_mean( [data[i][coor] for i in range(len(data))] ) )
       return res
    else:
       prod = 1.0
       for value in data:
           prod *= value
       return prod**(1.0/len(data))

def choose_new_parameters_geom( table, best_value ):
    sorted_table = sorted(table)
    if best_value == sorted_table[0]:
       # smallest value
       proposed_table = [ best_value*1.1*best_value/sorted_table[1],
                          #best_value,
		          geom_mean([sorted_table[1],best_value]) ]
    elif best_value == sorted_table[len(table)-1]:
       # largest value
       proposed_table = [ geom_mean([sorted_table[len(table)-2],best_value]), 
                          #best_value,
		          best_value*0.9*best_value/sorted_table[len(table)-2] ]
    else:
       # middle value (best case: dichotomie)
       if best_value not in sorted_table:
          raise TypeError, "in RBF.choose_new_parameters: "+str(best_value)+" not found in tried_parameters"
       index = sorted_table.index(best_value)
       proposed_table = [ geom_mean([sorted_table[index-1],best_value]),
                          #best_value,
		          geom_mean([sorted_table[index+1],best_value]) ]
    return proposed_table

def do_cross_validation(samples, targets, param, nr_fold):
	prob_l = len(targets)
	total_correct = 0
	prob = svm_problem(targets, samples)
    	outputs = cross_validation(prob, param, nr_fold)
	for i in range(prob_l):
	    if outputs[i] == targets[i]:
	       total_correct = total_correct + 1 
	return (100.0 * total_correct / prob_l)

def do_cross_validation(samples, targets, param, nr_fold):
        n_train = int( len(samples)/nr_fold )
        train_samples = samples[0:n_train]
	train_targets = targets[0:n_train]
	test_samples = samples[n_train:]
	test_targets = targets[n_train:]
        train_problem = svm_problem( train_targets, train_samples )
	model = svm_model(train_problem, param)
	return do_simple_validation(model, test_samples, test_targets, param)	     
	
def do_simple_validation(model, samples, targets, param):
	prob_l = len(targets)
	total_correct = 0
	for i in range(prob_l):
	    if model.predict(samples[i]) == targets[i]:
	       total_correct = total_correct + 1
	return (100.0 * total_correct / prob_l)

def check_samples_target_list(samples_target_list):
          if type(samples_target_list) != list or len(samples_target_list) == 0 or type(samples_target_list[0]) != list:
	     raise TypeError, "ERROR: samples_target_list must be a list of list (of arrays)"
          else:
	     for samples_target in samples_target_list:
	         if len(samples_target) != 2:
	            raise TypeError, "ERROR: samples_target_list has an element with length "+str(len(samples_target))+" (instead of 2)"
	  if len(samples_target_list) == 1:
	     print "cross-validation"
	     return
	  elif len(samples_target_list) == 2:
	     print "simple validation"
	     return
	  elif len(samples_target_list) == 3:
	     print "validation + test"
	     return
	  else:
	      raise TypeError, "ERROR: samples34_target_list have length "+str(len(samples_target_list))+" (not in [1,2,3])"
	  #print "samples_target_list has to be a list of [sample, target] arrays"
	  #print "for example :\n\t[[TrainSet, TrainLabels]]"
	  #print "\tor [[TrainSamples, TrainLabels], [ValidSamples, ValidLabels]]"
	  #print "\tor [[TrainSamples, TrainLabels], [ValidSamples, ValidLabels], [TestSamples, TestLabels]]"

def parameters2list(C, kernel_parameters):
          if kernel_parameters == None:
	     return [C]
	  elif type(kernel_parameters) == list:
	     return [C]+kernel_parameters
	  elif type(kernel_parameters) == tuple:
	     return [C]+[prm for prm in kernel_parameters]
	  else:
             return [C, kernel_parameters ]


if __name__ == '__main__':
   from random import *

   learner = loadObject("/u/bengioy/LisaPLearn/UserExp/bengioy/babyAI/exp/minimize1_baby_image_rbm_trainsize=1000000_nh=100_mbs=20_nstages=1000000_nskip=2_mincost=rec.err._mre=0_2007-06-04#08:51:14.600291/final_learner.psave")

   execfile("/u/bengioy/LisaPLearn/UserExp/bengioy/babyAI/baby_data.py")

   valid1000 = pl.SubVMatrix(source=validSet,length=1000)

   (ts,image_repr,c) = learner.test(valid1000,pl.VecStatsCollector(),1,0)

   data=valid1000.getMat()

   y = [ vec[data.shape[1]-1]  for vec in data]
   text = [ vec[1024:data.shape[1]-1]  for vec in data]
   x = [ [image_repr[isample][coor] for coor in range(len(image_repr[0]))]+[text[isample][coor] for coor in range(len(text[0]))]  for isample in range(len(image_repr)) ]


   # an EXAMPLE to use the class...

   # Initialisation 
   
   E=discr_power_SVM_eval()
   
   # Compute the accuracies (exploring a bit, each time, the space of parameters)
   # -> Type E.accuracy to have the accuracy value.
   
   E.valid_and_compute_accuracy( 'LINEAR' ,  [[x,y]])
   E.valid_and_compute_accuracy( 'LINEAR' ,  [[x,y]])
   E.valid_and_compute_accuracy( 'RBF' ,    [[x,y]])
   E.valid_and_compute_accuracy( 'RBF' ,    [[x,y]])

   print E.accuracy
   print E.best_parameters
   print E.tried_parameters

   # To reset the explored tables of parameters and accuracies
   # (only keep the information about the last best parameters sets)
   # -> This has an interest when you use a new representation/dataset closed to the previous one.

   E.reset()
   E.valid_and_compute_accuracy( 'LINEAR' ,  [[x,y]])
   E.valid_and_compute_accuracy( 'RBF' ,  [[x,t],[x2,t2]])

   print E.accuracy
   print E.best_parameters
   print E.tried_parameters
