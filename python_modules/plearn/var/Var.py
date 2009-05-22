## Copyright (C) 2007 Pascal Vincent
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##  1. Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##
##  2. Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
##  3. The name of the authors may not be used to endorse or promote
##     products derived from this software without specific prior written
##     permission.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
## IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
## OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
## NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
## TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
## PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
## LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
## NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## This file is part of the PLearn library. For more information on the PLearn
## library, go to the PLearn Web site at www.plearn.org
##
## Authors: Pascal Vincent

from plearn.pyplearn import *
from plearn.pyplearn.plearn_repr import plearn_repr, format_list_elements

from math import sqrt
    
class Var:

    def __init__(self, l, w=1, random_type="none", random_a=0., random_b=1., random_clear_first_row=False, varname="", min_value = None, max_value = None):
        if isinstance(l, Var):
            self.v = l.v
        elif isinstance(l,int):
            if min_value == None and max_value == None:
                self.v = pl.SourceVariable(build_length=l,
                                           build_width=w,
                                           random_type=random_type,
                                           random_a=random_a,
                                           random_b=random_b,
                                           random_clear_first_row=random_clear_first_row,
                                           varname=varname)
            elif min_value == None:
                pass
            elif max_value == None:
                pass
            else:
                self.v = pl.SourceVariable(build_length=l,
                                           build_width=w,
                                           random_type=random_type,
                                           random_a=random_a,
                                           random_b=random_b,
                                           random_clear_first_row=random_clear_first_row,
                                           varname=varname,
                                           min_value = min_value,
                                           max_value = max_value)
                
                
        else: # assume parameter l is a pl. plvar
            self.v = l

    def _serial_number(self):
        return self.v._serial_number()

    def exp(self):
        return Var(pl.ExpVariable(input=self.v))

    def sigmoid(self):
        return Var(pl.SigmoidVariable(input=self.v))

    def sigmoidInRange(self, minp=0.0, maxp=1.0):
        if minp==0.0 and maxp==1.0:
            return self.sigmoid()
        else:
            return (self*(maxp-minp)+minp).sigmoid()

    def tanh(self):
        return Var(pl.TanhVariable(input=self.v))

    def plearn_repr( self, indent_level=0, inner_repr=plearn_repr ):
        # asking for plearn_repr could be to send specification over
        # to another prg so that will open the .pmat
        # So we make sure data is flushed to disk.
        return self.v.plearn_repr(indent_level, inner_repr)

    def probabilityPairs(self, min, max, varname=""):
        return Var(pl.ProbabilityPairsVariable(input=self.v, min=min, max=max, varname=varname))
    
    def probabilityPairsInverse(self, min, max, varname=""):
        return Var(pl.ProbabilityPairsInverseVariable(input=self.v, min=min, max=max, varname=varname))

    def transpose(self):
        return Var(pl.TransposeVariable(input=self.v))

    def matrixProduct(self, other):
        return self.matrixProduct_A_B(other)
    
    def matrixProduct_A_B(self, other):
        return Var(pl.ProductVariable(input1=self.v, input2=other.v))

    def matrixProduct_At_B(self, other):
        return Var(pl.TransposeProductVariable(input1=self.v, input2=other.v))

    def matrixProduct_A_Bt(self, other):
        return Var(pl.ProductTransposeVariable(input1=self.v, input2=other.v))

    def matrixProduct_At_Bt(self, other):
        return other.matrixProduct(self).transpose()         

    def affineTransform(self, W):
        return Var(pl.AffineTransformVariable(self.v, W))

    # TODO: verifier si NegCrossEntropySigmoidVariable est bien ce qui est utilise par Hugo
    def negCrossEntropySigmoid(self, other, regularizer=0, ignore_missing=False):
        return Var(pl.NegCrossEntropySigmoidVariable(input1=self.v, input2=other.v, regularizer=regularizer, ignore_missing=ignore_missing))
        
    def doubleProduct(self, W, M):
        return Var(pl.DoubleProductVariable(varray=[self.v, W, M]))

    def logSoftMax(self):
        return Var(pl.LogSoftmaxVariable(input=self.v))

    def getElement(self, index):
        """Returns a Var that selects an element from the current Var
        index must be another scalar variable whose value will be
        interpreted as an integer index of the element to select."""
        return Var(pl.VarElementVariable(input1=self.v, input2=index.v))

    def __getitem__(self, index):
        return self.getElement(index)

    def dot(self, other):
        return Var(pl.DotProductVariable(input1=self.v, input2=other.v))

    def multiMax(self, igs, computation_type):        
        return Var(pl.MultiMaxVariable(input=self.v, groupsize=igs, computation_type=PLChar(computation_type)))

    def multiSoftMax(self, igs):        
        return self.multiMax(igs, 'S')

    def multiLogSoftMax(self, igs):
        return self.multiMax(igs, 'L')

    def multiSample(self, gs):
        return Var(pl.MultiSampleVariable(input=self.v, groupsize=gs))

    def bernoulliSample(self):
        return Var(pl.BernoulliSampleVariable(input=self.v))

    def timesConstantScalarVariable2(self, v2):
        return Var(pl.TimesConstantScalarVariable2(input1=self.v, input2=v2.v))

    def transposeDoubleProduct(self, W, M):
        return Var(pl.TransposedDoubleProductVariable(varray=[self.v, W, M]))

    def sumsquare(self):
        return Var(pl.SumSquareVariable(input=self.v))

    def square(self):
        return Var(pl.SquareVariable(input=self.v))

    def add(self, other):
        if type(other) in (int, float):
            return Var(pl.PlusConstantVariable(input=self.v, cst=other))
        else:
            return Var(pl.PlusVariable(input1=self.v, input2=other.v))

    def classificationLoss(self, class_index):
        return Var(pl.ClassificationLossVariable(input1=self.v, input2=class_index.v))
    
    def __add__(self, other):
        return self.add(other)

    def __sub__(self, other):
        return Var(pl.MinusVariable(input1=self.v,input2=other.v))

    def __mul__(self, other):
        if type(other) in (int, float):
            return Var(pl.TimesConstantVariable(input=self.v, cst=other))
        else:
            raise NotImplementedError

    def neg(self):
        return Var(pl.NegateElementsVariable(input=self.v))

    def __neg__(self):
        return self.neg()

