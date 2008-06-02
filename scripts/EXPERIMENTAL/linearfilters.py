#!/usr/bin/env python

import sys
from numpy import *
from numpy.linalg import *
from plearn.vmat.PMat import load_pmat_as_array
from plearn.plotting.netplot import showRowsAsImages

#server_command = 'plearn_exp server'
#serv = launch_plearn_server(command = server_command)

def computeAndShowFilters(datapmatfile, img_height, img_width, filtertype='PCA', lambd=1e-6, nu=0):
    """
    Input is considered to be the first img_height x img_width columns of datapmatfile.
    Filtertype can be 'PCA' or 'denoising'        
    Covariance matrix will get lambd*I added to its diagonal, and its off-diagonal terms multiplied by (1-nu).
    """
    data = load_pmat_as_array(datapmatfile)
    inputs = data[:,0:img_height*img_width]
    C = cov(inputs, rowvar=0, bias=1)
    if(filtertype=="PCA"):
        filters = computePCAFilters(C, lambd, nu)
    elif(filtertype=="denoising"):
        filters = computeDenoisingFilters(C, lambd, nu)
    else:
        raise ValueError("Invalid filtertype "+filtertype)    
    showRowsAsImages(filters, img_height, img_width, figtitle="Filters")

def mycov(inputs, centered=True):
    if centered:
        C = cov(inputs, rowvar=0, bias=1)
    else:
        C = dot(inputs.T, inputs)
        C *= 1.0/len(inputs)        
    return C

def computePCAFilters(C, lambd=1e-6, nu=0):
    C = C+diag(len(C)*[lambd])
    Cd = C.diagonal()
    C2 = C*(1.0-nu)
    # copy back intial diagonal
    for i in range(len(Cd)):
        C2[i,i] = Cd[i]
    eigvals, eigvecs = eig(C2)
    return real(eigvecs.T)

def computeDenoisingFilters(C, lambd=1e-6, nu=0.10):
    C = C+diag(len(C)*[lambd])
    Cd = C.diagonal()
    C2 = C*(1.0-nu)
    # copy back intial diagonal
    for i in range(len(Cd)):
        C2[i,i] = Cd[i]
    WW = dot(inv(C2),C)
    return WW


####################
### main program ###

if __name__ == "__main__":
    from plearn.utilities.autoscript import autoscript
    autoscript(computeAndShowFilters, True)
    
