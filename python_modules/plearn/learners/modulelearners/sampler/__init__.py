from pygame import *
import math
import sys

EXITCODE = -2
NEXTCODE = -1

def pause():
   c = sys.stdin.readline()
   if c.strip() == 'q' or  c.strip() == 'x' or c.strip() == 'Q' or c.strip() == 'X':
      return EXITCODE
#      raise SystemExit
   if c.strip() == 'n' :
      return NEXTCODE
   try: return int(c.strip())
   except: return 0

def init_screen(Nim,zoom_factor):
    init()
    width = int(math.sqrt(Nim*1.0))
    if width**2 != Nim:
       width = int(math.sqrt(Nim*1.0))+1
#       raise TypeError, "This code only deals with square images\n(and image size "+str(Nim)+" is not the square of an integer)"
    width *= zoom_factor
    return display.set_mode([width, width])

def draw_image(values_in_01,screen,zoom_factor):
    """ Draw a 2D image where the gray level corresponds to a value scaled in [0,1]
        (a warning is given when at least one of the value does not lie in the interval)
        - values_in_01 : list of values in [0,1]
        - screen  : output of init_screen()
	- zoom_factor : int > 0
    """
    GiveWarning=True
    
    Nim=len(values_in_01)
    width = int(math.sqrt(Nim*1.0))
    if width**2 != Nim:
       width += 1
#       raise TypeError, "This code only deals with square images\n(and image size "+str(Nim)+" is not the square of an integer)"
    width *= zoom_factor
    surface = Surface((width, width),0,8)
    surface.set_palette([(i,i,i) for i in range(2**8)])
    for x in range(width/zoom_factor):
       for y in range(width/zoom_factor):
           value = values_in_01[x*width/zoom_factor+y]
	   if value < 0. or value > 1.:
	      if GiveWarning:
	         GiveWarning=False
	         print "Warning: In draw image : value "+str(value)+" is not in [0,1]"
	      value = min(max(0.,value),1.)
           graycol = int(255.0*value)
           for i in range(zoom_factor):
               for j in range(zoom_factor):
                   surface.set_at((x*zoom_factor+i,y*zoom_factor+j),(graycol,graycol,graycol,255))
    screen.blit(surface, (0,0))
    display.update()
    return pause()

#
# Weights will be scaled in [0...255] such that:
# - the middle gray (127) corresponds to 0
# - a same variation in the gray level (negative, or positive)
#   correponds to a same variation
#
def draw_normalized_image(weights,screen,zoom_factor):
    """ Draw a 2D image where the gray level corresponds to a (scaled) weight
        ( gray represents a null weigth)
        - weights : list of real numbers, where 0 is a special case
        - screen  : output of init_screen()
	- zoom_factor : int > 0
    """

    MAX=max(weights)
    MIN=min(weights)
    MAX=2*max(MAX,-MIN)

    for i in range(len(weights)):
        weights[i] = weights[i]/MAX+0.5
    return draw_image(weights,screen,zoom_factor)

def max_matrix(array):
    print array[0]
    raise SystemExit
    MAX = max(array[0])
    for vec in array:
        maxtmp=max(vec)
        if maxtmp > MAX:
	   MAX = maxtmp
    return MAX
def min_matrix(array):
    MIN = min(array[0])
    for vec in array:
        mintmp=min(vec)
        if mintmp < MIN:
	   MIN = mintmp
    return MIN

