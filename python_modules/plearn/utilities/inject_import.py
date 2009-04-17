import os, sys

# Should be manipulated from within C++
CURRENT_SNIPPET = []
def setCurrentSnippet(handle):
    CURRENT_SNIPPET.append(handle)

def resetCurrentSnippet():
    CURRENT_SNIPPET.pop(-1)

# Add '' (current directory) to sys.path if it's not already there.
# (When a python interpreter is embedded within a C program, the
# current directory is not part of the python search path, whereas
# the normal python behavior is to have it when starting a normal
# interpreter from the shell).
if not '' in sys.path:
    sys.path = [ '' ] + sys.path

# In some embedded Python systems (2.4?), argv may not be in sys.
# If it's not, add an empty one.
if not 'argv' in sys.__dict__:
    sys.argv = ['']

# Class managing the injected functions
class __injected_functions:        
    def __init__(self, injected):
        assert CURRENT_SNIPPET
        self.__dict__['_snippet_map'] = { CURRENT_SNIPPET[-1] : injected }

    def __setattr__(self, key, val):
        injected = self._snippet_map[CURRENT_SNIPPET[-1]]
        injected[key] = val

    def __getattr__(self, key):
        injected = self._snippet_map[CURRENT_SNIPPET[-1]]
        if key in injected:
            return injected[key]
        raise AttributeError, key

    def __str__(self):
        return str(CURRENT_SNIPPET[-1])

    def update(self, injected):
        assert CURRENT_SNIPPET
        self._snippet_map[CURRENT_SNIPPET[-1]] = injected

# Overriding builtin import function
import __builtin__  
__builtin_import__ = __builtin__.__import__
def __inject_import__(name, globals_arg=None, locals_arg=None, fromlist=[], level=-1):
    """Import function that injects locals in the imported module.

    This function is meant to override the builtin I{__import__} function
    called by the I{import} statement. Its works somehow like if you would
    have called I{execfile} on the module's implementation file. It is
    however preferable in the PyPLearn mechanism's context.
    
    The I{globals_arg} argument defaults to I{globals()} and is used only
    as a default to the I{local_args} dictionary. After a standard import,
    the later is injected in the imported module. Hence, the imported
    module can rely on functions or classes expected to be defined at the
    moment of the import.
    """    
    if globals_arg is None:
        globals_arg = globals()
    
    if locals_arg is None:
        locals_arg = globals_arg

    # print >>sys.stderr, "Python path is:",sys.path
    # print >>sys.stderr, "Current directory is:",os.getcwd()
    module = __builtin_import__(name, globals_arg, locals_arg, fromlist, level)
    if '__injected__' in globals_arg:
        if hasattr(module, 'injected'):
            module.injected.update(globals_arg['__injected__'])
        else:
            setattr(module, 'injected', __injected_functions(globals_arg['__injected__']))
    return module

__builtin__.__import__ = __inject_import__

