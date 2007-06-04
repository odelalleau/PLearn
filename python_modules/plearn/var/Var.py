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

    def __init__(self, l, w=1, random_type="none", random_a=0., random_b=1., random_clear_first_row=False, varname=""):
        if isinstance(l, Var):
            self.v = l.v
        elif isinstance(l,int):
            self.v = pl.SourceVariable(build_length=l,
                                       build_width=w,
                                       random_type=random_type,
                                       random_a=random_a,
                                       random_b=random_b,
                                       random_clear_first_row=random_clear_first_row,
                                       varname=varname)
        else: # assume parameter l is a pl. plvar
            self.v = l

    def _serial_number(self):
        return self.v._serial_number()

    def exp(self):
        return Var(pl.ExpVariable(input=self.v))

    def sigmoid(self):
        return Var(pl.SigmoidVariable(input=self.v))

    def tanh(self):
        return Var(pl.TanhVariable(input=self.v))

    def plearn_repr( self, indent_level=0, inner_repr=plearn_repr ):
        # asking for plearn_repr could be to send specification over
        # to another prg so that will open the .pmat
        # So we make sure data is flushed to disk.
        return self.v.plearn_repr(indent_level, inner_repr)

    def probabilityPairs(self, min, max, varname=""):
        return Var(pl.ProbabilityPairsVariable(input=self.v, min=min, max=max, varname=varname))

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

    def transposeDoubleProduct(self, W, M):
        return Var(pl.TransposedDoubleProductVariable(varray=[self.v, W, M]))

    def square(self):
        return Var(pl.SquareVariable(input=self.v))

    def add(self, other):
        return Var(pl.PlusVariable(input1=self.v, input2=other.v))

    def classificationLoss(self, class_index):
        return Var(pl.ClassificationLossVariable(input1=self.v, input2=class_index.v))
    
    def __add__(self, other):
        return self.add(other)

    def __sub__(self, other):
        return Var(pl.MinusVariable(input1=self.v,input2=other.v))

    def neg(self):
        return Var(pl.NegateElementsVariable(input=self.v))

    def __neg__(self):
        return self.neg()
    

###################################################    
# RLayer stands for reconstruciton hidden layer


def addSigmoidTiedRLayer(input, iw, ow, add_bias=True, basename=""):
    """This assumes the input is a (1,iw) matrix,
    and will produce an output that will be a (1, ow) matrix.
    It will create parameter W(ow,iw)
    and an optional parameter b(1,ow)
    Then output = sigmoid(input.W^T + b)
    
    Returns a triple (hidden, reconstruciton_cost, reconstructed_input)"""
    W = Var(ow,iw,"uniform", -1./sqrt(iw), 1./sqrt(iw), varname=basename+'_W')
    
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

def addMultiSoftMaxDoubleProductTiedRLayer(input, iw, igs, ow, ogs, add_bias=False, constrain_mask=False, basename=""):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""
    M = Var(ow/ogs, iw, "uniform", -1./iw, 1./iw, False, varname=basename+"_M")
    if constrain_mask:
        M = M.sigmoid()
    W = Var(ogs, iw, "uniform", -1./iw, 1./iw, False, varname=basename+"_W")
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

def addMultiSoftMaxDoubleProductRLayer(input, iw, igs, ow, ogs, basename=""):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""
    M = Var(ow/ogs, iw, "uniform", -1./iw, 1./iw, False, varname=basename+"_M")
    W = Var(ogs, iw, "uniform", -1./iw, 1./iw, False, varname=basename+"_W")
    hidden = input.doubleProduct(W,M).multiSoftMax(ogs)
    Mr = Var(iw/igs, ow, "uniform", -1./ow, 1./ow, False, varname=basename+"_Mr")
    Wr = Var(igs, ow, "uniform", -1./ow, 1./ow, False, varname=basename+"_Wr")
    # TODO: a repenser s'il faut un transpose ou non
    log_reconstructed = hidden.doubleProduct(Wr,Mr).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxSimpleProductTiedRLayer(input, iw, igs, ow, ogs, basename=""):
    W = Var(ow, iw, "uniform", -1./iw, 1./iw, varname=basename+'_W')
    hidden = input.matrixProduct_A_Bt(W).multiSoftMax(ogs)
    log_reconstructed = hidden.matrixProduct(W).multiLogSoftMax(igs)
    reconstructed_input = log_reconstructed.exp()
    cost = log_reconstructed.dot(input).neg()
    return hidden, cost, reconstructed_input

def addMultiSoftMaxSimpleProductRLayer(input, iw, igs, ow, ogs):
    W = Var(ow, iw, "uniform", -1./iw, 1./iw, varname=basename+'_W')
    hidden = input.matrixProduct_A_Bt(W).multiSoftMax(ogs)
    Wr = Var(ow, iw, "uniform", -1./ow, 1./ow, varname=basename+'_Wr')
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
            

