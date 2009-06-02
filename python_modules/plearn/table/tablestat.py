## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

"""
tablestat.py

Copyright (C) 2005 ApSTAT Technologies Inc.

This file was contributed by ApSTAT Technologies to the
PLearn library under the following BSD-style license: 

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

"""

# Author: Pascal Vincent

import numpy.numarray as numarray
import numpy
import bisect
import math
from numpy.numarray import *
from plearn.table.table import *

def smartdiv(a,b):
    try: return a/b
    except: return 'Error'

def smartpercent(a,b):
    try: return str(a*100./b)+' %'
    except: return 'Error'

def num2str(num):
    try:
        if isinstance(num,float) and math.floor(num)==num:
            return str(int(num))
        else:
            return str(num)
    except OverflowError:
        #raise OverflowError('in num2str for num '+repr(num))
        return repr(num)

## def interval_to_string(interval):
##     if isinstance(interval,str):
##         return interval
##     elif isinstance(interval,tuple):
##         include_low, low, high, include_high = interval
##         if include_low: repr = '[ '
##         else: repr = '] '
##         repr += num2str(low)+', '+num2str(high)
##         if include_high: repr += ' ]'
##         else: repr += ' ['
##         return repr
##     return num2str(interval)


class Interval:
    """Defines an interval between values low and high where include_low and include_high
    are booleans that indicate where the interval is closed or open.
    If low==high this is to be understood as a point value"""    

    def __init__(self, include_low, low, high, include_high):
        if low>high:
            raise ValueError('Interval must have low<=high')
        self.include_low = include_low
        self.low = low
        self.high = high
        self.include_high = include_high

    def __str__(self):
        if self.low==self.high:
            return num2str(self.low)
        else:
            return repr(self)

    def __repr__(self):
        if self.include_low: res = '[ '
        else: res = '] '
        res += num2str(self.low)+', '+num2str(self.high)
        if self.include_high: res += ' ]'
        else: res += ' ['
        return res

    def __contains__(self, item):
        if not isinstance(item,Interval): # assume item is a number 
            if self.include_low:
                res = item>=self.low
            else:
                res = item>self.low
            if self.include_high:
                res = res and (item<=self.high)
            else:                
                res = res and (item<self.high)
        else: # item is an Interval
            if self.include_low:
                res = item.low>=self.low
            else:
                res = (item.low>self.low) or (item.low==self.low and not item.include_low)
            if self.include_high:
                res = res and (item.high<=self.high)
            else:
                res = res and (item.high<self.high or (item.high==self.high and not item.include_high))
        return res

    def __add__(self, other):
        """union of self and other (result must be an interval, or exception is raised"""
        if self.low<=other.low:
            low = self.low
            if self.low==other.low:
                include_low = self.include_low or other.include_low
            else:
                include_low = self.include_low

            if self.high<other.low or (self.high==other.low and not (self.include_high or other.include_low)):
                raise ValueError('Union of these 2 intervals is not an interval')

            if self.high>other.high:
                high = self.high
                include_high = self.include_high
            elif other.high>self.high:
                high = other.high
                include_high = other.include_high
            else: # self.high==other.high
                high = self.high
                include_high = self.include_high or other.include_high
            return Interval(include_low, low, high, include_high)
        else:
            return other+self


