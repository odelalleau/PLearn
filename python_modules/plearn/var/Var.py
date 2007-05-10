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


class Var:

    def __init__(self, plvar):
        self.v = plvar

    def sigmoid(self):
        return Var(pl.SigmoidVariable(input=self.v))


def reconstructionCost(hidden, input, W):
    return hidden.matrixTransposeProduct(W).dot(input).neg()


# RLayer stands for reconstruciton hidden layer

def addSigmoidRLayer(input, nh):
    """Returns a triple (hidden, reconstruciton_cost, params)"""
    ni = input.size()
    W = Var(ni,nh)
    hidden = input.matrixProduct(W).sigmoid()
    cost = -hidden.matrixTransposeProduct(W).log_sigmoid().dot(input)
    return (hidden, cost, W)

def addMultiSoftmaxRLayer(input, ing, igs, ong, ogs, tighed=true, use_double_product=false):
    """ing is the input's number of groups
    igs is the input's group size
    ong is the output's number of groups
    ogs is the output's group size
    If tighed is true we will use the transpose of the recognition weight matrix for reconstruction
    If use_double_product is false we'll use a regular weight matrix.
    If it's true, we'll use the decomposition as a 
    Returns a triple (hidden, params, reonstruction_cost)"""
    if not use_double_product:
        W = Var(ing*igs,ong*ogs)
        hidden = input.matrixProduct(W).multiSoftMax(ong, ogs)
        if tighed:
            cost = -hidden.matrixTransposeProduct(W).multiLogSoftMax(ing,igs).dot(input)
            return hidden, cost, W
        else:
            Wr = Var(ong*ogs, ing*igs)
            cost = -hidden.matrixProduct(Wr).multiLogSoftMax(ing,igs)
            return hidden, cost, (W, Wr)

    else: # use double product
        M = Var(ing*igs, ong)
        W = Var(ing*igs, ogs)
        hidden = input.doubleProduct(W,M).multiSoftMax(ong, ogs)
        if tighed:
            cost = -hidden.transposeDoubleProduct(W,M).multiLogSoftMax(ing,igs).dot(input)
            return hidden, cost, (W,M)
        else:
            Mr = Var()
            Wr = Var()
n1=100, g1=4
n2=50, g2=8
n3=10, g3=25
n4=1,  g4=10

d=400
input = Var(1,d)
W1 = Var(n1*g1,d)
h1 = input.matrixProduct(W1)
W2 = Var(n2*g2,n1*g1)
h2 = h1.matrixProduct(W2)
W3 = Var(n3*g3,n2*g2)
h3 = h2.matrixProduct(W3)
W4 = Var(n4*g4,n3*g3)
h4 = h3.matrixProduct(W4)

xr = reconstructionCost(h1, x, W1)
h1r = reconstructionCost(h2, h1, W2)
h2r = reconstructionCost(h3, h2, W3)
h3r = reconstructionCost(h4, h3, W4)


parameters = [ Var() for ]
W1 = Var(10,25)
W2 = Var(3,2)




learner = pl.DeepAutoAssociatorLeanrer(
    
    )
