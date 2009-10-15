#! /usr/bin/env python
#
# This python code is to use with PLearn (A C++ Machine Learning Library)
# Copyright (C) 2008 Jerome Louradour
#
# For any information or problem regarding this code, please contact jeromelouradour@gmail.com

import os

from plearn.learners import get_libsvm_module
exec 'from %s import *' % get_libsvm_module()

from plearn.pyext import *
from plearn.utilities.write_results import writeResults

import math
import numpy
import random
import fpconst # for NaN

class SVMHyperParamOracle__kernel(object):
    """ An oracle that gives values of hyperparameters      
        to train a SVM on vectors, given a popular kernel.
        
        Most of functions defined here deal with the choice
        of the hyperparameter 'C' (that controls the
        bias/variance trade-off in SVM training).
        These functions are used by subclasses, in functions
        that propose sets of hyperparameters (including
        kernel hyperparameters).

        List of attributes:    
        -------------------    

    user options:

        'C_initvalue': <float> Default value for 'C'.

        'verbosity': <int> Level of verbosity.

    learnt options:

        'best_param': <dict> of hyperparameters'values      
                      which gave the best performance by    
                      simple or cross- validation.
                      (minimum valid_cost)

        'best_cost': <float> Best cost obtained

        'inputsize': <int> input dimension. Used also sometimes
                     to choose the first hyperparameter values.

        'input_avgstd': <float> the average standard deviation
                          of the input data. It is estimated on
                          the train set, and is used to choose first
                          hyperparameter values.

        'input_means': <list> of mean values for each input component.

        'input_stds': <list> of standard deviation values for each input component.

    internally managed options:

        'kernel_type': <str> corresponding to the kernel.    
                       in ['gaussian','linear','poly'].

        'param_names': <list> of all hyperparameter names. It    
                       always includes at first the positive     
                       constant 'C' (trade-off bias/variance).   

        'trials_param_list': <list> of <dict> hyperparameter values
                            that have been already tried.

        'trials_cost_list': <list> or valid main costs corresponding to
                                hyperparameter values in 'trials_param_list'

    """
    __attributes__ = [  'C_initvalue',
                        'verbosity',
                        'best_param',
                        'best_cost'
                        'kernel_type',
                        'param_names',
                        'trials_param_list',
                        'trials_cost_list',
                        'input_avgstd',
                        'inputsize'
                      ]                  

    def __init__(self):
        self.param_names    = ['kernel_type','C']
        self.trials_param_list  = []
        self.trials_cost_list   = []
        self.best_param     = None
        self.best_cost      = None
        self.C_initvalue    = 1.
        self.stats_are_uptodate = False
        self.input_avgstd      = None
        self.input_means = None
        self.input_stds = None
        self.inputsize      = None
        self.verbosity      = 1

    def semiforget(self):
        """ To be called when training set change 'a bit'    
            (same kind of data, but different statistics, 
            so that we can assume new best parameters are
            close to previous ones.
        """
        if self.verbosity > 3:
            print "  (forget called)"
        self.trials_param_list  = []
        self.trials_cost_list = []
        self.stats_are_uptodate = False
        self.best_cost = None
        # Note: when we forget, we keep the value of
        #       'self.best_param'. This allow to initialize
        #       a new search (when data changed a bit) to
        #       a good candidate

    def forget(self):
        self.semiforget()
        self.best_param = None

    def set_input_stats( self, inputsize, input_avgstd ):
        self.stats_are_uptodate = True
        self.inputsize = int(inputsize)
        self.input_avgstd = input_avgstd

    def get_input_stats( self, samples=None ):
        if self.stats_are_uptodate:
            return ( self.inputsize, self.input_avgstd )
        if samples == None:
            if self.verbosity > 0:
                print "WARNING: get_data_stats() takes default value for input stats."
            return ( 2, 1. )

        if self.verbosity > 1:
            print "  (computing input stats)"
        
        self.inputsize = len(samples[0])
        self.input_avgstd, std__std_per_component = mean_std(samples)
        if( self.verbosity > 0 ): 
            if( self.input_avgstd < 0.5
            or  self.input_avgstd > 10.
            or  std__std_per_component/self.input_avgstd > 0.1 ):
                print "WARNING in SVMHyperParamOracle__kernel::get_input_stats() " + \
                    "\n\tYour data does not seem to be normalized: " + \
                    "\n\t(E[std_comp] = %.2f, std[std_comp] = %.2f)" % \
                    ( self.input_avgstd, std__std_per_component )

        self.stats_are_uptodate = True
        return (self.inputsize, self.input_avgstd)

    def init_C(self, C_value=None):
        """ Return a list of values to try for hyperparameter 'C'
            centered on a specified value 'C_value'.
            This is an internal function (should not be called outside this class def).
        """
        if C_value==None:
            C_value = self.C_initvalue
        return [ C_value, C_value/10., C_value*10.]

    def choose_first_C_param(self, kernel_param_list=None,
                                   C_value=None):
        """ Return <list> of <dict>: which hyperparameter values to try FIRST
            - 'kernel_param_list': <list> of <dict> kernel hyperparameters name->value.
        """
        # Input check
        if kernel_param_list==None:
            kernel_param_list=[{}]
        elif( type(kernel_param_list) <> list
           or len(kernel_param_list) == 0
           or type(kernel_param_list[0]) <> dict ):
            raise TypeError,"SVMHyperParamOracle__kernel::choose_first_C_param(), " + \
                            "last argument (%s) must be a list of dictionary" % \
                            (kernel_param_list)

        C_init_list = self.init_C(C_value)
        table  = []
        for C in C_init_list:
            for pdict in kernel_param_list:
                pdict.update({'C':C,'kernel_type':self.kernel_type})
                if pdict not in table:
                    table.append( pdict.copy() )
        return table

    def get_trials_oneparam_list(self, paramname='C', other_param=None):
        """ For a given hyperparameter name (default:'C'),
            return a list of values of this hyperparameter that have been tried
            (for a given sub-setting for other hyperparameters).
            Values are SORTED w.r.t to respective costs: the first one gave
            the best valid performance.
            - 'param_name': <string> generic name of the hyperparameter.
            - 'other_param': <dict> hyperparameters name->value.
        """
        # Input check
        if other_param==None:
            other_param={}
        if paramname in other_param:
            raise ValueError,"in SVMHyperParamOracle__kernel::get_trials_oneparam_list(), " + \
                             "hyperparameter %s must not be included in other_param (%s) " % \
                             (paramname, other_param)
        if paramname not in self.param_names:
            raise ValueError,"in SVMHyperParamOracle__kernel::get_trials_oneparam_list(), " + \
                             "hyperparameter %s not in self.param_names (%s) " % \
                             (paramname, self.param_names)

        if not len(self.trials_param_list):
            return []

        param_list=[]
        cost_list=[]
        for (param, cost) in zip(self.trials_param_list,
                                 self.trials_cost_list):
            do_match= True
            for pn in other_param:
                if param[pn] <> other_param[pn]:
                    do_match= False
                    break
            the_param_value = param[paramname]
            if do_match:
                # We avoid rendundancy in the returned list here
                if the_param_value in param_list:
                    if cost_list[ param_list.index(the_param_value) ] > cost:
                        cost_list[ param_list.index(the_param_value) ] = cost
                else:
                    param_list.append(the_param_value)
                    cost_list.append(cost)
        sorted_cost_list = sorted(cost_list)
        sorted_param_list = [None]*len(param_list)
        for cost, j in zip(sorted_cost_list, range(len(sorted_cost_list)) ):
            i = cost_list.index(cost)
            cost_list[i]  = None
            sorted_param_list[j] = param_list[i]
        return sorted_param_list

    def choose_new_param_geom( self,
                               crescentcosts_list,
                               allow_to_return_already_tried_ones = True ):
        """ A utility to give new hyperparameter values given the best value among a list
            when the hyperparameter is multiplicative and positive (so the geometrical mean
            is more suitable than the arithmetic mean).
            - 'allow_to_return_already_tried_ones': <bool> Can return already tried values?
                    It is recommended to be True if and only if :
                    1. there are other hyperparams to tune, and
                    2. the hyperoptimization problem is not necessarily convex.
        """
        best_value = crescentcosts_list[0]
        crescentvalues_list = sorted(crescentcosts_list)

        # smallest value
        if best_value == crescentvalues_list[0]:
            ratio = crescentvalues_list[0]/crescentvalues_list[1] # < 1
            in_between = geom_mean([crescentvalues_list[1],best_value])
            if allow_to_return_already_tried_ones:
                return [ best_value * ratio,
                         best_value,
                         in_between ]
            else:
                return [ in_between * ratio,
                         in_between,
                         in_between * ratio*ratio ]

        # largest value
        if best_value == crescentvalues_list[-1]:
            ratio = crescentvalues_list[-1]/crescentvalues_list[-2] # > 1
            in_between = geom_mean([crescentvalues_list[-2],best_value])
            if allow_to_return_already_tried_ones:
                return [ best_value *ratio,
                         best_value,
                         in_between ]
            else:
                return [ in_between * ratio,
                         in_between,
                         in_between * ratio*ratio ]

        #else:
        # middle value (best case: dichotomie)

        i = crescentvalues_list.index(best_value)
        L = len(crescentvalues_list)

        # Case when the 3 best hyperparam values for X are returned
        #  (assuming that the system will choose new values for Y and/or Z and/or... given X)
        # 
        if allow_to_return_already_tried_ones:
            if ( L > 4
             and i > 1
             and i < L-2
             and random.random() < 0.5):
                return crescentcosts_list[:3]
            else:
                return [  best_value,
                          geom_mean([crescentvalues_list[i-1],best_value]),
                          geom_mean([crescentvalues_list[i+1],best_value]) ]

        return [  geom_mean([crescentvalues_list[i-1],best_value]),
                  geom_mean([crescentvalues_list[i+1],best_value]) ]

    def choose_new_C_param(self, kernel_param_list=None):
        """ Return <list> of <dict>: which hyperparameter values to try NEXT.
            - 'kernel_param_list': <list> of <dict> kernel hyperparameters name->value. """
        # Input check
        if kernel_param_list==None:
            kernel_param_list=[{}]
        elif( type(kernel_param_list) <> list
          or  len(kernel_param_list) == 0
          or  type(kernel_param_list[0]) <> dict ):
            raise TypeError, "SVMHyperParamOracle__kernel::choose_new_C_param(), last argument (%s) must be a list of dictionary" % \
                             (kernel_param_list)

        new_param_list  = []
        for kprm in kernel_param_list:
            trials_C_list = self.get_trials_oneparam_list('C',kprm)
            # if we have already tried some 'C' for this kernel
            # parametrization, we try others
            if trials_C_list and len( trials_C_list ) > 2:
                C_table = self.choose_new_param_geom( trials_C_list, False)
            # if this kernel parametrization is new, or the C has been really tuned ont it
            # we give C values around the best one
            # (obtained with another parametrization)
            else:
                C_table = self.init_C( self.best_param['C'] )

            for C in C_table:
                kprm.update({'C':C,'kernel_type':self.kernel_type})
                if kprm not in self.trials_param_list:
                    new_param_list.append( kprm.copy() )
        return new_param_list


    def should_be_tuned_again(self, kernel_param_list=None):
        """ Return <bool>: whether or not we should try other 'C' for
                           a given set of kernel hyperparameters.
            - 'kernel_param': <dict> kernel parameters name->value.
        """
        if kernel_param_list==None:
            kernel_param_list={}
        trials_C_list = self.get_trials_oneparam_list('C',kernel_param_list)
        if len(trials_C_list) <= 3:
            return True
        best_C = self.best_param['C']
        return ( best_C == min(trials_C_list) or best_C == max(trials_C_list) )

    def update_trials(self, param, cost):
        """ Updates the statistics and internal trials history given new performances,
            and returns if the trial was the best one.    
            - 'param' is the <dict> of parameter values.
            - the 'costs' are <dict> of corresponding performance (with None wherever missing).
        """
        if 'C' not in param:
            raise IndexError,"in SVMHyperParamOracle__kernel::update_trials(), " + \
                             "2nd arg (param=%s) must include 'C'" % (param)

        if param not in self.trials_param_list:
            self.trials_param_list.append(param)
            self.trials_cost_list.append(cost)
        else:
            i = self.trials_param_list.index(param)
            if cost <= self.trials_cost_list[i]:
                self.trials_cost_list[i] = cost

        if( self.best_cost == None or cost < self.best_cost):
        # TODO: what if cost == bestcost and param <> self.best_param?
        #       -> best param could be a list...
            self.best_param = param
            self.best_cost  = cost
            return True

        return False