####################################################
# Operations implemented as functions


def hconcatVars(varlist):
    return Var(pl.ConcatColumnsVariable(varray=varlist))

def vconcatVars(varlist):
    return Var(pl.ConcatRowsVariable(varray=varlist))
    

###################################################    
# RLayer stands for reconstruction hidden layer


def addSigmoidTiedRLayer(input, iw, ow, add_bias=True, basename=""):
    """This assumes the input is a (1,iw) matrix,
    and will produce an output that will be a (1, ow) matrix.
    It will create parameter W(ow,iw)
    and an optional parameter b(1,ow)
    Then output = sigmoid(input.W^T + b)
    
    Returns a triple (hidden, reconstruciton_cost, reconstructed_input)"""
    ra = 1./max(iw,ow)
    W = Var(ow,iw,"uniform", -ra, 1./ra, varname=basename+'_W')
    
    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')        
        hidden = input.matrixProduct_A_Bt(W).add(b).sigmoid()        
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        reconstr_activation = hidden.matrixProduct(W).add(br)
    else:
        hidden = input.matrixProduct_A_Bt(W).sigmoid()
        reconstr_activation = hidden.matrixProduct(W)

    reconstructed_input = reconstr_activation.sigmoid()
    cost = reconstr_activation.negCrossEntropySigmoid(input)
    return hidden, cost, reconstructed_input

