import sys, os, time
from numarray import *
from math import *
from libsvm import *

             
class SVM_expert(object):
      __attributes__ = ['kernel_type',
                        'parameters_names',
                        'tried_parameters',
                        'best_parameters',
                        'error_rate',
                        ]
                        
      def __init__( self ):
          self.parameters_names = ['C']
          self.tried_parameters = {}
          self.best_parameters  = None
          self.error_rate       = 1.

      def reset(self):
          self.tried_parameters = {}
          #if self.best_parameters != None:
          #   self.add_parameter_to_tried_list(self.getBestValue('C'), self.best_parameters[1:])
          self.error_rate       = 1.
	  self.should_be_tuned_again = 1


      def should_be_tuned_again():
          raise StandardError, "should_be_tuned_again is nor implemented for this class"
          
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
          rho = sqrt(dim)*std
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

      def should_be_tuned_again( self ):
          best_gamma = self.getBestValue('gamma') 
          tried_gamma = self.tried_parameters
	  if tried_gamma.has_key(best_gamma):
	     is_lower = False
	     is_higher = False
             for gamma in tried_gamma:
	         if gamma<tried_gamma:
		    is_lower = True
	         elif gamma<tried_gamma:
		    is_higher = True
	     if is_lower and is_higher:
	        best_C_for_this_gamma  = self.getBestValue('C') 
		tried_C_for_this_gamma = self.tried_parameters[best_gamma]
		return not ( best_C_for_this_gamma <> min(tried_C_for_this_gamma) and best_C_for_this_gamma <> max(tried_C_for_this_gamma) )
	     else:
	        return True
	  else:
	     return True
      
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
	     
      def should_be_tuned_again( self ):
          best_degree = self.getBestValue('degree')
          tried_degrees = [prms[0] for prms in self.tried_parameters]
          if self.tried_parameters.has_key(best_degree):
             if best_degree <> max(tried_degrees):
	        best_C_for_this_degree  = self.getBestValue('C') 
		tried_C_for_this_degree = self.tried_parameters[(best_degree,self.best_parameters[2])]
		return not ( best_C_for_this_degree <> min(tried_C_for_this_degree) and best_C_for_this_degree <> max(tried_C_for_this_degree) )
	  return True
      
class LINEAR_expert(SVM_expert):
      def __init__( self ):
          SVM_expert.__init__( self )
          self.kernel_type = 'LINEAR'

      def init_parameters( self, samples ):
          return SVM_expert.init_parameters(self, [None])

      def choose_new_parameters( self ):
          return SVM_expert.choose_new_parameters(self, None)

      def should_be_tuned_again( self ):
          if len(self.tried_parameters[None]) <= 3:
	     return True
          best_C = self.getBestValue('C') 
          tried_C = self.tried_parameters[None]
	  return not (best_C <> min(tried_C) and best_C <> max(tried_C) )