class AutoDomain:

    def __init__(self, maxnstrings=25, maxnnumbers=25):

        # maximum number of strings recorded in strings_index
        # (actually strings_index might end up containing one or two extra
        # special strings: the empty string '' and the '/OTHER/' string.)
        self.maxnstrings = maxnstrings
        self.maxnnumbers = maxnnumbers
        
        # number of allocated indexes (the index method will return indexes witihn 0..nindexes_-1
        self.nindexes_ = 0

        # maps non float convertible strings to their index
        # It will include the empty string '' if it was encountered
        # It will include the '/OTHER/' string if we exceed maxnstrings registered strings
        self.strings_index = {}

        # sorted list of registered numbers 
        self.numbers = []
        # numbers_index[k] is the integer index of float value numbers[k]
        self.numbers_index = []
        # less_than_index[k] is the integer index of float values <numbers[k] and >number[k-1]
        # Note 1: less_than_index is one element longer than numbers (and numbers_index),
        # so that its last element is the index for values greater than the last of numbers.
        # (its first element is the index for values smaller than the first of numbers).
        # Note 1: less_than_index is empty as long as numbers has not been filled (len < maxnnumbers)
        # Then it is filled with -1, before recruiting indexes in it.
        self.less_than_index = []

        # min and max number values encountered (stays None if no numerical value is encountered).
        self.min_val = None
        self.max_val = None

    def nindexes(self):
        """Returns the total number of recorded indexes."""
        return self.nindexes_

    def index(self, x):
        """Will return the integer index associated with value x. If x does
        not map to an index already in the domain then the domain will
        be extended to include x and a corresponding new index will be
        associated to that extension and returned. """ 

        v = None
        
        try: v = float(x)
        except ValueError: pass

        if v is None or (isinstance(x,str) 
                         and len(x)>=2
                         and x[0]=='0' and x[1]!='.'): # handle it as a string
            s = str(x).strip()
            id = self.strings_index.get(s)
            #if not id: # unknown string
            if id == None: # unknown string
                
                if len(self.strings_index)<self.maxnstrings:
                    # we haven't reached maxnstrings so let's add this new one
                    id = self.nindexes_
                    self.strings_index[s] = id
                    self.nindexes_ += 1
                else: # we have reached maxnstrings
                    if s=='': # it's the empty string, add it all the same
                        id = self.nindexes_
                        self.strings_index[s] = id
                        self.nindexes_ += 1
                    else: # look for '/OTHER/' 
                        id = self.strings_index.get('/OTHER/')
                        if not id: # no /OTHER/ : add it
                            id = self.nindexes_
                            self.strings_index['/OTHER/'] = id
                            self.nindexes_ += 1                            

        else: # handle it as a float v
            pos = bisect.bisect_left(self.numbers, v)
            if pos < len(self.numbers) and self.numbers[pos]==v:
                # we have already registered that value
                id = self.numbers_index[pos]
            else: # this is not a registered value

                if self.min_val is None:
                    self.min_val = v
                    self.max_val = v
                else:
                    self.min_val = min(self.min_val, v)
                    self.max_val = max(self.max_val, v)

                if len(self.numbers)<self.maxnnumbers: # self.numbers not full: insert the new value
                    self.numbers.insert(pos,v)
                    id = self.nindexes_
                    self.numbers_index.insert(pos,id)
                    self.nindexes_ += 1
                    if len(self.numbers)==self.maxnnumbers: # we've filled self.numbers
                        if self.min_val<0. and self.max_val>0.:
                            # insert 0. if not already there
                            pos = bisect.bisect_left(self.numbers, 0.)
                            if self.numbers[pos]!=0.:
                                self.numbers.insert(pos,0)
                                self.numbers_index.insert(pos,self.nindexes_)
                                self.nindexes_ += 1
                        # fill the less_than_index with -1
                        self.less_than_index = [-1]*(len(self.numbers)+1)                        
                else: # self.numbers is full 
                    id = self.less_than_index[pos]
                    if id<0:
                        id = self.nindexes_
                        self.less_than_index[pos] = id
                        self.nindexes_ += 1

        return id

    def string_domain(self):
        """This will return an ordered list of (xstring, index) for the string part of the domain"""
        keys = self.strings_index.keys()
        keys.sort()
        dom = [ (key,self.strings_index[key]) for key in keys ]
        return dom
        
    def number_domain(self):
        """This will return an ordered list of (interval, index) for the numerical part of the domain
        interval is an instance of Interval 
        """
        dom = []
        if self.min_val and self.less_than_index and self.less_than_index[0]>=0:
            interval = Interval(True, self.min_val, self.numbers[0], False)
            dom.append( (interval, self.less_than_index[0]) )
        for k in xrange(len(self.numbers)):
            interval = Interval(True, self.numbers[k], self.numbers[k], True)
            dom.append((interval, self.numbers_index[k]))
            if self.less_than_index and self.less_than_index[k+1]>=0:
                if k<len(self.numbers)-1:
                    interval = Interval(False, self.numbers[k], self.numbers[k+1], False)
                    dom.append( (interval, self.less_than_index[k+1]) )
                elif self.max_val:
                    interval = Interval(False, self.numbers[k], self.max_val, True)
                    dom.append( (interval, self.less_than_index[k+1]) )
        return dom

    def domain(self):
        return self.string_domain()+self.number_domain()

    def domain_descr_id(self):
        do = self.domain()
        descriptions = []
        ids = []
        for range,id in do:            
            descriptions.append(str(range))
            ids.append(id)
        return descriptions,ids


