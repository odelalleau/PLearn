#!/usr/bin/env python

import sys
import os
import os.path

from numpy import array, arange, diag, outer
import matplotlib.pylab as plt

# chose a non-GUI backend
# matplotlib.use( 'Agg' )

def plot_offdiag_histogram(m, xlabel="historgram of off-diagonal values", nbins=10):
    vals = []
    for i in xrange(m.shape[0]):
        for j in xrange(m.shape[1]):
            if i!=j:
                vals.append(m[i,j])
    n, bins, patches = plt.hist(array(vals), nbins, normed=1, facecolor='green', alpha=0.75)
    plt.xlabel(xlabel)
    plt.grid(True)
    
def plot_diag_histogram(m, xlabel="historgram of diagonal values", nbins=10):
    vals = diag(m)
    n, bins, patches = plt.hist(vals, nbins, normed=1, facecolor='green', alpha=0.75)
    plt.xlabel(xlabel)
    plt.grid(True)
    
def plot_histogram(vals, xlabel="historgram", nbins=10):
    n, bins, patches = plt.hist(array(vals), nbins, normed=1, facecolor='green', alpha=0.75)
    plt.xlabel(xlabel)
    plt.grid(True)

    
def display_vecstascollector_summary(stcol):
    n = stcol.getStat("N[0]")
    print n
    mean = stcol.getMean()
    stddev = stcol.getStdDev()
    ucov = (1.0/n)*stcol.getXtX()
    cov = stcol.getCovariance()
    corr = stcol.getCorrelation()
    pxy_px_py = ucov-outer(mean,mean)
    
    plt.subplot(4,2,1)
    plt.errorbar(arange(len(mean)),mean,yerr=stddev)
    plt.title("activations mean and stddev")
    plt.subplot(4,2,2)
    plot_histogram(mean, "activations mean")

    plt.subplot(4,2,3)
    plot_offdiag_histogram(ucov, "uncentered covariances")
    plt.subplot(4,2,4)
    plot_diag_histogram(ucov, "uncentered variances")

    plt.subplot(4,2,5)
    plot_offdiag_histogram(cov, "covariances")
    plt.subplot(4,2,6)
    plot_diag_histogram(cov, "variances")

    plt.subplot(4,2,7)
    plot_offdiag_histogram(corr, "correlations")
    plt.subplot(4,2,8)
    plot_histogram(stddev, "stddevs")

    plt.show()
    
    

################
### methods ###
################

def print_usage_and_exit():
    print "Usage : displaystatscol.py statscol.psave"
    print "  will graphically display some of the statistics contained in a saved stats collector."
    sys.exit()


############
### main ###
############

if len(sys.argv)<2:
    print_usage_and_exit()

import plearn.pyext.plext as pl

stfname = sys.argv[1]
stcol = pl.loadObject(stfname)

display_vecstascollector_summary(stcol)