class SVM(object):

      __attributes__ = ['error_rate',
                        'valid_error_rate',
                        'best_parameters',
                        'tried_parameters',
                        'save_filename',
			'best_model',
			'nr_fold'
                        'result_list',
			'automatically_decide_when_to_stop_tuning'
                        ]
       
      def __init__( self ):
      
          self.error_rate       = 1.
          self.valid_error_rate = 1.
          
          self.LINEAR_expert  = LINEAR_expert()
          self.RBF_expert     = RBF_expert()
          self.POLY_expert    = POLY_expert()
          
          self.best_parameters      = None  
          self.best_model           = None
          self.tried_parameters     = {}
          self.result_list          = {}
          
          self.save_filename        = None
          
          # For cross-validation
          self.nr_fold        = 5

	  self.automatically_decide_when_to_stop_tuning = False

      def reset( self ):
          self.LINEAR_expert.reset()
          self.RBF_expert.reset()
          self.POLY_expert.reset()
          
          self.tried_parameters = {}
          self.result_list          = {}
          #if self.best_parameters != None:
          #   self.add_parameter_to_tried_list(self.best_parameters[0], self.best_parameters[1:])
          self.error_rate       = 1.
          self.valid_error_rate = 1.

      def add_parameter_to_tried_list(self, kernel, kernel_parameters):
          if self.tried_parameters.has_key(kernel):
             self.tried_parameters[kernel]+=[kernel_parameters]
          else:
             self.tried_parameters[kernel] =[kernel_parameters]

      def add_result_to_result_list(self, kernel, kernel_parameters, error_rate):
          if self.result_list.has_key(kernel):
             self.result_list[kernel]+= kernel_parameters, error_rate
          else:
             self.result_list[kernel] = kernel_parameters, error_rate
             
      def train_and_test(self, samples_target_list):
          check_samples_target_list(samples_target_list)

          best_expert = eval( 'self.'+self.best_parameters[0]+'_expert' )
          best_parameters = best_expert.best_parameters
          param = best_expert.get_svm_parameter( best_parameters )
          if len(samples_target_list) == 1: # cross-validation
             costs = do_cross_validation(samples_target_list[0][0], samples_target_list[0][1], param, self.nr_fold)
          else:
             costs = do_simple_validation(samples_target_list[0][0] , samples_target_list[0][1] , samples_target_list[1][0] , samples_target_list[1][1], param)
          return costs

      def test(self, samples_target_list):
          check_samples_target_list([samples_target_list])
          return test_model(self.best_model, [[x_i for x_i in x] for x in samples_target_list[0]], [float(l) for l in samples_target_list[1]])['error_rate']


      def train_and_tune(self, kernel_type, samples_target_list):
          check_samples_target_list(samples_target_list)
          if len(samples_target_list) == 1:
             print "\nCross-validation...\n"
          elif len(samples_target_list) == 2:
             print "\nSimple validation...\n"
          elif len(samples_target_list) == 3:
             print "\nValidation + test...\n"
           
          expert = eval( 'self.'+kernel_type+'_expert' )
          
          if len(expert.tried_parameters) == 0:
             recompute_best = True
          else:
             recompute_best = False

          if expert.best_parameters  == None:
             parameters_to_try = expert.init_parameters( samples_target_list[0][0] )
          else:
             parameters_to_try = expert.choose_new_parameters()
          
          best_parameters   = expert.best_parameters
          best_error_rate   = expert.error_rate
          best_model        = None

          for parameters in parameters_to_try:
              if parameters != expert.best_parameters or recompute_best:
              
                  self.add_parameter_to_tried_list(kernel_type, parameters)
                  param = expert.get_svm_parameter( parameters )
                  
                  if len(samples_target_list) == 1: # cross-validation
                     error_rate = do_cross_validation(samples_target_list[0][0], samples_target_list[0][1], param, self.nr_fold)['error_rate']
                  else:
                     train_problem = svm_problem( samples_target_list[0][1] , samples_target_list[0][0] )
                     model = svm_model(train_problem, param)
                     error_rate = test_model(model, samples_target_list[1][0], samples_target_list[1][1])['error_rate']
                     
                     if self.save_filename != None:
                        try:
                           FID=open(self.save_filename,'a')
                           FID.write('------------\nTry with '+kernel_type+' kernel :( parameters : '+str(parameters)+' )\n')
                           FID.write(' --> Error rate = '+str(error_rate)+'\n')
                           FID.close()
                        except:
                           print "COULD not write in save_filename"

                  self.add_result_to_result_list(kernel_type, parameters, error_rate)
                  if error_rate < best_error_rate:
                     best_parameters = parameters
                     best_error_rate = error_rate
		     if len(samples_target_list) <> 1: # in case of cross-validation, we will compute the model later
                        best_model   = model

          if best_error_rate < expert.error_rate:
             expert.best_parameters = best_parameters
             expert.error_rate = best_error_rate
             
             if best_error_rate < self.valid_error_rate:
                self.best_parameters = [kernel_type, best_parameters]
                self.valid_error_rate = best_error_rate
                if len(samples_target_list) == 1: # compute the best model in the case of cross-validation
                   train_problem = svm_problem( [float(l) for l in samples_target_list[0][1]] , [[float(x_i) for x_i in x] for x in samples_target_list[0][0]] )
                   param = expert.get_svm_parameter( best_parameters )
                   best_model = svm_model(train_problem, param)
                self.best_model = best_model
                if len(samples_target_list) == 3: # train-valid-test
                   self.error_rate = test_model(self.best_model, samples_target_list[2][0], samples_target_list[2][1])['error_rate']
                else:
                   self.error_rate = self.valid_error_rate
          
	  if self.automatically_decide_when_to_stop_tuning:
	     if expert.should_be_tuned_again():
	        self.train_and_tune(kernel_type, samples_target_list)
	  
	  return self.error_rate
          

##
## Using the cross validation of libSVM
## (problem: the folds are chosen randomly, so things are not rigorously compared)
##
#def do_cross_validation(samples, targets, param, nr_fold):
#        N = len(targets)
#        total_correct = 0
#        prob = svm_problem(targets, samples)
#            outputs = cross_validation(prob, param, nr_fold)
#        for i in range(N):
#            if outputs[i] == targets[i]:
#               total_correct = total_correct + 1 
#        return ( (N - total_correct) / N)

