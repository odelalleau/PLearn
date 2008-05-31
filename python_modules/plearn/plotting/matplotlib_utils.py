## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

# matplotlib_utils.py
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
#from numpy.numarray import *

from plearn.plotting.numpy_utils import *
import matplotlib
# matplotlib.interactive(True)
#matplotlib.use('TkAgg')
#matplotlib.use('GTK')
from pylab import *


def imshow_xymagnitude(regular_xymagnitude, interpolation='nearest', cmap = cm.jet):
    grid_values, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    # print 'In imshow_xymagnitude: ', type(regular_xymagnitude), type(grid_values)
    imshow_2d_grid_values(grid_values, x0, y0, deltax, deltay, interpolation, cm.jet)
    
def imshow_xyrgb(regular_xyrgb, interpolation='nearest'):
    grid_rgb, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xyrgb)
    # print 'grid_rgb shape=',grid_rgb.shape
    imshow_2d_grid_rgb(grid_rgb, x0, y0, deltax, deltay, interpolation, cm.jet)
    
def contour_xymagnitude(regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    clabel(contour(x, y, gridvalues))
    
def contourf_xymagnitude(regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    contourf(x, y, gridvalues)
    colorbar()

def imshow_2d_grid_rgb(gridrgb, x0, y0, deltax, deltay, interpolation='nearest', cmap = cm.jet):
    nx = numarray.size(gridrgb,0)
    ny = numarray.size(gridrgb,1)
    extent = (x0-.5*deltax, x0+nx*deltax, y0-.5*deltay, y0+ny*deltay)
    # gridrgb = numarray.reshape(gridrgb,(nx,ny))
    # print 'SHAPE:',gridrgb.shape
    # print 'gridrgb:', gridrgb
    imshow(gridrgb, cmap=cmap, origin='lower', extent=extent, interpolation=interpolation)
    colorbar()

def imshow_2d_grid_values(gridvalues, x0, y0, deltax, deltay, interpolation='nearest', cmap = cm.jet):
    nx = numarray.size(gridvalues,0)
    ny = numarray.size(gridvalues,1)
    extent = (x0-.5*deltax, x0+nx*deltax, y0-.5*deltay, y0+ny*deltay)
    print 'gridval type', type(gridvalues)
    gridvalues = numarray.reshape(gridvalues,(nx,ny))
    # print 'SHAPE:',gridvalues.shape
    # print 'gridvalues:', gridvalues
    imshow(gridvalues, cmap=cmap, origin='lower', extent=extent, interpolation=interpolation)
    colorbar()

def plot_2d_points(pointlist, style='bo'):
    x, y = zip(*pointlist)
    plot(x, y, style)

def plot_2d_class_points(pointlist, styles):
    classnum = 0
    for style in styles:
        points_c = [ [x,y] for x,y,c in pointlist if c==classnum]
        if len(points_c)==0:
            break
        # print 'points',classnum,':',points_c
        plot_2d_points(points_c, style)
        classnum += 1


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

