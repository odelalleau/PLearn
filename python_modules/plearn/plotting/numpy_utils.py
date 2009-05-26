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
import numpy
try:
    import MA
except ImportError:
    print "WARNING: Import of MA (masked array) failed. Skipping this import." 
import numpy.numarray as numarray
from numpy.numarray import *

threshold = 0

def default_is_missing(x):
    return x is None or x=='-' or x=='nan' or (type(x) is str and x.strip()=='')
    
def to_numpy_float_array(values_list,
                         missing_value = ValueError("Found value interpreted as missing value and no missing_value was specified"),
                         mapping = ValueError("Could not convert value to float and no mapping was specified"),
                         mapping_start = 1.0,
                         is_missing = default_is_missing ):
    """
    Transforms a list of values into a numpy array of floats.
    Values that are not floats are handled automatically, according to the following policies:    

      * missing_value specifies what to do if we encounter a value that we consider a missing value:
        If missing_value is a float, then its value is used
        If missing_value is None, then we'll return an apporpriately masked array (MA.array)
        If missing_value is not specified as a float nor None, it will get raised as an exception.        
      Note that a value x is considered a missing value if it satisfies is_missing(x)
      (defaults to None or blank string or single dash '-').
      
      * If the value x is not missing, an attempt will be made to convert it to float using float(x).      
        This has the effect of converting strings representing float to that float,
        and of converting a bool to 1. or 0.

      * If the value is the string 'True' or 'False' it will similarly be changed to 1. or 0.      

      * If all the above fails, mapping can be used to specify how to automatically handle the case:
          If mapping is a float, then its value will be used.
          If mapping is a dictionary, then it will be looked up to find the corresponding value to use
          and if it is not found, then new corresponding mapping will automatically be added
          starting at mapping_start (which will get incremented ny 1).
          If you don't want automatical adding of mappings you can specify mapping_start = None
          (in this case an exception will be raised if not present in the mapping)
          If mapping is not specified as a float or a dict it will get raised as an exception.B 
      """
    if type(missing_value) is int:
        missing_value = float(missing_value)
    if type(mapping) is int:
        mapping = float(mapping)

    vec = []
    missing_pos = []
    for i in xrange(len(values_list)):
        val = values_list[i]
        if is_missing(val):
            if type(missing_value) is float:
                val = missing_value
            elif missing_value is None:
                val = 0.
                missing_pos.append(1)
            else:
                raise missing_value
        else:
            try:
                val = float(val)
            except ValueError:
                if val=='True':
                    val = 1.
                elif val=='False':
                    val = 0.
                elif type(mapping) is float:
                    val = mapping
                elif type(mapping) is dict:
                    if val in mapping:
                        val = mapping[val]
                    elif mapping_start is None:
                        raise ValueError("At position "+str(i)+" value "+str(val)+" not present in specified mapping.")
                    else:
                        mapping[val] = mapping_start
                        val = mapping_start
                        mapping_start += 1.
                else: # mapping is neither float nor dict:
                    raise mapping
        # Now we have a val that should be a valid float
        vec.append(val)
            
    # now return a proper numpy.array or MA.array (if we wanted masking).
    if len(missing_pos)==0: # return numpy.array
        return numpy.array(vec)
    else: # return masked array
        n = len(vec)
        mask = [0]*n
        for pos in missing_pos:
            mask[pos] = 1
        return MA.array(vec, mask=mask)

def mapping_of_values_to_pos(array_or_list):
    """Returns a dictionary mapping values present in array_or_list to a vector of their positions in the array_or_list."""
    d = {}    
    for i in xrange(len(array_or_list)):
        pos = d.setdefault(array_or_list[i], [])
        pos.append(i)
    # convert the positions lists into numpy arrays
    for key in d:
        d[key] = numpy.array(d[key])
    return d

def margin(scorevec):
    if len(scorevec)==1:
        return abs(scorevec[0]-threshold)
    else:
        sscores = sort(scorevec)    
        return sscores[-1]-sscores[-2]

def winner(scorevec):
    if len(scorevec)==1:
        if scorevec[0]>threshold:
            return 1
        else:
            return 0
    else:
        return argmax(scorevec)

def xyscores_to_winner_and_magnitude(xyscores):
    return array([ (v[0], v[1], winner(v[2:]),max(v[2:])) for v in xyscores ])

def xyscores_to_winner_and_margin(xyscores):
    return array([ (v[0], v[1], winner(v[2:]),margin(v[2:])) for v in xyscores ])

def regular_xyval_to_2d_grid_values(xyval):
    """Returns (grid_values, x0, y0, deltax, deltay)"""
    xyval = numarray.array(xyval)
    n = len(xyval)
    x = xyval[:,0]
    y = xyval[:,1]
    values = xyval[:,2:].copy()
    # print "type(values)",type(values)
    valsize = numarray.size(values,1)
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
        # print "A type(values)",type(values)
        values = numarray.transpose(values,(1,0,2))
        # print "B type(values)",type(values)
    elif y[1]==y0:
        deltax = x[1]-x[0]
        while y[k]==y0:
            k = k+1
        deltay = y[k]-y0
        nx = k
        ny = n // nx
        # print 'B) nx,ny:',nx,ny
        values.shape = (ny,nx,valsize)
        # print "C type(values)",type(values)
        values = numarray.transpose(values,(1,0,2))
        # print "D type(values)",type(values)
    else:
        raise ValueError("Strange: x[1]!=x0 and y[1]!=y0 this doesn't look like a regular grid...")

    print 'In regular_xyval_to_2d_grid_values: ', type(xyval), type(values)
    return values, x0, y0, deltax, deltay


def divide_by_mean_magnitude(xymagnitude):
    mag = xymagnitude[:,2]
    meanval = mag.mean()
    mag *= 1./meanval
    return meanval

def divide_by_max_magnitude(xymagnitude):
    mag = xymagnitude[:,2]
    maxval = mag.max()
    mag *= 1./maxval
    return maxval

def transform_magnitude_into_covered_percentage(xymagnitude):
    magnitudes = []
    l = len(xymagnitude)
    for i in xrange(l):
        row = xymagnitude[i]
        magnitudes.append([row[-1],i])
    magnitudes.sort()
    # magnitude.reverse()
    cum = 0
    for row in magnitudes:
        mag, i = row
        cum += mag
        row[0] = cum
    for mag,i in magnitudes:
        xymagnitude[i][-1] = mag/cum
    return cum
        
def classcolor(winner,margin=0):
    colors = { 0: [0.5, 0.5, 1.0],
               1: [1.0, 0.5, 0.5],
               2: [0.5, 1.0, 0.5],
             }
    return colors[winner]
    
def xy_winner_magnitude_to_xyrgb(xy_winner_margin):
    res = []
    for x,y,w,m in xy_winner_margin:
        res.append([x,y]+classcolor(w,m))
    return res

def xymagnitude_to_x_y_grid(regular_xymagnitude):
    gridvalues, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    nx = numarray.size(gridvalues,0)
    ny = numarray.size(gridvalues,1)
    gridvalues = numarray.reshape(gridvalues,(nx,ny))
    x = numarray.arange(x0,x0+nx*deltax-1e-6,deltax)
    y = numarray.arange(y0,y0+ny*deltay-1e-6,deltay)
    # print "x = ",x
    # print "y = ",y
    # print "z = ",gridvalues
    # print "type(x) = ",type(x)
    # print "type(y) = ",type(y)
    # print "type(z) = ",type(gridvalues)
    # imv.view(gridvalues)
    return x, y, gridvalues
    

def main():
    pass

if __name__ == "__main__":
    main()


