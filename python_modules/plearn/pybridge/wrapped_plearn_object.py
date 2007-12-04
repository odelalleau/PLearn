## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

#
# Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

global plearn_module
plearn_module= None

class WrappedPLearnObject(object):

    def __init__(self, **kwargs):
        #print 'WrappedPLearnObject.__init__',type(self),kwargs
        if '_cptr' in kwargs:
            self._cptr= kwargs['_cptr'] # ptr to c++ obj
        elif hasattr(self,'_cptr'):
            #print 'init->SETOPTIONS2',kwargs
            self.setOptions(kwargs)
            
    def setOptions(self, kwargs):
        #print 'SETOPTIONS',kwargs
        call_build= True
        for k in kwargs:
            if k=='__call_build':
                call_build= kwargs[k]
            elif k=='_cptr':
                pass
            else:
                self.__setattr__(k, kwargs[k])
        if call_build:
            self.build()
    
    def __setattr__(self, attr, val):
        if attr != '_optionnames' and attr in self._optionnames:
            self.setOptionFromPython(attr, val)
        else:
            object.__setattr__(self, attr, val)

    def __getattr__(self, attr):
        if attr in self._optionnames:
            return self.getOption(attr)
        else:
            raise AttributeError, ("no attribute "
                                   + attr + " in "
                                   + repr(self))
        
    def __del__(self):
        if hasattr(self, '_cptr'):
            self._unref()

    def __repr__(self):
        return self.asString() #PLearn repr. for now

    def __deepcopy__(self, memo= None):
        if not memo: memo= {}
        if 'PLearnCopiesMap' not in memo:
            memo['PLearnCopiesMap']= {}
        plnewone, memo['PLearnCopiesMap']= \
            self.pyDeepCopy(memo['PLearnCopiesMap'])
        newone= self.__class__(_cptr= plnewone._cptr)
        memo[id(self)]= newone
        del plnewone._cptr
        newone._refCPPObj(newone, False)
        for k in self.__dict__:
            if k != '_cptr':
                newone.__dict__[k]= \
                    copy.deepcopy(self.__dict__[k], memo)
        return newone

    def __getstate__(self):
        d= self.__dict__.copy()
        d['_cptr']= self.asString()
        return d
    
    def __setstate__(self, dict):
        newone= plearn_module.newObject(dict['_cptr'])
        self._cptr= newone._cptr
        self._refCPPObj(self, False)
        for k in dict:
            if k != '_cptr':
                self.__setattr__(k, dict[k])
        return dict


from numpy.numarray import *

class WrappedPLearnVMat(WrappedPLearnObject):
    def __init__(self, cptr):
        WrappedPLearnObject.__init__(self, cptr)

    def __len__(self):
        return self.length

    def __getitem__(self, key):
        l= len(self)
        if isinstance(key,int):
            if key < 0: key+= l
            if key < 0 or key >= l:
                raise IndexError
            return self.getRow(key)
        if isinstance(key,slice):
            start= key.start or 0
            stop= key.stop or l-1
            step= key.step or 1
            if start < 0: start+= l
            if stop < 0: stop+= l
            if step==1:
                return self.subMat(start, 0, stop-start, self.width)
            #return pl.MemoryVMatrix(data= [self[i] for i in xrange(start, stop, step)])
            raise NotImplementedError, 'slice step != 1'
        raise TypeError, "key should be an int or a slice"
    