class SVMHyperParamOracle__linear(SVMHyperParamOracle__kernel):
    """ Sub-class of SVMHyperParamOracle__kernel, with a linear kernel.
    """
    def __init__(self):
        SVMHyperParamOracle__kernel.__init__(self)
        self.kernel_type = 'linear'

    def choose_first_param(self, samples=None ):
        """ Return <list> of <dict>: which hyperparameter values to try FIRST
            - 'samples': <array>(n_samples,dim) of (train/valid) samples.
        """
        # Invariance w.r.t scaling input
        d, std = self.get_input_stats( samples )
        # Note:  default C=1   [for default std=1.]
        self.C_initvalue = 1./(std*std)
        return self.choose_first_C_param()

    def choose_new_param(self):
        """ Return <list> of <dict>: which hyperparameter values to try NEXT.
        """
        return self.choose_new_C_param()

    """ Return <bool>: whether or not we should try other hyperparameter values """
    #def should_be_tuned_again(self):        
    #    return SVMHyperParamOracle__kernel.should_be_tuned_again(self)


class SVMHyperParamOracle__rbf(SVMHyperParamOracle__kernel):
    """ Sub-class of SVMHyperParamOracle__kernel, with a Gaussian kernel ('rbf').
    """
    def __init__(self):
        SVMHyperParamOracle__kernel.__init__(self)
        self.kernel_type  = 'rbf'
        self.param_names += ['gamma']

    def init_gamma(self, gamma_value=0.5):
        return [ gamma_value, gamma_value/9., gamma_value*9. ]

    def choose_first_param(self, samples=None ):
        """ Return <list> of <dict>: which hyperparameter values to try FIRST.
            - 'samples': <array>(n_samples,dim) of (train/valid) samples.
        """
        d, std = self.get_input_stats(samples)
        # Note:  default gamma=1/4   [for default d=2, std=1.]
        gamma0 = 1./(2*self.inputsize*std*std)
        gamma_list = self.init_gamma(gamma0)
        kernel_param_list=[]
        for g in gamma_list:
            kernel_param_list.append({'gamma':g})
        return self.choose_first_C_param( kernel_param_list )
        
    def choose_new_param(self):
        """ Return <list> of <dict>: which hyperparameter values to try NEXT.
        """
        best_gamma = self.best_param['gamma']
        tried_gamma = self.get_trials_oneparam_list('gamma')
        if best_gamma in tried_gamma:
            if best_gamma <> tried_gamma[0]:
                print "WARNING in SVMHyperParamOracle__rbf::choose_new_param() " + \
                                 "best gamma %s is not the 1st of tried_gamma %s." % \
                                 (best_gamma, tried_gamma ) + \
                                 "\nThis list should be ordered w.r.t. costs."
                tried_gamma.remove(best_gamma)
                tried_gamma = [best_gamma]+tried_gamma
            gamma_list = self.choose_new_param_geom( tried_gamma )
        else:
            gamma_list = self.init_gamma(best_gamma)
        return self.choose_new_C_param( [{'gamma':g}  for g in gamma_list] )

    def should_be_tuned_again(self):
        """ Return <bool>: whether or not we should try other hyperparameter values.
        """
        best_gamma = self.best_param['gamma']
        tried_gamma = self.get_trials_oneparam_list('gamma')
        if best_gamma not in tried_gamma:
            return True
        if( best_gamma == min(tried_gamma)
        or  best_gamma == max(tried_gamma) ):
            return True
        return SVMHyperParamOracle__kernel.should_be_tuned_again(self, {'gamma':best_gamma})

    