def addMultiSoftMaxDoubleProductTiedRLayer(input, iw, igs, ow, ogs, add_bias=False, constrain_mask=False, basename="", positive=False):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""

    ra = 1./max(iw,ow)
    sqra = sqrt(ra)

    if positive:
        W = Var(ogs, iw, "uniform", 0, sqra, False, varname=basename+"_W", min_value=0, max_value=1e100)
        M = Var(ow/ogs, iw, "uniform", 0, sqra, False, varname=basename+"_M", min_value=0, max_value=1e100)
    else:
        W = Var(ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_W")
        M = Var(ow/ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_M")
       
    if constrain_mask:
        M = M.sigmoid()
    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')
        hidden = input.doubleProduct(W,M).add(b).multiSoftMax(ogs)
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        log_reconstructed = hidden.transposeDoubleProduct(W,M).add(br).multiLogSoftMax(igs)
    else:
        hidden = input.doubleProduct(W,M).multiSoftMax(ogs)
        log_reconstructed = hidden.transposeDoubleProduct(W,M).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxDoubleProductRLayer(input, iw, igs, ow, ogs, add_bias=False, constrain_mask=False, basename="", positive=False):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""
    
    #TODO: constrain_mask is not used...
    if constrain_mask:
        raise Exception("not implemented yet")
    
    ra = 1./max(iw,ow)
    sqra = sqrt(ra)

    if positive:
        W = Var(ogs, iw, "uniform", 0, sqra, False, varname=basename+"_W", min_value=0, max_value=1e100)
        M = Var(ow/ogs, iw, "uniform", 0, sqra, False, varname=basename+"_M", min_value=0, max_value=1e100)
    else:           
        M = Var(ow/ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_M")
        W = Var(ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_W")
    
    if positive:
        Mr = Var(iw/igs, ow, "uniform", 0, sqra, False, varname=basename+"_Mr", min_value=0, max_value=1e100)
        Wr = Var(igs, ow, "uniform", 0, sqra, False, varname=basename+"_Wr", min_value=0, max_value=1e100)
    else:
        Mr = Var(iw/igs, ow, "uniform", -sqra, sqra, False, varname=basename+"_Mr")
        Wr = Var(igs, ow, "uniform", -sqra, sqra, False, varname=basename+"_Wr")
        
    # TODO: a repenser s'il faut un transpose ou non
    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')        
        hidden = input.doubleProduct(W,M).add(b).multiSoftMax(ogs)
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        log_reconstructed = hidden.doubleProduct(Wr,Mr).add(br).multiLogSoftMax(igs)
    else:    
        hidden = input.doubleProduct(W,M).multiSoftMax(ogs)    
        log_reconstructed = hidden.doubleProduct(Wr,Mr).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxMixedProductRLayer(input, iw, igs, ow, ogs, add_bias=False, basename=""):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""
    ra = 1./max(iw,ow)
    sqra = sqrt(ra)
    M = Var(ow/ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_M")
    W = Var(ogs, iw, "uniform", -sqra, sqra, False, varname=basename+"_W")
    Wr = Var(ow, iw, "uniform", -ra, ra, varname=basename+'_Wr')

    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')
        hidden = input.doubleProduct(W,M).add(b).multiSoftMax(ogs)
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        log_reconstructed = hidden.matrixProduct(Wr).add(br).multiLogSoftMax(igs)
    else:
        hidden = input.doubleProduct(W,M).multiSoftMax(ogs)
        log_reconstructed = hidden.matrixProduct(Wr).multiLogSoftMax(igs)

    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxSimpleProductTiedRLayer(input, iw, igs, ow, ogs, add_bias=False, basename="", positive=False, stochastic_sample=False):
    
    sup = 1./max(iw,ow)
    if positive:
        W = Var(ow, iw, "uniform", 0, sup, varname=basename+'_W', min_value=0, max_value=1e100)
    else:
        W = Var(ow, iw, "uniform", -sup, sup, varname=basename+'_W')

    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')
        hidden = input.matrixProduct_A_Bt(W).add(b).multiSoftMax(ogs)
        if stochastic_sample:
            hidden = hidden.multiSample(ogs)
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        log_reconstructed = hidden.matrixProduct(W).add(br).multiLogSoftMax(igs)
    else:        
        hidden = input.matrixProduct_A_Bt(W).multiSoftMax(ogs)
        if stochastic_sample:
            hidden = hidden.multiSample(ogs)
        log_reconstructed = hidden.matrixProduct(W).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxSimpleProductRLayer(input, iw, igs, ow, ogs, add_bias=False, basename="", positive=False, stochastic_sample=False):
    ra = 1./max(iw,ow)

    if positive:
        W = Var(ow, iw, "uniform", 0, ra, varname=basename+'_W',min_value=0, max_value=1e100)
        Wr = Var(ow, iw, "uniform", 0, ra, varname=basename+'_Wr', min_value=0, max_value=1e100)
    else :            
        W = Var(ow, iw, "uniform", -ra, ra, varname=basename+'_W')
        Wr = Var(ow, iw, "uniform", -ra, ra, varname=basename+'_Wr')
        
    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')
        hidden = input.matrixProduct_A_Bt(W).add(b).multiSoftMax(ogs)
        if stochastic_sample:
            hidden = hidden.multiSample(ogs)
        br = Var(1,iw,"fill",0, varname=basename+'_br')
        log_reconstructed = hidden.matrixProduct(Wr).add(br).multiLogSoftMax(igs)
    else:
        hidden = input.matrixProduct_A_Bt(W).multiSoftMax(ogs)
        if stochastic_sample:
            hidden = hidden.multiSample(ogs)
        log_reconstructed = hidden.matrixProduct(Wr).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

#################################
# These build supervised layers

def addLinearLayer(input, iw, ow, add_bias=True, basename=""):
    """This assumes the input is a (1,iw) matrix,
    and will produce an output that will be a (1, ow) matrix.
    It will create parameter W(ow,iw)
    and an optional parameter b(1,ow)
    Then output = input.W^T + b    
    Returns the output layer"""
    W = Var(ow,iw,"uniform", -1./iw, 1./iw, varname=basename+'_W')
    
    if add_bias:
        b = Var(1,ow,"fill",0, varname=basename+'_b')
        out = input.matrixProduct_A_Bt(W).add(b)
    else:
        out = input.matrixProduct_A_Bt(W)
    return out


def addClassificationNegLogSoftmaxLayer(activation, target):
    """Assumes that target is a scalar variable containing a class number between 0 and m-1,
    and activation is a m-dimensional vector of class scores (reals).
    Returns a pair of output, cost variables where:
      output = log(sotmax(activation))
      cost = -output[target]
      """
    output = activation.logSoftMax()
    cost = -output[target]
    return output, cost



## def addMultiSoftmaxRLayer(input, ing, igs, ong, ogs, tied=True, use_double_product=True):
##     """ing is the input's number of groups
##     igs is the input's group size
##     ong is the output's number of groups
##     ogs is the output's group size
##     If tied is true we will use the transpose of the recognition weight matrix for reconstruction
##     If use_double_product is false we'll use a regular weight matrix.
##     If it's true, we'll use the decomposition as a 
##     Returns a triple (hidden, params, reonstruction_cost)"""
##     if not use_double_product:
##         W = Var(ing*igs,ong*ogs)
##         hidden = input.matrixProduct(W).multiSoftMax(ogs)
##         if tied:
##             cost = -hidden.matrixTransposeProduct(W).multiLogSoftMax(igs).dot(input)
##             return hidden, cost, W
##         else:
##             Wr = Var(ong*ogs, ing*igs)
##             cost = -hidden.matrixProduct(Wr).multiLogSoftMax(igs)
##             return hidden, cost, (W, Wr)

##     else: # use double product
##        # M = Var(ing*igs, ong, "uniform", -1/ing/igs, 1/ing/igs, False)
##        # W = Var(ing*igs, ogs, "uniform", -1/ing/igs, 1/ing/igs, False)
##         M = Var(ong, ing*igs, "uniform", -1/ing/igs, 1/ing/igs, False)
##         W = Var(ogs, ing*igs, "uniform", -1/ing/igs, 1/ing/igs, False)
##         hidden = input.doubleProduct(W,M).multiSoftMax(ogs)
##         if tied:
##             cost = -hidden.transposeDoubleProduct(W,M).multiLogSoftMax(igs).dot(input)            
##             return hidden, cost, (W,M)
##         else:
##             Mr = Var()
##             Wr = Var()
            