class AutoCube:

    def __init__(self, nkeys, nvalues=1, typecode='f8'):
        self.nvalues = nvalues
        shape = [1]*nkeys
        if nvalues>0:
            shape.append(nvalues)
        self.data = zeros(shape, typecode)

    def enlarge(self,idx):
        if isinstance(idx,int):
            idx = [idx] # make it a list
        oldshape = self.data.shape
        newshape = list(oldshape)
        for k in xrange(len(idx)):
            newshape[k] = max(newshape[k], idx[k]+1)
        newdata = zeros(newshape, numarray.typefrom(self.data))
        slicespec = [ slice(0,dim) for dim in oldshape ]
        newdata[slicespec] = self.data
        self.data = newdata

    def __getitem__(self,idx):
        try:
            return self.data[idx]
        except IndexError:
            self.enlarge(idx)
        return self.data[idx]
            
    def __setitem__(self,idx,val):
        try:
            self.data[idx] = val
        except IndexError:
            self.enlarge(idx)
        self.data[idx] = val
        

def combinations(lists):    
    if not lists:
        yield []
    else:
        for i in lists[0]:
            for c in combinations(lists[1:]):
                yield [i]+c

def all_combinations(lists):
    return [ comb for comb in combinations(lists) ]
    

def domain_union( range_ids_a, range_ids_b ):
    ra,idsa = range_ids_a
    rb,idsb = range_ids_b
    



class BaseTableStats:

    def __init__(self, var_combinations, var_domains, valuenames=[], weightvar='', full_shuffle_stats=True):
        """
        var_combinations is a list of tuples of variable names
        var domains maps variable names to AutoDomain
        valuef is a function that takes a row and returns a tuple of values
        valuenames is a list of variable names for the tuples returned by valuef
        """
        self.var_combinations = var_combinations
        self.var_domains = var_domains
        self.valuenames = valuenames
        self.weightvar = weightvar
        # self.valuef = valuef
        self.cubes = {}
        self.nsamples = 0
        self.full_shuffle_stats = full_shuffle_stats
        
        for vars in var_combinations:
            self.cubes[tuple(vars)] = AutoCube(len(vars), 1+len(self.valuenames))

    def extended_value_names(self):
        if self.weightvar:
            return [self.weightvar]+[ self.weightvar+'*'+varname for varname in self.valuenames ]
        else:
            return ['_count_']+self.valuenames