class SVMHyperParamOracle__poly(SVMHyperParamOracle__kernel):
    """ Sub-class of SVMHyperParamOracle__kernel, with a polynomial kernel.
    """
    def __init__(self):
        SVMHyperParamOracle__kernel.__init__(self)
        self.kernel_type     = 'poly'
        self.param_names    += ['degree','coef0']
        self.coef0_initvalue   = 1.

    def init_degree(self, degree=3):
        if degree < 3: degree = 3
        return [  degree, degree-1, degree+1 ]

    def init_coef0(self,
                   coef0= None):
        if not coef0:
            coef0 = self.coef0_initvalue
        return [  coef0, coef0/10., coef0*10. ]

    def choose_first_param(self, samples=None ):
        """ Return <list> of <dict>: which hyperparameter values to try FIRST.
            - 'samples': <array>(n_samples,dim) of (train/valid) samples.
        """
        degree_list  = self.init_degree()
        if samples <> None:
            d, std = self.get_input_stats(samples)
            self.coef0_initvalue = std*std
            first_param_list = []
            # We choose a table of C different for each d, for scaling invariance.
            # We also hope there won't be any numerical problem after...
            for d in degree_list:
                self.C_initvalue = 1./(std**int(2*d))
                first_param_list += self.choose_first_C_param( \
                                         [{'degree':d,'coef0':self.coef0_initvalue}] )
            return first_param_list
        return self.choose_first_C_param( [ {'degree':d,'coef0':self.coef0_initvalue} for d in degree_list ] )        

    def choose_new_param(self):
        """ Return <list> of <dict>: which hyperparameter values to try NEXT.
        """
        best_degree = self.best_param['degree']
        tried_degrees = self.get_trials_oneparam_list('degree')
        if best_degree in tried_degrees:
            if best_degree == max(tried_degrees):
               degree_list = self.init_degree(best_degree+1)
            elif best_degree == min(tried_degrees):
               degree_list = self.init_degree(best_degree-1)
            else:
               degree_list = self.get_trials_oneparam_list('degree')[:3]
        else:
            degree_list = self.init_degree(best_degree)

        new_param_list=[]
        for d in degree_list:
            tried_coef0 = self.get_trials_oneparam_list('coef0',{'degree':d})
            tried_C = self.get_trials_oneparam_list('C',{'degree':d})

            if not tried_C:
                new_param_list += self.choose_first_C_param( \
                                        [ {'degree':d, 'coef0':self.best_param['coef0']} ], \
                                        self.best_param['C'] )
            
            elif( tried_C[0] == min(tried_C)
               or tried_C[0] == max(tried_C) ):
                new_param_list += self.choose_new_C_param( [ {'degree':d, 'coef0':tried_coef0[0]} ] )

            # new degree tried: one value of 'coef0' tried and several for 'C' first
            elif len(tried_coef0) == 0:
                new_param_list += self.choose_first_C_param( \
                                       [ {'degree':d,'coef0':self.coef0_initvalue} ], \
                                       tried_C[0] )

            # this degree was tried once with several 'C' but one (or two?) coef0
            # we try others coef0
            elif len(tried_coef0) < 3:
                new_param_list += self.choose_first_C_param( \
                                       [ {'degree':d, 'coef0':c} for c in self.init_coef0(tried_coef0[0]) ], \
                                       tried_C[0] )

            # coef0 is critical, we want to try again
            elif( tried_coef0[0] == min(tried_coef0)
               or tried_coef0[0] == max(tried_coef0) ):
                new_param_list += self.choose_first_C_param( \
                                       [ {'degree':d, 'coef0':c} for c in self.choose_new_param_geom( tried_coef0 )[:2] ], \
                                       tried_C[0] )
                                       
            # we try other 'C'
            else:
                new_param_list += self.choose_new_C_param( [ {'degree':d, 'coef0':tried_coef0[0]} ] )
                                    
        return new_param_list

    def should_be_tuned_again(self):
        """ Return <bool>: whether or not we should try other hyperparameter values.
        """
        best_degree = self.best_param['degree']
        tried_degrees = self.get_trials_oneparam_list('degree')
        if( best_degree in tried_degrees
        and best_degree <> max(tried_degrees)
        and ( best_degree <> min(tried_degrees) or best_degree == 2 ) ):
                tried_C = self.get_trials_oneparam_list('C',{'degree':best_degree})
                tried_coef0 = self.get_trials_oneparam_list('coef0',{'degree':best_degree})
                return (  tried_C[0] == min(tried_C)
                       or tried_C[0] == max(tried_C)
                       or tried_coef0[0] == min(tried_coef0)
                       or tried_coef0[0] == max(tried_coef0) )
        return True
    


