# plotting.py
# Copyright (C) 2005 Pascal Vincent
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


# Author: Pascal Vincent

# from array import *

import string
import matplotlib
# matplotlib.interactive(True)
matplotlib.use('TkAgg')
#matplotlib.use('GTK')
  
from pylab import *
from mayavi.tools import imv

def load_pmat_as_array(fname):
    s = file(fname,'rb').read()
    formatstr = s[0:64]
    datastr = s[64:]
    structuretype, l, w, elemtype, endianness = string.split(formatstr)
    l = int(l)
    w = int(w)
    X = fromstring(datastr,'d')
    X.shape = (l,w)
    return X

def margin(scorevec):
    sscores = sort(scorevec)
    return sscores[-1]-sscores[-2]

def xyscores_to_winner_and_magnitude(xyscores):
    return array([ (v[0], v[1], argmax(v[2:]),max(v[2:])) for v in scores ])

def xyscores_to_winner_and_margin(xyscores):
    return array([ (v[0], v[1], argmax(v[2:]),margin(v[2:])) for v in scores ])

def regular_xyval_to_2d_grid_values(xyval):
    """Returns (grid_values, x0, y0, deltax, deltay)"""
    n = len(xyval)
    x = xyval[:,0]
    y = xyval[:,1]
    values = xyval[:,2:].copy()
    valsize = size(values,1)
    x0 = x[0]
    y0 = y[0]

    k = 1
    if x[1]==x0:
        deltay = y[1]-y[0]
        while x[k]==x0:
            k = k+1
        deltax = x[k]-x0
        ny = k
        nx = n // ny
        # print 'A) nx,ny:',nx,ny
        values.shape = (nx,ny,valsize)
    elif y[1]==y0:
        deltax = x[1]-x[0]
        while y[k]==y0:
            k = k+1
        deltay = y[k]-y0
        nx = k
        ny = n // nx
        # print 'B) nx,ny:',nx,ny
        values.shape = (ny,nx,valsize)
        values = transpose(values,(1,0,2))
    return values, x0, y0, deltax, deltay


def imshow_2d_grid_values(gridvalues, x0, y0, deltax, deltay, interpolation='nearest', cmap = cm.jet):
    nx = size(gridvalues,0)
    ny = size(gridvalues,1)
    extent = (x0-.5*deltax, x0+nx*deltax, y0-.5*deltay, y0+ny*deltay)
    gridvalues = reshape(gridvalues,(nx,ny))
    # print 'SHAPE:',gridvalues.shape
    # print 'gridvalues:', gridvalues
    imshow(gridvalues, cmap=cmap, origin='lower', extent=extent, interpolation=interpolation)

def plot_2d_points(pointlist, style):
    x, y = zip(*pointlist)
    plot(x, y, style)
    

def plot_2d_magnitude_and_points(regular_xymagnitude, pointlists, styles = ['bo','go','ro']):
    grid_values, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    imshow_2d_grid_values(grid_values, x0, y0, deltax, deltay, 'bicubic', cm.jet)

    for pointlist,style in zip(pointlists,styles[0:len(pointlists)]):
        plot_2d_points(pointlist, style)


def plot_surface(regular_xymagnitude):
    gridvalues, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    nx = size(gridvalues,0)
    ny = size(gridvalues,1)
    gridvalues = reshape(gridvalues,(nx,ny))
    x = arange(x0,x0+nx*deltax,deltax)
    y = arange(y0,y0+ny*deltay,deltay)    
    imv.surf(x, y, gridvalues)


# def generate_2D_color_plot(x_y_color):

# def plot_2D_decision_surface(training_points

def main():
    print "Still under development. Do not use!!!"
    extent = (1, 25, -5, 25)

    x = arange(7)
    y = arange(5)
    X, Y = meshgrid(x,y)
    Z = rand( len(x), len(y))
    # pcolor_classic(X, Y, transpose(Z))
    #show()
    #print 'pcolor'

    for interpol in ['bicubic',
                     'bilinear',
                     'blackman100',
                     'blackman256',
                     'blackman64',
                     'nearest',
                     'sinc144',
                     'sinc256',
                     'sinc64',
                     'spline16',
                     'spline36']:

        raw_input()
        print interpol
        clf()
        imshow(Z, cmap=cm.jet, origin='upper', extent=extent, interpolation=interpol)
        markers = [(15.9, 14.5), (16.8, 15), (20,20)]
        x,y = zip(*markers)
        plot(x, y, 'o')
        plot(rand(20)*25, rand(20)*25, 'o') 
        show()
        # draw()

    show()


if __name__ == "__main__":
    main()



#t = arange(0.0, 5.2, 0.2)

# red dashes, blue squares and green triangles
#plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')
#show()