##
## Doing cross validation on samples
## - divide the set of samples in nr_fold subsets
## - (nr_fold - 1) subsets serve as training set / 1 subset as test set
##   for each step.
## - Then the error rate is simply the average error rate...
##
def do_cross_validation(samples, targets, param, nr_fold):
    arrayType=False
    if 'array' in str(type(samples)):
       arrayType=True
    targets_subsets=[]
    samples_subsets=[]
    N=len(samples)
    for i in range(nr_fold):
        samples_subsets.append(samples[i:N:nr_fold,:])
        targets_subsets.append(targets[i:N:nr_fold])
    cum_error_rate=0.
    for i in range(nr_fold):
        test_samples = samples_subsets[i]
        test_targets = targets_subsets[i]
        train_targets=[]
	if arrayType:
           train_samples=[]
	else:
           train_samples=[]
        for j in range(0,i)+range(i+1,nr_fold):
            train_targets += targets_subsets[j]
	    if arrayType:
	       L=len(train_samples)
               train_samples=resize(samples,[L+len(samples_subsets[j]),len(samples_subsets[j][0])])
	       train_samples[L:,:]=samples_subsets[j]
	    else:
               train_samples += samples_subsets[j]	       
        cum_error_rate += do_simple_validation(train_samples, train_targets, test_samples, test_targets, param)['error_rate']
#        cum_error_rate += do_simple_validation(samples, targets, test_samples, test_targets, param)['error_rate']
    av_error_rate = cum_error_rate*1.0 / nr_fold
    print av_error_rate
    return {'error_rate':av_error_rate}
        
def do_simple_validation(train_samples, train_targets, test_samples, test_targets, param):    
    train_samples = [[float(x_i) for x_i in x] for x in train_samples]
    test_samples = [[float(x_i) for x_i in x] for x in test_samples]
    train_problem = svm_problem( [float(l) for l in train_targets], train_samples )
    model = svm_model(train_problem, param)
    return test_model(model,test_samples,[float(l) for l in test_targets])

def test_model(model, samples, targets):
    N = len(samples)
    diffs = {}
    for i in range(N):
          diff = abs(model.predict([float(x_i) for x_i in samples[i]]) - float(targets[i]))
          if diffs.has_key(diff):
                diffs[diff] += 1
          else:
                diffs[diff] = 1
    error_rate = 0
    linear_class_error = 0
    square_class_error = 0
    for diff, nbdiff in diffs.iteritems():
          if not diff == 0:
                error_rate += 1*nbdiff
          linear_class_error += diff*nbdiff
          square_class_error += (diff*diff)*nbdiff
    error_rate = float(error_rate) / N
    linear_class_error = float(linear_class_error) / N
    square_class_error = float(square_class_error) / N
    return {'error_rate':error_rate, 'linear_class_error':linear_class_error, 'square_class_error':square_class_error }

#
# Some useful functions
#

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

def normalize(data,mean,std):
    if mean == None:
       mean=[]
       for i in range(len(data[0])):
           mean.append( get_mean_cmp(data,i) )
    if std == None:
       std=[]
       for i in range(len(data[0])):
           std_tmp=get_std_cmp(data,i)
           if std_tmp == 0.:
              print "WARNING : standard deviation is 0 on component "+str(i)
              std.append( 1. )
           else:
              std.append( std_tmp )
    for i in range(len(data[0])):
        for j in range(len(data)):
            data[j][i]=(data[j][i]-mean[i])/std[i]
    return mean, std

def mean_std(data):
    stds=[get_std_cmp(data,i) for i in range(len(data[0]))]
    return sum(stds)/len(stds)
def get_std_cmp(data,i):
    values=[vec[i] for vec in data]
    tot = sum(values)
    avg = tot*1.0/len(values)
    sdsq = sum([(i-avg)**2 for i in values])
    return (sdsq*1.0/(len(values)-1 or 1))**.5
def get_mean_cmp(data,i):
    values=[vec[i] for vec in data]
    return  sum(values)/len(values)

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