class SVM(object):
    """ An expert driven by different oracles (1 oracle = 1 kernel)
        to try values of hyperparameters to train a SVM.
        The user may want to try one or several kernels.
        Results are stored per kernel type.

        List of attributes:    
        -------------------    

    user options:

        'costnames': <list> of cost names. By default, ['class_error'].
                 All cost names implemented are:
                 - 'class_error': classification error.         
                 - 'confusion_matrix': confusion matrix. It will     
                                  output several stats on the results.amat file.         
                                  e.g. 'cm_1_0' is the % of time the system
                                  returned 0 when the truth was 1.

                 - 'norm_ce': normalized classification error. It is computed
                              by weighting each classification error by its cost
                              given in 'errorcosts'.
                              
                 - 'n_sv': <int> number of Support Vectors.

        'errorcosts': <array> of shape (nclasses, nclasses) that
                      specifies the costs of each type of misclassification,
                      to compute 'norm_ce'.
                      errorcosts[c,c] should be 0. errorcosts[c,r] is the cost
                      assigned when the truth is 'class c' and the system
                      returns 'class r'.
                      By default, if 'norm_ce' is asked, 'errorcosts' will be
                      set w.r.t classes frequencies.

        'maincost_name': <str> Cost that leads the choice of hyperparameters.
                         The main cost is the cost to minimize it by validation.
                         By default (if set to None), the first cost of costnames
                         is considered. 

        'n_fold': <int> Number of folds in the cross-validation

        'normalize_inputs': <bool> Whether to normalize inputs so as to have
                            mean=0, std=1 for each input component on the train set.
        
        'balanceC': <bool> Whether to use a different 'C' for each class,
                           based on the frequency of each class in the training
                           set. Useful when: classes are unbalanced and the cost
                           of interest is normalized w.r.t class prior probas.

        'multiclass_strategy': <string> Kind of strategy to compute outputs.
                           - 'onevsone': One-against-one
                           - 'onevsall': One-against-all

        'outputs_type': <string> that specifies the kinds of SVM outputs (of which
                        may depend the predictions).
                        - 'onehot' is a simple onehot of the prediction.
                        - 'votes' is the vote counts (one-against-one strategy for multi-class).
                        - 'dist' is the usual output for a SVM (distance to the boundary). For one-vs-one, it is the sum of distances.
                        - 'softvotes' is the same as 'dist' with a softmax of the output after (does not change the decision, except if you process bags of outputs).
                        - 'proba' is the posterior emperical probability (using sigmoids of SVM standard outputs).
                        Note: with the one-vs-all strategy, 'votes' and 'onehot' give the same outputs.
                                     
        'retrain_on_valid': <bool> Use the best hyperparameters (found on valid)
                             to re-train a bigger model on {train, valid}.
                             Note: this option is obsolete for cross-validation
                             (valid_samples = None), were the model is re-trained
                             in any case.

        'test_on_train': <bool> Should we test best models on the train set.

        'retrain_until_local_optimum_is_found': <bool> when calling run(), whether or
                         not to continue to tune hyperparameters until a local optimum
                         is found. If False, you can re-run run() several times. If True,
                         you can also re-run to possibly find better performance.

        'testlevel': <int> Frequency of test:
                     - 0: write results only at the end of the run()
                     - 1: write results each time a better validation cost is found in run()
                     - 2: write all intermediate results

        'max_ntrials': <int> when calling run(), maximum number of hyperparameters to try
                      since the last forget(). If set to 1, then no hyperoptimization will be performed
                      (first arbitrary hyperaparameters will be chosen, this is recommended for debugging only)
                      
        'min_cost': <float> minimum sufficient cost value. If this value is reached then the hyperoptimization
                    will stop immediately. Recommended for speed-up when the optimization is easy.
                    
        'max_cost': <float> maximum admissible cost value. If this value is not reached during the first set
                    of hyperoptimization (e.g. 9 trials with a RBF kernel), then the hyperoptimization will stop.
                    Recommended when you test SVM with different front-end and you suspect some front-ends to be
                    simply bad ones (you do not want to waste time with them).

        'results_filename': <string> Path to an output file for results
        
        'preproc_optionnames': <string> or <list of strings> indicating the names of the
                               hyperparameters of the pre-processing that should appear
                               in the results (results_filename)

        'preproc_optionvalues': <string> or <list> indicating the values of the hyperparameters
                                corresponding to preproc_optionnames.

        'verbosity': <int> Level of verbosity.

        'additional_libsvm_params': <dict> any additional parameters you would like to pass to libsvm when
                                    calling svm_model. This allow to use locally modified versions of libsvm.
                                    e.g: dict(max_iter= 10000) for the libsvm version installed at ApStat.

        'intern_kfold': <list> of <list> (or None) list of indices to use for the internal k-folds (if provided).
                        The list has to be exactly of length n_fold (for safety), and each element is a list of
                        length two (first list for train indices, second list of test indices).

        'recompute_stats_at_each_train': <bool> Do we re-estimate train statistics each time we call train()
                        The value of this option has an impact only in the following cases:
                        * if balanceC is activated
                          (weights of penalty C will be computed at each train).
                        * if you normalize inputs.

    learnt options:

        'best_model':

        'best_param': <dict> hyperparameters name->value
                      which gave the best performance by    
                      simple or cross- validation.
                      (minimum valid_cost)

        'valid_stats': <VecStatsCollector> costs corresponding to
                       the best validation main cost.

        'test_stats': <VecStatsCollector> test costs corresponding to
                      the best validation main cost.

        'train_stats': <VecStatsCollector> train costs corresponding to
                       the best validation main cost.

        
        'nclasses': <int> number of classes.

        'class_priors': <list> frequency for each class in the train set.
                        This stat is used when the 'balancedC' option is ON.

        'inputsize': <int> input size

        'input_avgstd': <float> input std

        'validtype': <str> type of validation: 'simple' or 'cross'.
               
    internally managed options:

        'param_names': <list> of all hyperparameter names. It    
                       always includes at first the positive     
                       constant 'C' (trade-off bias/variance).   

        'stats_are_uptodate': <bool> Are the statistics computed
                        on the train set up to date?


    """


    __attributes__ = [  'name',
                        'costnames',
                        'errorcosts',
                        'maincost_name',
                        'n_fold',
                        'normalize_inputs',
                        'balanceC',
                        'multiclass_strategy',
                        'outputs_type',
                        'retrain_until_local_optimum_is_found',
                        'max_ntrials',
                        'min_cost',
                        'max_cost',
                        'retrain_on_valid',
                        'test_on_train',
                        'testlevel',
                        'verbosity',
                        'results_filename',
                        'preproc_optionnames',
                        'preproc_optionvalues',
                        'intern_kfold',
                        'recompute_stats_at_each_train',
                        \
                        'validtype',
                        'param',
                        'best_param',
                        'model',
                        'best_model',
                        'valid_stats',
                        'test_stats',
                        'train_stats',
                        'nclasses',
                        'class_priors',
                        'inputsize',
                        'input_avgstd',
                        \
                        'param_names',
                        'stats_are_uptodate',
                        \
                        'additional_libsvm_params', 
                     ]
     
    def __init__(self):
        
        self.name = 'svm'
        
        self.costnames       = ['class_error', 'confusion_matrix']
        self.errorcosts      = None
        self.maincost_name   = None
        self.valid_stats     = None
        self.valid_outputs   = None
        self.test_stats      = None
        self.train_stats     = None
        self.validtype       = 'simple'
        self.multiclass_strategy = 'onevsone'

        self.n_fold = 5
        self.intern_kfold = None
        self.recompute_stats_at_each_train = False
        self.balanceC = False
        self.balance_classes = False
        self.normalize_inputs = False

        self.HyperParamOracle__linear  = SVMHyperParamOracle__linear()
        self.HyperParamOracle__rbf     = SVMHyperParamOracle__rbf()
        self.HyperParamOracle__poly    = SVMHyperParamOracle__poly()
        self.all_experts        = [self.HyperParamOracle__linear,
                                   self.HyperParamOracle__rbf,
                                   self.HyperParamOracle__poly,
                                  ]
        self.param_names =['outputs_type','multiclass_strategy','validtype','normalize_inputs','kernel_type','balanceC']
        for expert in self.all_experts:
            for pn in expert.param_names:
                if pn not in self.param_names:
                    self.param_names.append(pn)
        
        self.kernel_type = 'linear'
        self.param = None
        self.best_param  = None
        self.model = None
        self.best_model = None
 
        self.stats_are_uptodate = False
        self.inputsize = None
        self.input_avgstd = None
        self.input_means = None
        self.input_stds = None
        self.nclasses = None
        self.class_priors = None
        self.weight = None
        self.labels = None
                
        self.results_filename      = None
        self.preproc_optionnames  = []
        self.preproc_optionvalues = []

        self.retrain_on_valid = True
        self.retrain_until_local_optimum_is_found = True

        self.test_on_train = False
        self.max_ntrials = 50
        self.min_cost = None
        self.max_cost = None
        
        self.verbosity = 0
        self.testlevel = 1
        
        self.trainset_key = 'trainset'
        self.validset_key = 'validset'
        self.testset_key  = 'testset'

        self.outputs_type = 'votes'
        
        self.additional_libsvm_params = {}

    def semiforget(self):
        for expert in self.all_experts:
             expert.semiforget()
        self.valid_stats     = None
        self.valid_outputs   = None
        self.test_stats      = None
        self.train_stats     = None
        self.stats_are_uptodate = False
        self.input_means = None
        self.input_stds = None
        self.model = None
        self.best_model = None
        # Note: when we forget, we keep the value of
        #       'self.best_param'. This allow to initialize
        #       a new search (when data changed a bit) to
        #       a good candidate

    def forget(self):
        self.semiforget()
        for expert in self.all_experts:
             expert.forget()
        self.best_param = None

    def train_inputspec(self, dataspec):
        assert type(dataspec) == dict
        if self.trainset_key not in dataspec:
            raise KeyError, "Key %s not in dataspec (keys %s)" % \
                            ( self.trainset_key, dataspec.keys() )
        return dataspec[ self.trainset_key ]
    def valid_inputspec(self, dataspec):
        assert type(dataspec) == dict
        return dataspec.get(self.validset_key)
    def test_inputspec(self, dataspec):
        assert type(dataspec) == dict
        return dataspec.get( self.testset_key )

    ## specific to PLearn and libsvm
    def get_datalist(self, input_vmat ):
        """ From a VMatrix, return samples and targets in the format required
            by libsvm, i.e. lists of float.
            This is only a default function, so that the user can
            define another method for any sophisticated VMatrix
            (this can be done by changing the attribute:
                                    SVM.get_datalist ).
            - 'input_vmat': <VMatrix> Must have the function getMat()
                            that returns the corresponding array,
                            as well as attributes:
                            inputsize, targetsize, length.
                            
        """
        data_array = input_vmat.getMat()
        inputsize = input_vmat.inputsize
        targetsize = input_vmat.targetsize
        nsamples = input_vmat.length
        assert data_array.shape[0] == nsamples
        samples   = [ [ float(x_t_i)    for x_t_i in x_t ]
                                        for x_t in data_array[:,:inputsize] ]
        assert targetsize == 1
        targets   = [ float(t) for t in data_array[:,inputsize] ]       
        return samples, targets

    def get_svminputlist(self, input_vmat, isTrain=False):
        if self.balance_classes and isTrain:
            input_vmat = ReplicateSamplesVMatrix(source = input_vmat,
                                                 operate_on_bags = (input_vmat.targetsize > 1 ) )
        samples, targets = self.get_datalist( input_vmat )
        if self.normalize_inputs:
            if self.input_means == None:
                self.input_means, self.input_stds = normalize_data(samples)
            else:
                assert self.input_means
                normalize_data(samples, self.input_means, self.input_stds)
        return samples, targets
        

    def get_class_priors(self, targets ):
        """ Return a dictionary class label -> class frenquency,
            where frequencies are estimated from 'targets', a list of class labels
        """
        class_priors = {}
        for label in targets:
            class_priors[label] = class_priors.get(label, 0.) + 1.
        nsamples = sum( class_priors.values() )
        for label in class_priors:
            class_priors[label] /= nsamples
        return class_priors
           
    def get_data_stats(self, vmat=None):
        """ Return the input/target dimensions and stats estimated on a VMatrix 'vmat'.
        """
        if self.stats_are_uptodate:
            return ( self.nclasses, self.class_priors,
                     self.inputsize, self.input_avgstd )
        assert vmat <> None

        samples, targets = self.get_svminputlist( vmat, True )

        self.all_experts[0].verbosity = self.verbosity
        self.inputsize, self.input_avgstd = self.all_experts[0].get_input_stats(samples)
        for expert in self.all_experts[1:]:
            expert.set_input_stats( self.inputsize, self.input_avgstd )

        if targets == None:
            if self.verbosity > 1:
                print "WARNING: get_data_stats() takes default value for class info."
            return ( 2, [ .5, .5 ],
                 self.inputsize, self.input_avgstd )

        class_priors = self.get_class_priors( targets )
        self.nclasses = len( class_priors )
        self.class_priors = class_priors

        if self.verbosity > 0:
            print "  ( class priors: %s  -- %d samples of dim %d )" % \
                     ( class_priors, len(targets), len(samples[0]) )

        if self.balanceC:
            weight = [ 1./p for p in class_priors.values() ]
            # i think it is better to have integers with libsvm
            # but we can generalize this to whatever representation
            #           (maybe not so safe with PLearn...)
            self.labels = [ int(l) for l in class_priors.keys() ]
            
            # average weight = 1
            S = sum(weight)
            self.weight = [ w*self.nclasses*1./S for w in weight ]
            if self.verbosity > 0:
                print "  (to compensate for class priors: weight C by %s)" % \
                         ( self.weight )

        self.stats_are_uptodate = True
        return ( self.nclasses, self.class_priors,
                 self.inputsize, self.input_avgstd )

    def get_expert(self, kernel_type ):
        return eval( 'self.HyperParamOracle__'+kernel_type.lower() )
    
    """ The following 3 functions simply give access to costs
        from a VecStatsCollector 'stat'
    """
    def get_cost(self, stats, cost_name):
        """ cost_name is a string """
        if stats == None:
            return None
        if cost_name not in stats.getFieldNames():
            raise KeyError, "in SVM::main_cost(), " + \
                            "stats does not have field %s" % \
                            ( cost_name )
        return stats.getStat('E[%s]' % cost_name )
    def get_maincost(self, stats):
        if self.maincost_name:
            maincost_name= self.maincost_name
        else:
            maincost_name= self.costnames[0]
        return self.get_cost( stats, maincost_name )
    def get_all_costs(self, stats):
        allcosts={}
        allNones = True
        for cost_name in self.costnames:
            allcosts[cost_name] = self.get_cost(stats, cost_name)
            if allNones and allcosts[cost_name] <> None:
                allNones = False
        if allNones:
            return None
        return allcosts

    ## specific to libsvm
    def get_libsvm_param(self, param ):
        """ Return a svm_parameter in the format for libsvm.
            - 'param': <dict> parameters name->value.
        """
        param = param.copy()
        for pn in param:
            if param[pn] == None:
                param.pop(pn)
            elif pn == 'kernel_type':
                # libsvm wants upper case for kernel_type
                param[pn] = eval(param[pn].upper())

        if 'proba' in self.outputs_type:
            param.update({'probability':1})
        
        return svm_parameter( svm_type = C_SVC, **param )
    

    def write_results(  self, param,
                        valid_stats,
                        test_stats = None,
                        train_stats= None,
                        ntrain = None,
                        nvalid = None,
                        ntest = None,
                        only_stdout= False ):
        """ Write given results with corresponding parameters
            In a PLearn format (.amat).True
        """
        if valid_stats==None and param == self.best_param:
                valid_stats = self.valid_stats
        if test_stats==None and param == self.best_param:
                test_stats = self.test_stats
        if train_stats==None and param == self.best_param:
                train_stats = self.train_stats
        valid_costs = self.get_all_costs(valid_stats)
        test_costs = self.get_all_costs(test_stats)
        train_costs = self.get_all_costs(train_stats)
        
        # If no file specified, print on stdout in a readable format
        if self.results_filename == None or self.verbosity > 0:
            if train_costs <> None:
                print " -- train costs: ", train_costs
            if valid_costs <> None:
                print " -- valid costs: ", valid_costs
            if test_costs <> None:
                print " -- test costs: ",  test_costs
            if self.results_filename == None or only_stdout:
                return
        
        # Format preprocessing option names to obtain one string
        if isinstance(self.preproc_optionnames,str):
            preproc_optionnames = self.preproc_optionnames.split()
        elif isinstance(self.preproc_optionnames,list):
            preproc_optionnames = self.preproc_optionnames
        else:
            raise TypeError, "preproc_optionnames (type %s) must be of type str or list" % type(self.preproc_optionnames)

        # Format preprocessing option values to obtain one string
        if isinstance(self.preproc_optionvalues,str):
            preproc_optionvalues = self.preproc_optionvalues.split()
        elif isinstance(self.preproc_optionvalues,list):
            preproc_optionvalues = self.preproc_optionvalues
        else:
            raise TypeError, "preproc_optionvalues (type %s) must be of type str or list" % type(self.preproc_optionvalues)
        
        param_names=self.param_names
        param_values=[]
        for pn in param_names:
            if pn in param:
                param_values.append(param[pn])
            elif pn in self.__attributes__:
                param_values.append( eval('self.'+pn) )
            else:
                param_values.append(None)
        
        costnames = []
        costvalues = []
        
        for cn in self.costnames:
            for dataset in ['train','valid','test']:
                costs = eval(dataset+'_costs')
                if costs == None:continue
                
                # Special processing for the confusion matrix
                if cn == 'confusion_matrix':
                    if cn in costs:
                        confusion_matrix=costs['confusion_matrix']
                    else:
                        confusion_matrix=None
                    for cli in range(self.nclasses):
                        for clj in range(self.nclasses):
                            costnames.append( "E[%s.E[cm_%d_%d]]" % \
                                                (dataset, cli, clj) )
                            if confusion_matrix <> None:
                                costvalues.append( confusion_matrix[cli,clj] )
                            else:
                                costvalues.append( None )
                    continue

                costnames.append( "E[%s.E[%s]]" % (dataset, cn) )
                costvalues.append( costs.get(cn) )
        
        for variable_name in ['ntrain','nvalid','ntest']:
            if eval(variable_name) <> None:
                preproc_optionnames += ['%s' % variable_name]
                preproc_optionvalues += ['%s' % eval(variable_name)]
        
        # Write the result in the file specified by 'results_filename'
        writeResults(   [ preproc_optionnames + param_names, preproc_optionvalues + param_values ]
                      , [ costnames, costvalues ]
                      , self.results_filename)

    def update_trials(self, param,
                      valid_stats,
                      test_stats = None,
                      train_stats= None):
        """ Updates the statistics and internal trials history given new performances,
            and returns if the trial was the best one.
            - 'param' is the <dict> of parameter values
            - the 'costs' are <dict> of corresponding performance (with None wherever missing).
        """

        # Check input: enforce to give a valid cost (and not test cost)
        # or else the valid has been done and we try with the best hyperparameters
        if valid_stats==None:
            if param == self.best_param:
                if test_stats <> None:
                    self.test_stats = test_stats
                if train_stats <> None:
                    self.train_stats = train_stats
                return True
            else:
                raise KeyError,"in SVM::update_trials(), " + \
                                "3rd arg (valid_stats=%s) must be a <dict> with key '%s' pointing to a float" % \
                                (valid_stats, maincost_name)

        kernel_type = param['kernel_type']

        maincost=self.get_maincost(valid_stats)
        bestcost=self.get_maincost(self.valid_stats)

        expert = self.get_expert(kernel_type)
        kernel_param = {}
        for pn in expert.param_names:
            kernel_param[pn] = param[pn]
        expert.update_trials(kernel_param, maincost)

        if( bestcost == None or maincost < bestcost):
            self.best_param = param
            self.valid_stats = valid_stats
            self.test_stats  = test_stats
            self.train_stats = train_stats
            return True
        return False


    def train( self,
               dataspec,
               param = None
             ):
        """ Return a libSVM model trained for a given set
            of hyperparameters 'param' and some dataset
        """
        if self.verbosity > 3:
            print "SVM::train() called ", dataspec.keys()
        
        if param == None:
            if not self.best_param:
                if self.verbosity > 0:
                    print "WARNING: train() called without hyper-parameters "+\
                          "and no best hyperparameters found => run() called"
                return self.run(dataspec)
            param = self.best_param.copy()
        elif 'kernel_type' not in param:
            param['kernel_type'] = self.kernel_type
        if self.verbosity > 1:
            print "launching libsvm with param %s " % param    
        
        trainset = self.train_inputspec(dataspec)
        if self.recompute_stats_at_each_train:
            self.stats_are_uptodate= False
        self.get_data_stats( trainset )

        train_samples, train_targets = self.get_svminputlist( trainset, True )
        # one-against-one strategy is the only one implemented in libsvm
        if self.multiclass_strategy == 'onevsone':
            if self.balanceC:
                param.update({'weight':self.weight,
                              'nr_weight':len(self.weight),
                              'weight_label':self.labels})
            train_problem = svm_problem( train_targets ,
                                         train_samples )
            hyperparam= copy.deepcopy(param)
            hyperparam.update(self.additional_libsvm_params)
            model = svm_model(train_problem, self.get_libsvm_param( hyperparam ) )
        elif self.multiclass_strategy == 'onevsall':
            model = []
            for c in range(self.nclasses):
                if self.balanceC:
                    p = self.class_priors[c]
                    cst = 2*p*(1-p) # constant to have average 1 on weights
                    param.update({'weight':[ cst/p,cst/(1-p) ],
                                  'nr_weight':2,
                                  'weight_label':[1,-1]})
                onevsall_targets = [ int(t)==c and 1 or -1 for t in train_targets ]
                train_problem = svm_problem( onevsall_targets ,
                                             train_samples )
                hyperparam= copy.deepcopy(param)
                hyperparam.update(self.additional_libsvm_params)
                model.append( svm_model(train_problem, self.get_libsvm_param( hyperparam ) ) )
        else:
            raise ValueError, "Unknown value %s for option 'multiclass_strategy'" % self.multiclass_strategy
        
        if param == self.best_param:
            self.best_model = model
        self.model = model
        self.param = param
        
        return dataspec
        
    def predict_from_outputs(self, outputs, targets, vmat):
        """ This function can be changed by the user to take another
            decision than the standard ones, for instance when
            classifying bags of data (where each bag correspond to a target)
        """
        assert self.nclasses == len(outputs[0])
        assert type(outputs[0]) == list
        predictions = []
        for o in outputs:
            predictions.append( numpy.array(o).argmax() )
        return predictions, targets, outputs

    def get_outputs_targets( self,
                             testset ):
        assert testset <> None
        # By default take the LAST model trained
        if self.model <> None:
            model = self.model
        # unless self.model is None
        else:
            assert self.best_model <> None
            model = self.best_model

        samples, targets = self.get_svminputlist( testset )
        nclasses = self.nclasses
        nsamples = len(samples)

        ## The following is specific to libsvm
        ##
        # Note: model.predict_values(x)
        #           gives a dictionary with the SVM scores for one-against-one
        #       model.predict(x)
        #           gives the prediction by "max win" strategy (each SVM has a vote 1 for a class)
        #       model.predict_probability(x)
        #           gives a dictionary with proabilities corresponding to each class
        if self.multiclass_strategy == 'onevsone':
            outputs = []
            if self.outputs_type == 'proba':
                for x in samples:
                    prd, prb = model.predict_probability(x)
                    output = [0]*nclasses
                    for c in prb:
                        output[int(c)] = prb[c]
                    outputs.append(output)
            elif self.outputs_type == 'onehot':
                for x in samples:
                    output = [0]*nclasses
                    output[ int(model.predict(x)) ] += 1
                    outputs.append( output )                
            elif self.outputs_type == 'votes':
                for x in samples:
                    output = [0]*nclasses
                    onevsone_dict = model.predict_values(x)
                    for cl1cl2 in onevsone_dict:
                        if onevsone_dict[cl1cl2] > 0:
                            output[cl1cl2[0]] += 1
                    outputs.append( output )
            elif self.outputs_type == 'dist':
                for x in samples:
                    output = [0]*nclasses
                    onevsone_dict = model.predict_values(x)
                    for cl1cl2 in onevsone_dict:
                        if onevsone_dict[cl1cl2] > 0:
                            output[cl1cl2[0]] += onevsone_dict[cl1cl2]
                    outputs.append( output )
            elif self.outputs_type == 'softvotes':
                for x in samples:
                    output = [0]*nclasses
                    onevsone_dict = model.predict_values(x)
                    for cl1cl2 in onevsone_dict:
                        if onevsone_dict[cl1cl2] > 0:
                            output[cl1cl2[0]] += softvalue( onevsone_dict[cl1cl2] )
                    outputs.append( output )
            else:
                raise ValueError, "Unknown value %s for option 'outputs_type'" % self.outputs_type
        elif self.multiclass_strategy == 'onevsall':
            assert type(model) == list
            outputs = []
            if self.outputs_type == 'proba':
                for x in samples:
                    output = [0]*nclasses
                    for c in range(nclasses):
                        prd, prb = model[c].predict_probability(x)
                        output[c] = prb[1]
                    outputs.append( softmax(output) )
            elif self.outputs_type == 'onehot' or self.outputs_type == 'votes':
                for x in samples:
                    output = [0]*nclasses
                    svm_outputs = numpy.zeros(nclasses)
                    for c in range(nclasses):
                        onevsall_dict = model[c].predict_values(x)
                        svm_outputs[c] = onevsall_dict[(1,-1)]
                    output[ svm_outputs.argmax() ] += 1
                    outputs.append( output )                
            elif self.outputs_type == 'dist':
                for x in samples:
                    output = [0]*nclasses
                    for c in range(nclasses):
                        onevsall_dict = model[c].predict_values(x)
                        output[c] = onevsall_dict[(1,-1)]
                    outputs.append( output )
            elif self.outputs_type == 'softvotes':
                for x in samples:
                    output = [0]*nclasses
                    for c in range(nclasses):
                        onevsall_dict = model[c].predict_values(x)
                        output[c] = softvalue( onevsall_dict[(1,-1)] )
                    outputs.append( output )
            else:
                raise ValueError, "Unknown value %s for option 'outputs_type'" % self.outputs_type
        else:
            raise ValueError, "Unknown value %s for option 'multiclass_strategy'" % self.multiclass_strategy

        assert len(outputs) == len(targets)
        return outputs, targets

    def test( self,
              testset,
              teststats = None,
              return_outputs = False
             ):
        """ Return the costs obtained by a libSVM model
            on a given dataset.
            If 'return_outputs' is set to True, also returns a numpy array
            containing outputs.
        """
        nclasses = self.nclasses
        costnames = self.costnames
        # Translation of the cost 'confusion_matrix'
        # to several costs (one per matrix component)
        for cn, icost in zip(costnames, range(len(costnames)) ):
            if cn == 'confusion_matrix':
                costnames = costnames[:icost] \
                            + [ 'cm_%s%s' % (i,j) for i in range(nclasses)
                                                  for j in range(nclasses) ] \
                            + costnames[icost+1:]                
                self.costnames = costnames
                break
        if teststats == None:
            teststats= VecStatsCollector()        
            teststats.setFieldNames( costnames )

        outputs, targets = self.get_outputs_targets( testset )
        predictions, targets, outputs = self.predict_from_outputs( outputs, targets, testset)

        # Computing misclassification costs for the default normalized
        # classification error (= class error weighted w.r.t class priors)
        
        class_priors = self.get_class_priors( targets )
                                            # [int(t) for t in targets ]
        if self.verbosity > 3:
            print "SVM::test() called on ",len(targets)," samples"
            print "class priors: ", class_priors

        cm_weights = [ (class_priors.get(t,0.)!=0. and 1./class_priors.get(t,0.)) or fpconst.NaN for t in range(nclasses) ]
        if 'norm_ce' in self.costnames:
            if self.errorcosts == None:        
                errorcosts = numpy.zeros((nclasses,nclasses))
                for classe in range(nclasses):
                    cp= class_priors.get(classe,0.)
                    for prediction in range(classe)+range(classe+1,nclasses):
                        if cp != 0.:
                            errorcosts[classe,prediction] = 1./ ( nclasses * cp )
                        else:
                            errorcosts[classe,prediction] = fpconst.NaN           
            else:
                errorcosts = self.errorcosts

        for prediction, t in zip( predictions, targets ):
            truth = int(t)
            statVec = []
            for cn in costnames:
                if cn == 'class_error':
                    if truth == prediction:
                        statVec.append(0.)
                    else:
                        statVec.append(1.)
                elif cn[:3] == 'cm_':
                    c1 = int(cn[3])
                    c2 = int(cn[4])
                    if c1 == truth and c2 == prediction:
                        statVec.append(cm_weights[truth])
                    elif fpconst.isNaN(cm_weights[c1]):
                        statVec.append(fpconst.NaN)
                    else:
                        statVec.append(0.)
                elif cn == 'norm_ce':
                    statVec.append( errorcosts[truth, prediction] )
                #elif cn == 'n_sv': # number of support vector (not done)
                #    statVec.append( svmc.svm_get_nr_sv(model) )
                else:
                    raise ValueError, "computation of cost %s not implemented in SVM::test()" % cn
            teststats.update(statVec,1.)

        if return_outputs:
            return teststats, numpy.array(outputs)
        else:
            return teststats #, outputs, costs


    def valid( self,
               dataspec,
               param= None,
               return_outputs = False):
        if self.valid_inputspec(dataspec) <> None:
            self.validtype = 'simple'
            return self.simplevalid(dataspec, param, return_outputs = return_outputs)
        else:
            return self.crossvalid(dataspec, param, return_outputs = return_outputs)

    def simplevalid( self,
                     dataspec,
                     param,
                     validstats = None,
                     verbose = True,
                     return_outputs = False ):
        """ Return the costs obtained by (simple) validation
            for a given set of hyperparameter.
        """
        if self.verbosity > 0 and verbose:
            print "\n** Simple Validation"
            print "   with param %s" % param
        self.train(dataspec, param)
        validset = self.valid_inputspec(dataspec)
        self.validset = validset
        return self.test( validset, validstats, return_outputs )


    def crossvalid( self,
                    dataspec,
                    param,
                    return_outputs = False ):
        """ Return the costs obtained by cross-validation
            for a given set of hyperparameter.
        """
        nclasses = self.nclasses
        n_fold = self.n_fold
        self.validtype = '%s-fold' % n_fold
        if self.verbosity > 0:
            print "\n** %d-fold Cross Validation" % n_fold
            print "   with param %s" % param
        
        trainset = self.train_inputspec(dataspec)
        if not self.intern_kfold:
            # determining an internal k-fold that keep the class proportions inside each split
            trainset_class = [None]*nclasses
            N=[0]*nclasses
            Nfold=[0]*nclasses
            Nlastfold=[0]*nclasses
            for c in range(nclasses):
                trainset_class[c] = ClassSubsetVMatrix( source = trainset,
                                                        classes = [c],)
                N[c] = trainset_class[c].length
                Nfold[c] = int(N[c] / n_fold)
                if Nfold[c] == 0:
                    raise ValueError, "%d-fold cross-validation is not possible as class %d has only %d samples" % \
                                     (n_fold, c, N[c])
                Nlastfold[c] = N[c] - Nfold[c] * (n_fold-1)
            sub_trainset_class = [None]*nclasses
            sub_testset_class = [None]*nclasses
        
        validstats = None
        if return_outputs:
            validoutputs = numpy.zeros((trainset.length, nclasses))
            
        for i_fold in range(n_fold):
        
            if self.intern_kfold:
                assert isinstance(self.intern_kfold,list)
                assert len(self.intern_kfold) == n_fold
                assert isinstance(self.intern_kfold[0],list)
                assert len(self.intern_kfold[0]) == 2
                train_indices = self.intern_kfold[i_fold][0]
                test_indices = self.intern_kfold[i_fold][1]
                
                print "Loading suggested intern k-fold --  train: ", len(train_indices), "  -- test: ", len(test_indices)
                
                sub_testset = SelectRowsVMatrix(
                                source = trainset,
                                indices = test_indices,)
                sub_trainset= SelectRowsVMatrix(
                                source = trainset,
                                indices = train_indices,)
            else:
                test_indices = []
                for c in range(nclasses):
                    if i_fold < n_fold-1:
                        test_indices_class = range( i_fold*Nfold[c], (i_fold+1)*Nfold[c] )
                        train_indices_class = range( 0,i_fold*Nfold[c])+range((i_fold+1)*Nfold[c], N[c] )
                    else:
                        test_indices_class = range( i_fold*Nfold[c], N[c] )
                        train_indices_class = range( 0, N[c]-Nlastfold[c] )
                    sub_testset_class[c] = SelectRowsVMatrix(
                                        source = trainset_class[c],
                                        indices = test_indices_class,)
                    sub_trainset_class[c] = SelectRowsVMatrix(
                                        source = trainset_class[c],
                                        indices = train_indices_class,)
                    test_indices += test_indices_class
                sub_testset = ConcatRowsVMatrix( sources = sub_testset_class,
                                                 fully_check_mappings = False,)
                sub_trainset = ConcatRowsVMatrix( sources = sub_trainset_class,
                                                 fully_check_mappings = False,)
            if return_outputs:
                validstats, outputs = self.simplevalid( 
                                        {   self.trainset_key:sub_trainset,
                                            self.validset_key:sub_testset},
                                        param,
                                        validstats = validstats, verbose = False, return_outputs = True)
                for i, o in enumerate(outputs):
                    validoutputs[test_indices[i]] = o
                validoutputs = numpy.array(list(validoutputs)+list(outputs))
            else:
                validstats = self.simplevalid(
                                        {   self.trainset_key:sub_trainset,
                                            self.validset_key:sub_testset},
                                        param,
                                        validstats = validstats, verbose = False)
        if return_outputs:
            return validstats, validoutputs
        else:
            return validstats


    def retrain_and_writeresults(self, dataspec):
        trainset = self.train_inputspec(dataspec)
        validset = self.valid_inputspec(dataspec)
        testset  = self.test_inputspec(dataspec)
        # Cross Validation
        if self.validset_key not in dataspec:
            if self.verbosity > 0:
                print "\n** training model on entire train\n\twith param %s" % self.best_param
            self.train( dataspec )

        # Simple Validation
        else:
            if self.retrain_on_valid:
                """ Uncomment following lines if you want to check that
                    retraining on {train + valid} sets does not degrade test performance.
                """
                #train_stats = None
                #test_stats = None
                #if self.test_on_train:
                #    if self.verbosity > 2:
                #        print "\n** (testing simple valid on "+self.trainset_key+")"
                #    train_stats = self.test( trainset )
                #if testset <> None:
                #    if self.verbosity > 2:
                #        print "\n** (testing simple valid on "+self.testset_key+")"
                #test_stats = self.test( testset  )
                #
                #self.write_results( self.best_param,
                #                    valid_stats, test_stats, train_stats )
                self.validtype = 'simple+retrain'
                tv_set = ConcatRowsVMatrix(
                            sources = [ trainset,
                                        validset
                                        ],
                            fully_check_mappings = False,
                        )
                if self.verbosity > 0:
                    print "\n** re-training model on { train + valid }\n\twith params %s" % (self.best_param)
                self.train( {self.trainset_key: tv_set} )

            # CAUTION: in the case of simple validation without retraining on {train+valid},
            #          self.best_model is supposed to be up-to-date or None
            elif self.best_model == None:
                self.validtype = 'simple'
                self.train( dataspec )

        train_stats = None
        test_stats = None
        if self.test_on_train:
            if self.verbosity > 2:
               print "\n** (testing on "+self.trainset_key+")"
            train_stats = self.test( trainset )
        if testset <> None:
            if self.verbosity > 2:
                print "\n** (testing on "+self.testset_key+")"
            test_stats = self.test( testset )
        self.update_trials( self.best_param,
                            None, test_stats, train_stats )
        self.write_results( self.best_param,
                            self.valid_stats, self.test_stats, self.train_stats,
                            ntrain = (trainset <> None and trainset.length or 0),
                            nvalid = (validset <> None and validset.length or 0),
                            ntest = (testset <> None and testset.length or 0) )
        return dataspec

    def run(self, dataspec, param = None, L0 = None, save_valid_outputs = False):
        """ THE interesting function of the class.
            See __main__ below for usage.
            dataspec is a dictionary which specifies train, valid, test sets.
            The train set is mandatory, but valid and/or test sets can be missing.
            cf. train_inputspec(), valid_inputspec(), and test_inputspec().
        """
        random.seed(17)
        assert self.testlevel >= 0
        assert self.max_ntrials > 0
        trainset = self.train_inputspec(dataspec)
        validset = self.valid_inputspec(dataspec)
        testset  = self.test_inputspec(dataspec)

        # Input statistics are needed to choose the first values of some hyperparameters
        self.get_data_stats( trainset )

        expert = self.get_expert( self.kernel_type )
        expert.verbosity = self.verbosity
        
        if L0 == None:
            L0=len(expert.trials_param_list)
        local_retrain_until_local_optimum_is_found = True

        # HyperParamOracle__kernel.best_param is None just at the __init__
        if param == None or len(param) == 0:
            if expert.best_param  == None:
                param_to_try = expert.choose_first_param()
            else:
                param_to_try = expert.choose_new_param()
        else:
            param_to_try = [param]
            local_retrain_until_local_optimum_is_found = False
        
        for param in param_to_try:

            if self.max_ntrials == 1: # No hyper-optimization (debug)
                self.best_param = param
                self.validset = None
                return self.retrain_and_writeresults(dataspec)
            
            if save_valid_outputs:
                valid_stats, valid_outputs = self.valid(dataspec, param, return_outputs = True)
            else:
                valid_stats = self.valid(dataspec, param, return_outputs = False)

            # No improvement measured
            if not self.update_trials( param, valid_stats ):

                # We reject the model (to avoid testing on it)
                self.model = None
                self.write_results( param, valid_stats, None, None,
                                    ntrain = (trainset <> None and trainset.length or 0),
                                    nvalid = (validset <> None and validset.length or 0),
                                    ntest = (testset <> None and testset.length or 0),
                                    only_stdout = self.testlevel < 2  )

            # Better valid cost is obtained!
            else:
                if save_valid_outputs:
                    self.valid_outputs = valid_outputs
            
                # Simple Validation
                if validset <> None:
                    self.best_model = self.model
                    
                if self.testlevel > 0:
                    self.retrain_and_writeresults(dataspec)
                else:
                    self.write_results( param, valid_stats, None, None,
                                        ntrain = (trainset <> None and trainset.length or 0),
                                        nvalid = (validset <> None and validset.length or 0),
                                        ntest = (testset <> None and testset.length or 0),
                                        only_stdout = True  )

            if( len(expert.trials_param_list)-L0 >= self.max_ntrials
            or  ( self.min_cost <> None and expert.best_cost <= self.min_cost ) ):
                local_retrain_until_local_optimum_is_found = False
                break

        if( self.retrain_until_local_optimum_is_found
        and local_retrain_until_local_optimum_is_found
        and expert.should_be_tuned_again()
        and ( self.max_cost == None or expert.best_cost <= self.max_cost ) ):
           return self.run( dataspec, param = None, save_valid_outputs = save_valid_outputs, L0 = L0 )

        if self.testlevel == 0:
             return self.retrain_and_writeresults(dataspec)

        return dataspec

