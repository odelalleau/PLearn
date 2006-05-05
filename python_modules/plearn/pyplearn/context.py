
class __ListMap(dict):
    def __getitem__(self, key):
        if not key in self:
            self.__setitem__(key, [])
        return super(__ListMap, self).__getitem__(key)

class __Context(object):
    def __init__(self):
        self.binders = {}
        self.namespaces = {}
        self.plopt_holders = {}
        self.plopt_overrides = {}

__contexts = [ __Context() ]
__current_context = 0

def actualContext():
    """Function returning the actual context object.

    All other functions in this module manipulate context through
    handles. This function allows one to actually modify the current
    context: B{advised users only}.
    """
    return __contexts[__current_context]

def createNewContext():
    global __contexts, __current_context
    plargs = actualContext().binders['plargs']
    __current_context = len(__contexts)
    __contexts.append( __Context() )
    actualContext().binders['plargs'] = plargs
    return __current_context

def getCurrentContext():
    return __current_context

def setCurrentContext(handle):
    global __current_context
    __current_context = handle

