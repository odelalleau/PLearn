#
# Build snippet for a PythonCodeSnippet
#
class __injected_functions:
    def __init__(self, injected):
        self.__dict__ = injected
        
        def frozen_setattr(self, key, val): raise TypeError("Read-only object.")
        self.__dict__['__setattr__'] = frozen_setattr
        
import __builtin__  
__builtin_import__ = __builtin__.__import__
def __inject_import__(name, globals_arg=None, locals_arg=None, fromlist=[]):
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
    
    module = __builtin_import__(name, globals_arg, locals_arg, fromlist)
    if '__injected__' in globals_arg:
        setattr(module, 'injected', __injected_functions(globals_arg['__injected__']))
    return module

__builtin__.__import__ = __inject_import__