""" Some AUXILIARY FUNCTIONS, to have access or modify 
    the input statistics.
"""
def get_std_cmp(data,i):
    values=[float(vec[i]) for vec in data]
    tot = sum(values)
    avg = tot*1.0/len(values)
    sdsq = sum([(i-avg)**2 for i in values])
    return (sdsq*1.0/(len(values)-1 or 1))**.5

def mean_std(data):
    stds=[get_std_cmp(data,i) for i in range(len(data[0]))]
    while 0 in stds:
        stds.remove(0)
    stds=numpy.array(stds)
    return stds.mean(), stds.std()

def get_mean_cmp(data,i):
    values=[float(vec[i]) for vec in data]
    return  sum(values)/len(values)

def normalize_data(data, mean=None, std=None):
    if mean == None:
        mean=[]
        for i in range(len(data[0])):
            mean.append( get_mean_cmp(data,i) )
    if std == None:
        std=[]
        for i in range(len(data[0])):
            std_tmp=get_std_cmp(data,i)
            if std_tmp == 0.:
                std.append( 1. )
            else:
                std.append( std_tmp )
    for i in range(len(data[0])):
        for j in range(len(data)):
            data[j][i]=(data[j][i]-mean[i])/std[i]
    return mean, std

def geom_mean(data):
    """ Return geometric mean (useful to deal with multiplicative hyperparameters
                        such as 'C', 'gamma', ...)
    """
    if type(data[0]) == list:
        res=[]
        for coor in range(len(data[0])):
            res.append( geom_mean( [data[i][coor] for i in range(len(data))] ) )
        return res
    else:
        prod = 1.0
        for value in data:
            prod *= value
        return prod**(1./len(data))

