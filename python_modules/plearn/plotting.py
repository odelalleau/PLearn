# from array import *

import matplotlib
# matplotlib.interactive(True)
# matplotlib.use('TkAgg')
#matplotlib.use('GTK')
  
from pylab import *

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

def xyscores_to_winner_magnitude(xyscores):
    return array([ (v[0], v[1], argmax(v[2:]),max(v[2:])) for v in scores ])

def xyscores_to_winner_margin(xyscores):
    return array([ (v[0], v[1], argmax(v[2:]),margin(v[2:])) for v in scores ])

def regular_xyval_to_2d_grid(xyval):
    """Returns (grid, x0, y0, deltax, deltay)"""
    n = len(xyval)
    x = xyval[:,0]
    y = xyval[:,1]
    values = xyval[:,2:]
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
        values.shape = (nx,ny,valsize)
    elif y[1]==y0:
        deltax = x[1]-x[0]
        while y[k]==y0:
            k = k+1
        deltay = y[k]-y0
        nx = k
        ny = n // nx
        values.shape = (ny,nx,valsize)
        values = transpose(values,(1,0,2))
    return values, x0, y0, deltax, deltay

def scores_to_winner_level(scores):
    
    pass
    
def scores_to_winner_margin(scores):
    pass

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

