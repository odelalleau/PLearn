#!/usr/bin/env python

import sys
from numpy import *
from numpy.linalg import *
from plearn.vmat.PMat import load_pmat_as_array
from plearn.plotting.netplot import showRowsAsImages

#server_command = 'plearn_exp server'
#serv = launch_plearn_server(command = server_command)

def computeAndShowFilters(datapmatfile, img_height, img_width, filtertype='PCA', lambd=1e-6, nu=0, centered=True, displaytranspose=False):
    """
    Input is considered to be the first img_height x img_width columns of datapmatfile.
    centered indicates whether we compte centered covariance (default) or uncentered covariance.
    Covariance matrix will get lambd*I added to its diagonal, and its off-diagonal terms multiplied by (1-nu).    
    Filtertype can be 'PCA' or 'denoising' or 'denoising_eig'.

    Original version of linear denoising with zeroing noise (with probability of zeroing equal to nu)
    is obtained for centered=False and lambda=0 
    """
    data = load_pmat_as_array(datapmatfile)
    inputs = data[:,0:img_height*img_width]
    C = mycov(inputs, centered)
    if(filtertype=="PCA"):
        filters = computePCAFiltersFromCovariance(C, lambd, nu)
    elif(filtertype=="denoising"):
        filters = computeDenoisingFiltersFromCovariance(C, lambd, nu)
    elif(filtertype=="denoising_eig"):
        filters = computeDenoisingEigenFiltersFromCovariance(C, lambd, nu)
    else:
        raise ValueError("Invalid filtertype "+filtertype)
    if displaytranspose:
        filters = filters.T
    showRowsAsImages(filters, img_height, img_width, figtitle="Filters")

def mycov(inputs, centered=True):
    if centered:
        C = cov(inputs, rowvar=0, bias=1)
    else:
        C = dot(inputs.T, inputs)
        C *= 1.0/len(inputs)        
    return C

def computePCAFiltersFromCovariance(C, lambd=1e-6, nu=0):
    C = C+diag(len(C)*[lambd])
    Cd = C.diagonal()
    C2 = C*(1.0-nu)
    # copy back intial diagonal
    for i in range(len(Cd)):
        C2[i,i] = Cd[i]
    eigvals, eigvecs = eig(C2)
    return real(eigvecs.T)

def computeDenoisingFiltersFromCovariance(C, lambd=1e-6, nu=0.10):
    C = C+diag(len(C)*[lambd])
    Cd = C.diagonal()
    C2 = C*(1.0-nu)
    # copy back intial diagonal
    for i in range(len(Cd)):
        C2[i,i] = Cd[i]
    WW = dot(inv(C2),C)
    return WW.T

def computeDenoisingEigenFiltersFromCovariance(C, lambd=1e-6, nu=0.10):
    WW = computeDenoisingFiltersFromCovariance(C, lambd, nu).T
    eigvals, eigvecs = eig(WW)
    return real(eigvecs.T)
    # return real(inv(eigvecs).T)

####################
### main program ###

if __name__ == "__main__":
    from plearn.utilities.autoscript import autoscript
    autoscript(computeAndShowFilters, True)
    