def softmax(output):
    """ Softmax function
    """
    try:
        expterms = [ math.exp(o) for o in output ]
    except OverflowError:
        MAX= max(output)
        expterms = [ int(o==MAX) for o in output ]
    S = sum(expterms)
    return [ e/S for e in expterms ]

def softvalue(value):
    return softmax([ value, - value ])[0]

""""""

""" Below:
    Some functions that can be assigned
    to a SVM object, to deal with bags of data
    to classify.

# Usage:
#-------
    
svm = SVM()
svm.get_datalist = get_datalist_onBags
svm.predict_from_outputs = predict_from_outputs_onBags

"""         
def get_baginfo( input_vmat ):
    data_array = input_vmat.getMat()
    inputsize = input_vmat.inputsize
    targetsize = input_vmat.targetsize
    assert targetsize == 2
    baginfo = [ int(t) for t in data_array[:,inputsize+1] ]
    return baginfo

def get_datalist_onBags( input_vmat ):
    data_array = input_vmat.getMat()
    inputsize = input_vmat.inputsize
    samples   = [ [ float(x_t_i)    for x_t_i in x_t ]
                                    for x_t in data_array[:,:inputsize] ]
    targets   = [ float(t) for t in data_array[:,inputsize] ]
    return samples, targets

def predict_from_outputs_onBags(outputs, targets, vmat):
    baginfo = get_baginfo(vmat)
    assert len(outputs) == len(targets) == len(baginfo)
    nclasses = len(outputs[0])
    assert type(outputs[0]) == list
    bag_predictions=[]
    bag_targets=[]
    bag_outputs=[]
    sum_outputs = numpy.zeros(nclasses)
    for output, t, b in zip(outputs, targets, baginfo):
        if b in [1,3]: # beginning of a bag
            sum_outputs = numpy.zeros(nclasses)
        for o, c in zip(output, range(nclasses)):
            sum_outputs[c] += o
        if b in [2,3]: # end of a bag
            bag_predictions.append( sum_outputs.argmax() )
            bag_targets.append( t )
            bag_outputs.append( sum_outputs )
    return bag_predictions, bag_targets, numpy.array(bag_outputs)


