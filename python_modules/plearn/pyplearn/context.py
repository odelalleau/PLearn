# context.py
# Copyright (C) 2006 Christian Dorion
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

# Author: Christian Dorion
"""Context management for PyPLearn scripts.

Opening multiple PyPLearn scripts within the same Python interpreter
requires each script to be encapsulated in some I{context}, independent and
disconnected from other contexts. This module implement the notion of context.

This module provides functions to access or manage contexts. The
L{actualContext()} function is the only one given direct access to the
(private) context objects. It must handled with care where and only where
the designer of some PyPLearn module wants to memoize some context
sensitive informations. B{Most PyPLearn users will never be (or need to be)
aware of the notion of context.}
"""
import inspect, os, time
from plearn.pyplearn import config

def actualContext(cls):
    """Function returning the actual context object.

    All other functions in this module manipulate context through
    handles. This function allows one to actually modify the current
    context: B{advised users only}.
    """
    context = __contexts[__current_context]
    context.buildClassContext(cls)
    return context

def createNewContext():
    global __contexts, __current_context

    contextual_classes = __contexts[__current_context].built_class_contexts

    __current_context = len(__contexts)
    __contexts.append( __Context(contextual_classes) )

    return __current_context

def generateExpdir( ):
    """Generates a standard experiment directory name."""
    from plearn.utilities.toolkit import date_time_string    

    expdir = "expdir"
    if (os.environ.get('PYTEST_STATE', '') != 'Active'):
        expdir = '%s_%s' % ( expdir, date_time_string('_','_') )

    return expdir

def getCurrentContext():
    return __current_context

def setCurrentContext(handle):
    global __current_context
    __current_context = handle

#######  Classes and Module Variables  ########################################

class __Context(object):
    __expdirs = []
    
    def __init__(self, contextual_classes=[]):
        self.built_class_contexts = []
        for cls in contextual_classes:
            self.buildClassContext(cls)
        
    def buildClassContext(self, cls):
        # try:
        #     print cls, cls.__name__, id(cls),
        #     print id(self.built_class_contexts), self.built_class_contexts,
        #     print (cls not in self.built_class_contexts 
        #            and hasattr(cls, "buildClassContext"))
        # except Exception, e:
        #     print e
        #     raise RuntimeError(e)
        
        assert inspect.isclass(cls)
        if cls not in self.built_class_contexts \
               and hasattr(cls, "buildClassContext"):

            # print "Within!",
            # print id(cls), id(self.built_class_contexts), self.built_class_contexts,
            # print (cls not in self.built_class_contexts 
            #        and hasattr(cls, "buildClassContext"))
            
            cls.buildClassContext(self)            
            self.built_class_contexts.append(cls)
            
    def getExpdir(self):        
        if not hasattr(self, '_expdir_'):
            expdir = generateExpdir()            
            attempt = 1
            while expdir in self.__expdirs:
                time.sleep(1)
                expdir = generateExpdir()
                attempt += 1
                if expdir == "expdir":
                    expdir = "expdir%d"%attempt
                
            self._expdir_ = expdir
            self.__expdirs.append(expdir)

        if not hasattr(self, '_expdir_root_'):
            self._expdir_root_ = config.get_option('EXPERIMENTS', 'expdir_root')

        return os.path.join(self._expdir_root_, self._expdir_)

__contexts = [ __Context() ]
__current_context = 0