def check_samples_target_list(samples_target_list):
    if type(samples_target_list) != list or len(samples_target_list) == 0 or type(samples_target_list[0]) != list:
       raise TypeError, "ERROR: samples_target_list must be a list of list (of arrays)"
    else:
       for samples_target in samples_target_list:
           if len(samples_target) != 2:
              raise TypeError, "ERROR: samples_target_list has an element with length "+str(len(samples_target))+" (instead of 2)"
           if len(samples_target[0]) == 0 or len(samples_target[1]) == 0:
              raise ValueError, "ERROR: samples_target_list has an element that has an element with an empty length"
           if len(samples_target[0]) != len(samples_target[1]):
              raise ValueError, "ERROR: samples_target_list has an element that has an elements with different len. Len are: " + len(samples_target[0])+" and " + len(samples_target[1])
    if len(samples_target_list) not in [1,2,3]:
       raise TypeError, "ERROR: samples_target_list have length "+str(len(samples_target_list))+" (not in [1,2,3])\n"+"samples_target_list has to be a list of [sample, target] arrays\n"+"for example :\n\t[[TrainSet, TrainLabels]]\n"+"\tor [[TrainSamples, TrainLabels], [ValidSamples, ValidLabels]]\n"+"\tor [[TrainSamples, TrainLabels], [ValidSamples, ValidLabels], [TestSamples, TestLabels]]\n"

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

    # an EXAMPLE to use the class...

#>>># Initialization 

    my_svm=SVM()

#<<<#
#>>># To save the results (progressively) in a ASCII file

    my_svm.save_filename = 'my_svm_results.txt'
   
#<<<#
#>>># Pre-processing your data : it is better to normalize...
    
    # Get the mean and standard deviation on the training set
    # and normalize the training set (Mahalanobis)
    #
    mean, std = normalize(train_samples, None, None)
    #
    # DO NOT FORGET to apply the same normalization to other datasets
    #
    normalize(valid_samples, mean, std)
    normalize(test_samples, mean, std)
    
#<<<#
#>>># Defining train / valid data
    # - CROSS-VALIDATION
    
    DATA = [ [train_samples , train_targets] ]
    svm.nr_fold = 5  #(will train on 4/5 of the data and test on 1/5: this will be done 5 times)

    # or...
    # - SIMPLE VALIDATION
    
    DATA = [ [train_samples , train_targets] , [valid_samples , valid_targets] ]
    
    # Note:
    # You can also do
    #
    # DATA = [ [train_samples , train_targets] , [valid_samples , valid_targets] , [test_samples , test_targets] ]
    #
    # (my_svm.error_rate will be the statistic on test set, and my_svm.valid_error_rate the statistics on the validation set)
    #
    # But it is not the most efficient way to do :
    #     You'd better tune your model with validation,
    #     and then test when you have the best model
    
    
#<<<#
#>>># Train several models with different sets of parameters and choosing the best set ("tuning"/"twicking")
    # - my_svm.error_rate indicates the current error rates.
    # - This error rate can only decrease while you run "train_and_tune"
    #   (as you are tuning parameters so as to improve the results)
    # So one should run train_and_tune several times (as long as he can wait), at least for the LINEAR and RBF kernel
   
    my_svm.train_and_tune( 'LINEAR' ,  DATA )
    my_svm.train_and_tune( 'LINEAR' ,  DATA )
    my_svm.train_and_tune( 'RBF' ,     DATA )
    my_svm.train_and_tune( 'RBF' ,     DATA )
    my_svm.train_and_tune( 'POLY' ,    DATA )
    my_svm.train_and_tune( 'POLY' ,    DATA )
    #[..]

    valid_error_rate =  my_svm.error_rate
    print valid_error_rate
    print my_svm.best_parameters
    print my_svm.tried_parameters
    
#<<<#
#>>># When training with new data (closed to the previous one)
    # ones can need to forget the explored tables of parameters and accuracies
    # while reminding the best set of parameters

    my_svm.reset()
   
    # Cross-validation
    NEW_DATA =  [ [train_samples , train_targets] ]
    #
    # or
    #
    # Simple validation
    NEW_DATA =  [ [train_samples , train_targets], [valid_samples , valid_targets]  ]
   
    # If you want to tune again on the new data
    my_svm.train_and_tune( 'RBF' , NEW_DATA )
    #
    # or
    #
    # If you want to try what give the best parameters
    # (retrain the model on new train data, but no search for better parameters)
    my_svm.train_and_test( NEW_DATA )
    
    valid_error_rate = my_svm.error_rate

#<<<#
#>>># To try the best trained model with new data (and obtain "fair" error rates)
   
    TEST_DATA=[test_samples , test_targets]
    
    test_error_rate = my_svm.test( TEST_DATA )
    
    print test_error_rate
