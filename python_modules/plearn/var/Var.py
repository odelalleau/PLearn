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

    def sigmoid(self):
        return Var(pl.SigmoidVariable(input=self.v))

    def plearn_repr( self, indent_level=0, inner_repr=plearn_repr ):
        # asking for plearn_repr could be to send specification over
        # to another prg so that will open the .pmat
        # So we make sure data is flushed to disk.
        return self.v.plearn_repr(indent_level, inner_repr)

    def probabilityPairs(self, min, max):
        return Var(pl.ProbabilityPairsVariable(input=self.v, min=min, max=max))

    def matrixProduct(self, W):
        return Var(pl.ProductVariable(self.v, W))

    def matrixTransposeProduct(self, W):
        return Var(pl.TransposeProductVariable(self.v, W))

    def affineTransform(self, W):
        return Var(pl.AffineTransformVariable(self.v, W))

    def doubleProduct(self, W, M):
        return Var(pl.DoubleProductVariable(varray=[self.v, W, M]))

    def dot(self, input):
        return Var(pl.DotProductVariable(input1=self, input2=input))

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

    def __add__(self, other):
        return Var(pl.PlusVariable(input1=self.v, input2=other.v))

    def __sub__(self, other):
        return Var(pl.MinusVariable(input1=self.v,input2=other.v))

    def __neg__(self):
        return Var(pl.NegateElementsVariable(input=self.v))
    
    

    
# RLayer stands for reconstruciton hidden layer

def addSigmoidRLayer(input, iw, ow):
    """Returns a triple (hidden, reconstruciton_cost, params)"""
    W = Var(1+iw,ow)
    hidden = input.affineTransform(W).sigmoid()
    cost = -hidden.matrixTransposeProduct(W).log_sigmoid().dot(input)
    return (hidden, cost, W)




def addMultiSoftMaxDoubleProductTiedRLayer(input, iw, igs, ow, ogs, basename=""):
    """iw is the input's width
    igs is the input's group size
    ow and ogs analog but for output"""
    M = Var(ow/ogs, iw, "uniform", -1/iw, 1/iw, False, varname=basename+"_M")
    W = Var(ogs, iw, "uniform", -1/iw, 1/iw, False, varname=basename+"_W")
    hidden = input.doubleProduct(W,M).multiSoftMax(ogs)
    cost = -hidden.transposeDoubleProduct(W,M).multiLogSoftMax(igs).dot(input)            
    return hidden, cost, (W,M)

def addMultiSoftMaxDoubleProductNotTiedRLayer(input, iw, igs, ow, ogs):
    return 1,2,3

def addMultiSoftMaxSimpleProductTiedRLayer(input, iw, igs, ow, ogs):
    W = Var(iw, ow)
    hidden = input.matrixProduct(W).multiSoftMax(ogs)
    cost = -hidden.matrixTransposeProduct(W).multiLogSoftMax(igs).dot(input)
    return hidden, cost, W

def addMultiSoftMaxSimpleProductNotTiedRLayer(input, iw, igs, ow, ogs):
    W = Var(iw,ow)
    hidden = input.matrixProduct(W).multiSoftMax(ogs)
    Wr = Var(ow, iw)
    cost = -hidden.matrixProduct(Wr).multiLogSoftMax(igs)
    return hidden, cost, (W, Wr)






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
            

