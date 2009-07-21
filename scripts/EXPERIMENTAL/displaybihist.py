#!/usr/bin/env python

import sys
import os
import os.path
import math

import matplotlib.pylab as mpl

from numpy import array, arange, diag, outer

from plearn.vmat.PMat import load_pmat_as_array
from plearn.plotting.netplot import plotRowsAsImages

class showBihistRows:

    def __init__(self, X, 
                 img_height,
                 img_width,
                 nrows = 10, ncols = 20,
                 startidx = 0,
                 figtitle="",
                 luminance_scale_mode=0,
                 colormaps = [mpl.cm.jet, mpl.cm.gray],
                 vmin = None, vmax = None,
                 transpose_img=False):

        self.X = X
        self.img_height = img_height
        self.img_width = img_width
        self.nrows = nrows
        self.ncols = ncols
        self.figtitle = figtitle
        self.startidx = startidx

        # appearance control
        self.luminance_scale_mode = luminance_scale_mode
        self.interpolation = 'nearest'
        self.colormaps = colormaps
        self.cmapchoice = 0
        self.show_colorbar = True
        self.disable_ticks = True
        self.vmin = vmin
        self.vmax = vmax
        self.transpose_img = transpose_img

        # plot it
        self.draw()      
        mpl.connect('key_press_event', self.keyPressed)
        # connect('button_press_event', self.__clicked)

        # start interactive loop
        mpl.show()

    def draw(self):
        # print "Start plotting..."
        mpl.clf()
        endidx = min(self.startidx+self.nrows*self.ncols, len(self.X))        
        title = self.figtitle+" ("+str(self.startidx)+" ... "+str(endidx-1)+")"
        plotRowsAsImages(self.X[self.startidx : endidx],
                         img_height = self.img_height,
                         img_width = self.img_width,
                         nrows = self.nrows,
                         ncols = self.ncols,
                         figtitle = title,
                         luminance_scale_mode = self.luminance_scale_mode,
                         show_colorbar = self.show_colorbar,
                         disable_ticks = self.disable_ticks,
                         colormap = self.colormaps[self.cmapchoice],
                         vmin = self.vmin,
                         vmax = self.vmax,
                         transpose_img = self.transpose_img
                         )
        # print "Plotted,"
        mpl.draw()
        # print "Drawn."
        

    def plotNext(self):
        self.startidx += self.nrows*self.ncols
        if self.startidx >= len(self.X):
            self.startidx = 0
        self.draw()

    def plotPrev(self):
        if self.startidx>0:
            self.startidx -= self.nrows*self.ncols
        else:
            self.startidx = len(self.X)-self.nrows*self.ncols
        if self.startidx<0:
            self.startidx = 0            
        self.draw()

    def keyPressed(self, event):
        char = event.key
        print 'Pressed',char
        if char == 'c':
            self.changeColorMap()
        elif char == 'right':
            self.plotNext()
        elif char == 'left':
            self.plotPrev()
        elif char == 'b':
            self.show_colorbar = not self.show_colorbar
            self.draw()
        elif char == 't':
            self.transpose_img = not self.transpose_img
            self.draw()
        elif char == 'i':
            self.disable_ticks = not self.disable_ticks
            self.draw()
        elif char == 's':
            self.luminance_scale_mode = (self.luminance_scale_mode+1)%3
            self.draw()
        elif char == '':
            pass
        else:
            print """
            *******************************************************
            * KEYS
            *  right : show next filters
            *  left  : show previous filters
            *  t     : tranpose images
            *  c     : change colormap
            *  s     : cycle through luminance scale mode
            *          0 independent luminance scaling for each
            *          1 min-max luminance scaling across display
            *          2 +-min or +- max (largest range)
            *  b     : toggle showing colorbar 
            *  i     : toggle showing ticks
            *
            * Close window to stop.
            *******************************************************
            """

    def changeColorMap(self):
        self.cmapchoice = (self.cmapchoice+1)%len(self.colormaps)
        self.draw()

    
def display_bihists(m):
    nbins = int(math.sqrt(m.shape[1]))
    nvars = int(math.sqrt(m.shape[0]))
    
    m2 = m[:,:]
    hists = [ diag(m[i*nvars+i].reshape(nbins,nbins)) for i in xrange(nvars) ] 
    for i in xrange(nvars):        
        for j in xrange(nvars):
            bihist = m2[i*nvars+j].reshape(nbins,nbins)
            if i==j:
                bihist.fill(1.)
            else:
                bihist[:,:] = bihist-outer(hists[i],hists[j])
                
    showBihistRows(m2, nbins, nbins,
                     nrows = 10, ncols = 20,
                     startidx = 0,
                     figtitle="",
                     luminance_scale_mode=1,
                     vmin = -1., vmax = 1.,
                     transpose_img=False)

def print_usage_and_exit():
    print "Usage : displaybihist.py bihists.pmat"
    print "  will graphically display the bivariate histograms in the pmat."
    sys.exit()


############
### main ###
############

if len(sys.argv)<2:
    print_usage_and_exit()

pmatfname = sys.argv[1]
m = load_pmat_as_array(pmatfname)

display_bihists(m)
