"""Utility for managing global variables in a multi-modules program.

The L{globalvars} variable is an instance of an empty class. It simply
allows one to encapsulate global variables in a clear scope. It also
allows declaring global variables in the main module and using these
afterwards in other modules. For instance, the following would print 'bar'::

    ## main.py
    from plearn.utilities.global_variables import globalvars
    from other_module import print_foo

    globalvars.foo = 'bar'
    print_foo()
    ## end

    ## other_module.py
    from plearn.utilities.global_variables import globalvars

    def print_foo():
        print globalvars.foo
    ## end of other_module.py

B{Note}: This use of the L{globalvars} instance allows one to create a
large program and split it in different modules while using the same
global variables. Still, using L{globalvars} this way B{is nothing
more than a clean hack} since clean modularization should be such that
all modules define the variable they need, letting a main module link
the components altogether...
"""
__cvs_id__ = "$Id: global_variables.py,v 1.3 2004/12/21 15:31:50 dorionc Exp $"

class __global_variables:
    """An empty class creating a scope for global variables."""
    def __init__(self):
        pass
    
globalvars = __global_variables()