##         countvar = self.weightvar
##         if not countvar:
##             countvar = '_count_'
##         valuenames = [countvar]+self.valuenames
##        return valuenames        

    def update(self, row):
        # values = self.valuef(row)
        values = array([1.]+[ float_or_zero(row[valuename]) for valuename in self.valuenames ])
        if self.weightvar:
            w = float_or_zero(row[self.weightvar])
            values *= w
        for vars,cube in self.cubes.items():
            indexes = tuple([ self.var_domains[varname].index(row[varname]) for varname in vars ])
            cube[indexes] += values
        self.nsamples += 1
        return self.nsamples


    def stringDomain(self, varname):
        s_domain = self.var_domains[varname].string_domain()
        return [ (s, [id] ) for s,id in s_domain ]

    def numberDomain(self, varname, condvar, summarize_min_prob = -1.):
        """Will return an ordered list of (interval, indexes) for the numerical part of the domain
        interval is an instance of Interval
        indexes is a list of corresponding cube-indexes for variable varname in a cube
        corresponding to varname, such that the union of the individual intervals associated with
        those indexes gives interval (thus the slices corresponding to those indexes must be summed
        to get the values corresponding to interval).
        Consecutive intervals will be merged until the largest conditional probability within them
        reaches summarize_min_prob
        """
        
        num_domain = self.var_domains[varname].number_domain()
        if not num_domain: # empty list
            return num_domain

        if summarize_min_prob<=0.:
            return [ (interval, [id] ) for interval,id in num_domain ]
        
        try:
            cube = self.cubes[(varname, condvar)]
        except KeyError:
            cube = self.cubes[(condvar, varname)]
            cube = transpose(cube,(1,0,2))

        num_ids = [ id for interval,id in num_domain ]
        # we consider only the "counts" (value index 0)
        counts = cube[:,:,0]
        # sum the counts 
        count_sums = abs(sum(counts, 0))+1e-6  # we add 1e-6 just to make sure we don't have zeros and divisions by zero
        condprobs = counts/count_sums
        # we keep only the numerical domain
        condprobs = take(condprobs,num_ids)
        l,w = condprobs.shape

        summarized_domain = []
        newinterval = None
        newrow = zeros(w, numarray.typefrom(condprobs))
        ids = []
        for i in xrange(l):
            row = condprobs[i]
            interval, id = num_domain[i]
            if not newinterval:
                newinterval = interval
            else:
                newinterval = Interval(newinterval.include_low, newinterval.low, interval.high, interval.include_high)
            ids.append(id)
            newrow += row
            if max(newrow)>=summarize_min_prob or i==l-1:
                summarized_domain.append( (newinterval, ids) )
                newinterval = None
                newrow[:] = 0.
                ids = []
        
        return summarized_domain

    def trimmedNumberDomain(self, varname, condvar, summarize_remove_n, summarize_min_prob):
        """Will return an ordered list of (interval, indexes) for the numerical part of the domain
        interval is an instance of Interval
        indexes is a list of corresponding cube-indexes for variable varname in a cube
        corresponding to varname, such that the union of the individual intervals associated with
        those indexes gives interval (thus the slices corresponding to those indexes must be summed
        to get the values corresponding to interval).
        summarize_remove_n is the number of small intervals to 'remove'
        In addition, all intervals whose prob is less than summarize_min_prob will also be 'removed'
        """

        num_domain = self.var_domains[varname].number_domain()
        summarized_domain = [ (interval, (id,) ) for interval,id in num_domain ]

        if (summarize_remove_n<=0 and summarize_min_prob>=1.) or len(summarized_domain)==0:
            return summarized_domain

        try:
            cube = self.cubes[(varname, condvar)]
        except KeyError:
            cube = self.cubes[(condvar, varname)]
            cube = transpose(cube,(1,0,2))

        num_ids = [ id for xrange,id in num_domain ] 
        # we consider only the "counts" (value index 0)
        counts = cube[:,:,0]
        # sum the counts 
        count_sums = abs(sum(counts, 0))+1e-6  # we add 1e-6 just to make sure we don't have zeros and divisions by zero
        condprobs = counts/count_sums
        # we keep only the numerical domain
        condprobs = take(condprobs,num_ids)

        l,w = condprobs.shape
        nremoved = 0

        while l>0:
            maxprobs = array([ max(p) for p in condprobs ])
            k = argmin(maxprobs)
            minprob = maxprobs[k]
            #print >>f, 'maxprobs: ',maxprobs
            #print >>f, 'k: ',k
            #print >>f, 'minprob: ',minprob
            if nremoved>=summarize_remove_n or minprob>=summarize_min_prob:
                break # exit while loop
            if k==0:
                k_a = 0
                k_b = 1
            elif k==l-1:
                k_a = l-2
                k_b = l-1
            elif maxprobs[k-1]<=maxprobs[k+1]:
                k_a = k-1
                k_b = k
            else:
                k_a = k
                k_b = k+1
            interval_a, ids_a = summarized_domain[k_a]
            interval_b, ids_b = summarized_domain[k_b]
            try:
                union_interval = interval_a + interval_b
            except ValueError: # union of the 2 intervals is not an interval!
                break
            summarized_domain[k_a] = (union_interval, ids_a+ids_b)
            del summarized_domain[k_b]
            #newcondprobs = numarray.typefrom(zeros((l-1,w),condprobs))
            newcondprobs = zeros((l-1,w),numarray.typefrom(condprobs))
            newcondprobs[0:k_b] += condprobs[0:k_b]
            newcondprobs[k_a:] += condprobs[k_b:]
            condprobs = newcondprobs
            l = l-1
            nremoved += 1

        return summarized_domain


    def getSumsMatrixAndNames(self, condvari, condvarj, summarize_min_prob=1e10):
        """ Returns the tuple (mat, rownames, colnames)"""
        try:
            cube = self.cubes[(condvari, condvarj)]
        except KeyError:
            cube = self.cubes[(condvarj, condvari)]
            cube = transpose(cube,(1,0,2))

        condi_domain = self.stringDomain(condvari)+self.numberDomain(condvari, condvarj, summarize_min_prob)
        condi_descr = [ str(interval) for interval,ids in condi_domain ]

        condj_descr, condj_id = self.var_domains[condvarj].domain_descr_id()

        values = cube[:,:,:]
        values = take(values,condj_id,axis=1)
        l, w, d = values.shape
        n = len(condi_domain)
        #mat = numarray.typefrom(zeros((1+n,1+w,d),values))
        mat = zeros((1+n,1+w,d),numarray.typefrom(values))
        firstrow = sum(values, 0)
        mat[0, 1:1+w, :] = firstrow

        mat[0, 0, :]   = sum(firstrow)
        for i in xrange(n):
            interval,ids = condi_domain[i]
            s = mat[1+i,0,:]
            for id in ids:
                row = values[id]                
                mat[1+i, 1:, :] += row
                s += sum(row)

        condi_descr.insert(0,'/*/')
        condj_descr.insert(0,'/*/')
        return mat, condi_descr, condj_descr

    def getCondTable(self, condvari, condvarj, sumvar='', divvar='', marginalize=0, summarize_min_prob=0., combinvarsmode=0):
        mat, rownames, colnames = self.getSumsMatrixAndNames(condvari, condvarj, summarize_min_prob)
        l,w,d = mat.shape
        valuenames = self.extended_value_names()
        #numarray.ufunc.Error.pushMode(dividebyzero="ignore", invalid="ignore")
        if sumvar and not divvar:
            m = mat[:,:,valuenames.index(sumvar)]
            title = 'SUM['+sumvar+'] | ' + condvari +', '+condvarj
        elif sumvar and divvar:
            if combinvarsmode==0:
                title = 'SUM['+sumvar+']/SUM['+divvar+'] | ' + condvari +', '+condvarj
                m = mat[:,:,valuenames.index(sumvar)]/mat[:,:,valuenames.index(divvar)]
            elif combinvarsmode==1:
                title = 'SUM['+sumvar+']-0.7*SUM['+divvar+'] | ' + condvari +', '+condvarj
                m = mat[:,:,valuenames.index(sumvar)]-0.7*mat[:,:,valuenames.index(divvar)]
        elif divvar and not sumvar:
            m = 1./mat[:,:,valuenames.index(divvar)]
            title = '1/SUM['+divvar+'] | ' + condvari +', '+condvarj
        else:
            raise ValueError('Must specify at least one of sumvar or divvar')

        if marginalize:
            if marginalize==1:
                m = m/m[0:1,:]
            elif marginalize==2:
                m = m/m[:,0:1]
            elif marginalize==3:
                m = m/m[0,0]
            m *= 100.0
            title = title + ' -> percents '+str(marginalize)

        if summarize_min_prob>0.:
            title = title + '   [prob>='+str(int(summarize_min_prob*10000)/100.0)+'%]'

        #numarray.ufunc.Error.popMode()
        table = MemoryTable(colnames, m)
        table.set_rownames(rownames)
        table.set_title(title)
        return table


