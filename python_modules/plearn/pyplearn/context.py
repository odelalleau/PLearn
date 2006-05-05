import os
from plearn.pyplearn import config
from plearn.pyplearn.pyplearn import generate_expdir

def actualContext():
    """Function returning the actual context object.

    All other functions in this module manipulate context through
    handles. This function allows one to actually modify the current
    context: B{advised users only}.
    """
    return __contexts[__current_context]

__all_context_binders = []
def allContextsBinder(binder):
    assert not binder in __all_context_binders
    __all_context_binders.append(binder)

def createNewContext():
    global __contexts, __current_context

    __current_context = len(__contexts)
    __contexts.append( __Context() )

    for binder in __all_context_binders:
         actualContext().binders[binder.__name__] = binder

    return __current_context

def getCurrentContext():
    return __current_context

def setCurrentContext(handle):
    global __current_context
    __current_context = handle

#######  Classes and Module Variables  ########################################

class __Context(object):
    __expdirs = []
    
    def __init__(self):
        self.binders = {}
        self.namespaces = {}
        self.plopt_holders = {}
        self.plopt_overrides = {}

    def getExpdir(self):        
        if not hasattr(self, '_expdir_'):
            expdir = generate_expdir()
            while expdir in self.__expdirs:
                expdir = generate_expdir()
            self._expdir_ = expdir
            self.__expdirs.append(expdir)
        if not hasattr(self, '_expdir_root_'):
            self._expdir_root_ = config.get_option('EXPERIMENTS', 'expdir_root')
        return os.path.join(self._expdir_root_, self._expdir_)

__contexts = [ __Context() ]
__current_context = 0