if __name__ == '__main__':

    import os.path


    """ =============================================================== """
    """ 1. BUILD a SVM Hyper-Optimizer. """

    svm=SVM()
    svm.costnames = ['class_error']
    svm.verbosity = 1

    """ ............................................................... """
    """ Uncomment following lines to change default values (indicated). """

    """ <bool> weight the coeff 'C' with the inverse prior proba of each class
               (scaled so as to have average=1 on weights). Useful when classes are unbalanced. """
    # svm.balanceC = True

    """ <list> of cost names to compute. """
    # svm.costnames = ['class_error','confusion_matrix']

    """ <string> name of the final cost to be minimized. """
    # svm.maincost_name = svm.costnames[0]

    """ <bool> let the algo automatically decide when to stop tuning hyperparameters. """
    # svm.retrain_until_local_optimum_is_found = 1

    """ <bool> after simple validation, retrain a model on {train + valid sets} before computing test error. """
    # svm.retrain_on_valid = 1

    """ <int> verbosity level. """
    # svm.verbosity = 0

    """ ............................................................... """
    """ =============================================================== """


    """ =============================================================== """
    """ 2. PREPARE inputs and outputs. """

    inputPath = '/u/larocheh/myUserExp/deep_non-local_benchmark/data/mnist-rotation'
    outputPath = '.'

    train_file, valid_file, test_file = [ os.path.join(inputPath,
                                              'mnist_all_rotation_normalized_float_'+dataType+'.amat')
                                          for dataType in ['train','valid','test']
                                        ]

    trainset, validset, testset = [ pl.AutoVMatrix(
                                        specification = filename,
                                        inputsize = 784,
                                        targetsize = 1,
                                    )
                                    for filename in [ train_file, valid_file, test_file ]
                                  ]

    dataspec = {'trainset':trainset,
                'validset':validset,
                'testset': testset,
               }
    """ Note1: This dataspec corresponds to the standard setting for simple validation.
               -> Do not specify any 'validset' to do cross-validation.
               -> Do not specify any 'testset' if you do not want to check the test error
                  during the model selection.
        Note2: Keys of this dictionary can be changed using svm.trainset_key ['trainset'],
               svm.validset_key ['validset'] and svm.testset_key ['testset'].
    """

    svm.results_filename = os.path.join( outputPath,
                                         'RES_%s_svm' % ( os.path.basename(train_file) )
                           )

    """ ................................................................. """
    """ Uncomment following lines to change default values (empty lists). """

    """ <list> of front-end hyperparameter names and values, to report in results_filename. """
    # svm.preproc_optionnames = [ 'train_file', 'simple_valid' ]
    # svm.preproc_optionvalues = [ train_file,   svm.validset_key in dataspec ]
    """ ................................................................. """
    """ =============================================================== """


    """ =============================================================== """
    """ 3. RUN EXPERIMENTS. """


    normalize_inputs = 0
    outputs_type = 'votes'
    
    svm.normalize_inputs = normalize_inputs
    svm.outputs_type = outputs_type
    
    svm.preproc_optionnames = [ 'renormalize_inputs', 'outputs_type' ]
    svm.preproc_optionvalues = [ normalize_inputs ,  outputs_type ]
    
    svm.kernel_type = 'poly'
    svm.run(dataspec)

    svm.kernel_type = 'rbf'
    svm.run(dataspec)

    svm.kernel_type = 'linear'
    svm.run(dataspec)

    """ =============================================================== """