##     def getCondProbTable(self, targetvar, condvar):
##         try:
##             cube = self.cubes[(targetvar, condvar)]
##         except KeyError:
##             cube = self.cubes[(condvar, targetvar)]
##             cube = transpose(cube,(1,0,2))
            
##         cond_descr, cond_id = self.var_domains[condvar].domain_descr_id()
##         targ_descr, targ_id = self.var_domains[targetvar].domain_descr_id()

##         prob = cube[:,:,0]
##         prob = take(take(prob,targ_id,axis=0),cond_id,axis=1)
##         prob_sum = sum(prob,0)
##         # prob = prob*100./sum(prob)

##         table = MemoryTable(cond_descr)
##         for pr in prob:
##             row = [ smartpercent(p,ps) for p,ps in zip(pr,prob_sum) ]
##             table.append(row)
##         table.set_rownames(targ_descr)
##         return table
        
##     def getCondValuesTable(self, condvar1, condvar2, strvaluef):
##         try:
##             cube = self.cubes[(condvar1, condvar2)]
##         except KeyError:
##             cube = self.cubes[(condvar2, condvar1)]
##             cube = transpose(cube,(1,0,2))
            
##         cond1_descr, cond1_id = self.var_domains[condvar1].domain_descr_id()
##         cond2_descr, cond2_id = self.var_domains[condvar2].domain_descr_id()

