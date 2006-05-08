import inspect, os, time
from plearn.pyplearn import config
from plearn.pyplearn.pyplearn import generate_expdir

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
        assert inspect.isclass(cls)
        if cls not in self.built_class_contexts \
               and hasattr(cls, "buildClassContext"):
            cls.buildClassContext(self)            
            self.built_class_contexts.append(cls)
            
    def getExpdir(self):        
        if not hasattr(self, '_expdir_'):
            expdir = generate_expdir()            
            attempt = 1
            while expdir in self.__expdirs:
                time.sleep(1)
                expdir = generate_expdir()
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
