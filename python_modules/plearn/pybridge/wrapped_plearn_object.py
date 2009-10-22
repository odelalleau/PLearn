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


import warnings

global plearn_module
plearn_module= None
def get_plearn_module():
    global plearn_module
    return plearn_module

# remote pickle: when true, __getstate__ includes remotetransmit options;
#                    used to transmit objects to remote processes
#                when false, __getstate__ does not include nosave options;
#                    used to save objects to disk
global remote_pickle
remote_pickle= False
def get_remote_pickle():
    global remote_pickle
    return remote_pickle
def set_remote_pickle(rp):
    global remote_pickle
    prev= remote_pickle
    remote_pickle= rp
    return prev


class WrappedPLearnObject(object):
    """
    Wrapper class for PLearn objects used from Python.
    """
    
    # for debug purposes: can print warnings when setting attributes
    # which are not PLearn options 
    allowed_non_PLearn_options= ['_cptr']
    warn_non_PLearn_options= False # you can turn this on to help debugging

    def __init__(self, **kwargs):
        """
        ctor.: manage pointer to underlying C++ object
        """
        if '_cptr' in kwargs:
            self._cptr= kwargs['_cptr'] # ptr to c++ obj
        elif hasattr(self,'_cptr'):
            self.setOptions(kwargs)        
            
    def setOptions(self, kwargs):
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
            if self.warn_non_PLearn_options \
                    and attr not in self.allowed_non_PLearn_options:
                warnings.warn("This is not a PLearn option: '"+attr
                              +"' (for class "+self.__class__.__name__+")")
            object.__setattr__(self, attr, val)

    def __getattr__(self, attr):
        if attr in self._optionnames:
            return self.getOption(attr)
        else:
            raise AttributeError("no attribute "
                                 + attr + " in "
                                 + repr(self))

    def __del__(self):
        if hasattr(self, '_cptr'):
            self._unref()

    # DEPRECATED: now use Python's default <modulename.classname object>
    #def __repr__(self):
    #    return self.asString() #PLearn repr. for now

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
        """
        Returns self's dict, except that the value associated with the
        '_cptr' key is replaced by a PLearn class name with a dict of
        options and values.
        """
        PLEARN_PICKLE_PROTOCOL_VERSION= 2
        d= self.__dict__.copy()
        d['_cptr']= (PLEARN_PICKLE_PROTOCOL_VERSION,self.classname(),{})
        for o in self._optionnames:
            if 'nosave' not in self._optionnames[o] \
                    or get_remote_pickle() and 'remotetransmit' in self._optionnames[o]:
                d['_cptr'][2][o]= self.getOption(o)
        return d
    ##### old, deprecated version follows: (for reference only)
    def old_deprecated___getstate__(self):
        d= self.__dict__.copy()
        if remote_pickle:
            d['_cptr']= self.asStringRemoteTransmit()
        else:
            d['_cptr']= self.asString()
        return d
    
    def __setstate__(self, dict):
        """
        Unpickle wrapped PLearn objects pickled using
        pybridge's 2nd protocol; the original protocol is
        also supported for backward compatibility
        (through old_deprecated___setstate__)
        Protocol #2 expects a standard pickle dictionary,
        except that the '_cptr' element of this dict. is
        a 3-tuple as follows:
          ( <protocol version>, <PLearn class name>, <dict of PLearn options->values> )
        e.g.:
          (2, 'LinearRegressor', {'weight_decay': 1e-3})
        Note that only protocol version 2 is allowed; the version 1 protocol
        uses a totally different format (see old_deprecated___setstate__) 
        """
        d= dict['_cptr']
        if isinstance(d, str):# Protocol v.1 (deprecated)
            return self.old_deprecated___setstate__(dict)
        # Protocol v.2:
        PLEARN_PICKLE_PROTOCOL_VERSION= 2
        if d[0] != PLEARN_PICKLE_PROTOCOL_VERSION:
            raise RuntimeError, "PLearn pickle protocol version should be 2"

        if not hasattr(self, '_cptr'):
            # TODO: check that this works in all cases...
            newone= plearn_module.newObjectFromClassname(d[1])
            self._cptr= newone._cptr
            self._refCPPObj(self, False)

        # empty PLearn object already exists (from __new__)
        for k in dict:
            if k != '_cptr':
                self.__setattr__(k, dict[k])
        for o in d[2]:
            self.setOptionFromPython(o,d[2][o])
        self.build()
        return dict

    def old_deprecated___setstate__(self, dict):
        """
        Provide support to unpickle objects saved
        in the original (PLearn) format.  This is
        automatically called from __setstate__ when
        needed.
        """
        newone= plearn_module.newObject(dict['_cptr'])
        self._cptr= newone._cptr
        self._refCPPObj(self, False)
        for k in dict:
            if k != '_cptr':
                self.__setattr__(k, dict[k])
        return dict


class WrappedPLearnVMat(WrappedPLearnObject):
    """
    Specialized wrapper for PLearn VMatrices;
    supplies __len__ and __getitem__ methods.
    """
    def __len__(self):
        return self.getLength()

    def __getitem__(self, key):
        class len_thunk(object):
            """
            get length only as needed, and only once.
            """
            __slots__ = ['vm','l']
            def __init__(self, vm):
                self.vm= vm
                self.l= None
            def __call__(self):
                self.l= self.l or len(self.vm)
                return self.l
        lt= len_thunk(self)
        if isinstance(key, int):
            if key < 0: key+= lt()
            try:
                return self.getRow(key)
            except get_plearn_module().PLearnError, e:
                if 'OUT OF BOUND' in e.message:
                    raise IndexError(e)
                else:
                    raise
        if isinstance(key, slice):
            start= key.start or 0
            stop= key.stop or lt()-1
            step= key.step or 1
            if start < 0: start+= lt()
            if stop < 0: stop+= lt()
            if step==1:
                return self.subMat(start, 0, stop-start, self.width)
            raise NotImplementedError('slice step != 1')
        raise TypeError("key should be an int or a slice")
    
class RealRange:
    """
    To support PLearn<->Python conversion of RealRange (which is not a PLearn Object)
    """
    def __init__(self, leftbracket, low, high, rightbracket):
        self.leftbracket = leftbracket
        self.low = low
        self.high = high
        self.rightbracket = rightbracket

class VMField:
    """
    To support PLearn<->Python conversion of VMField (which is not a PLearn Object)
    """
    def __init__(self, name, fieldtype):
        self.name = name
        self.fieldtype = fieldtype

class FieldType:
    """
    C-type enum implementation for PLearn::VMField::FieldType
    """
    UnknownType    = 0
    Continuous     = 1
    DiscrGeneral   = 2
    DiscrMonotonic = 3
    DiscrFloat     = 4
    Date           = 5

# vim: filetype=python:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