##         sumt = take(take(cube,cond1_id,axis=0),cond2_id,axis=1)

##         table = MemoryTable(cond2_descr)
##         table.set_rownames(cond1_descr)
##         for i in xrange(len(cond1_descr)):
##             row = []
##             for j in xrange(len(cond2_descr)):
##                 row.append(strvaluef(sumt[i,j]))
##             table.append(row)
##         return table
            
##     def getCondSumsTable(self, condvari, condvarj):
##         try:
##             cube = self.cubes[(condvari, condvarj)]
##         except KeyError:
##             cube = self.cubes[(condvarj, condvari)]
##             cube = transpose(cube,(1,0,2))
            
##         condi_descr, condi_id = self.var_domains[condvari].domain_descr_id()
##         condj_descr, condj_id = self.var_domains[condvarj].domain_descr_id()

##         sumt = cube[:,:,:]
##         sumt = take(take(sumt,condi_id,axis=0),condj_id,axis=1)

##         table = MemoryTable(['sum_var']+condj_descr)
##         rownames = []
##         valuenames = self.extended_value_names()
##         for i in xrange(len(condi_descr)):
##             condi_str = condi_descr[i]
##             for k in xrange(len(valuenames)):
##                 row = [ valuenames[k] ]
##                 for j in xrange(len(condj_descr)):
##                     row.append(sumt[i,j,k])
##                 table.append(row)
##                 rownames.append(condi_str)
##                 condi_str = ''
##         table.set_rownames(rownames)
##         return table
            
##     def getCondRatioTable(self, condvari, condvarj, divide_by):
##         """divide_by must be the valuename by which to divide"""        
##         try:
##             cube = self.cubes[(condvari, condvarj)]
##         except KeyError:
##             cube = self.cubes[(condvarj, condvari)]
##             cube = transpose(cube,(1,0,2))
            
##         condi_descr, condi_id = self.var_domains[condvari].domain_descr_id()
##         condj_descr, condj_id = self.var_domains[condvarj].domain_descr_id()

##         sumt = cube[:,:,:]
##         sumt = take(take(sumt,condi_id,axis=0),condj_id,axis=1)

##         valuenames = self.extended_value_names()
##         divideidx = valuenames.index(divide_by)
##         table = MemoryTable(['sum_var']+condj_descr)
##         rownames = []
##         for i in xrange(len(condi_descr)):
##             condi_str = condi_descr[i]
##             for k in xrange(len(valuenames)):
##                 row = [ valuenames[k] ]
##                 for j in xrange(len(condj_descr)):
##                     row.append(smartdiv(sumt[i,j,k],sumt[i,j,divideidx]))
##                 table.append(row)
##                 rownames.append(condi_str)
##                 condi_str = ''
##         table.set_rownames(rownames)
##         return table
            

