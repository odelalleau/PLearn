# from array import *

import matplotlib
matplotlib.interactive(True)
matplotlib.use('TkAgg')
  
from matplotlib.matlab import *

def scores_to_winner_level(scores):
    
    pass
    
def scores_to_winner_margin(scores):
    pass

# def generate_2D_color_plot(x_y_color):

# def plot_2D_decision_surface(training_points


x = arange(7)
y = arange(5)
X, Y = meshgrid(x,y)
Z = rand( len(x), len(y))
pcolor_classic(X, Y, transpose(Z))
show()
print 'pcolor'

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
    clf()
    imshow(Z, interpolation=interpol)
    # show()
    draw()
    print interpol
    
show()



#t = arange(0.0, 5.2, 0.2)

# red dashes, blue squares and green triangles
#plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')
#show()

