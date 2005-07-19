
from math import *

def fast_softmax( x ):
    s = 0
    res = []
    for r in x:
        e = exp(r)
        s += e
        res.append( e )

    return [ r/s for r in res ]

if __name__ == '__main__':
    print fast_softmax([ 0, 0, 100 ])
