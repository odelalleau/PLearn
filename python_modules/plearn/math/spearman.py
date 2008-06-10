# spearman.py
# Copyright (C) 2008 Xavier Saint-Mleux, ApSTAT Technologies inc.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Xavier Saint-Mleux

import math

def spearmans_rho(xs, ys):
    """
    Calculate Spearman's rank correlation coefficient on two
    lists or arrays.
    """
    n= len(xs)
    assert(n==len(ys))

    # sort xs and ys while keeping original indices
    def add_idx_and_sort(l):
        r= zip(l, range(len(l)))
        r.sort()
        return r
    xs2= add_idx_and_sort(xs)
    ys2= add_idx_and_sort(ys)

    # assign ranks, averaging if necessary
    def assign_ranks(li):
        n= len(li)
        r= []# list of (idx, val, rank) to be built
        k= 0
        while k < n:
            k0= k
            v= li[k0][0]
            while k < n and li[k][0]==v:
                k+= 1
            rank= 1.+float(k0+k-1)/2.
            for i in range(k0,k):
                r+= [(li[i][1], li[i][0], rank)]
        r.sort()#re-sort in orig. order
        return r
    xs3= assign_ranks(xs2)
    ys3= assign_ranks(ys2)

    # calc. correlation
    sumx= 0.
    sumy= 0.
    sumxy= 0.
    sumx2= 0.
    sumy2= 0.
    ds= []
    ds2= []
    for i in range(n):
        x= xs3[i][2]
        y= ys3[i][2]
        ds+= [x-y]
        ds2+= [(x-y)**2]
        sumx+= x
        sumy+= y
        sumxy+= x*y
        sumx2+= x*x
        sumy2+= y*y
    rho= (n*sumxy - sumx*sumy) / (math.sqrt(n*sumx2 - sumx*sumx) * math.sqrt(n*sumy2 - sumy*sumy))
    return rho


if __name__ == '__main__':
    #####
    # test example from Wikipedia (http://en.wikipedia.org/wiki/Spearman's_rank_correlation_coefficient)
    #(IQ - Xi, 	Hours of TV per week - Yi)
    xys= [(106,	7),
          (86,	0),
          (100,	27),
          (101,	50),
          (99,	28),
          (103,	29),
          (97,	20),
          (113,	12),
          (112,	6),
          (110,	17)]
    print spearmans_rho([xy[0] for xy in xys], [xy[1] for xy in xys])
    
