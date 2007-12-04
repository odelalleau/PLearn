# plargs.py
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
"""Management of command-line options.

A custom (and encouraged) practice is to write large PyPLearn scripts
which behaviour can be modified by command-line arguments, e.g.:: 
        
    prompt %> plearn command_line.pyplearn on_cmd_line="somefile.pmat" input_size=10

with the command_line.pyplearn script being::

    #
    # command_line.pyplearn
    #
    from plearn.pyplearn import *

    dataset = pl.AutoVMatrix( specification = plargs.on_cmd_line,
                              inputsize     = int( plargs.input_size ),
                              targetsize    = 1
                              )

    def main():
        return pl.SomeRunnableObject( dataset  = dataset,
                                      internal = SomeObject( dataset = dataset ) )

Those command-line arguments are widely refered as L{plargs}, on
account of this class, troughout the pyplearn mechanism. Note that
I{unexpected} (see binders and L{namespaces<plnamespace>} hereafter)
arguments given on the command line are interpreted as strings, so if
you want to pass integers (int) or floating-point values (float), you
will have to cast them as above.

To set default values for some arguments, one can use
L{plarg_defaults<_plarg_defaults>}. For instance::

    # 
    # command_line_with_defaults.pyplearn
    #
    from plearn.pyplearn import *

    plarg_defaults.on_cmd_line = "some_default_file.pmat"
    plarg_defaults.input_size  = 10
    dataset = pl.AutoVMatrix( specification = plargs.on_cmd_line,
                              inputsize     = plargs.input_size,
                              targetsize    = 1
                              )

    def main( ):
        return pl.SomeRunnableObject( dataset  = dataset,
                                      internal = SomeObject( dataset = dataset ) )
    
which won't fail and use C{"some_default_file.pmat"} with C{input_size=10} if::

    prompt %> plearn command_line_with_defaults.pyplearn

is entered. Note that since I{input_size} was defined as an int
(C{plarg_defaults.input_size = 10}). Even if
L{plarg_defaults<_plarg_defaults>} is still supported, it is preferable
to define all arguments' default values through some I{binder}. We
refer to subclasses of L{plargs} as I{binders} since they bind default
values to expected/possible command-line arguments, e.g::

    # 
    # my_script.pyplearn
    #
    from plearn.pyplearn.plargs import *

    class Misc(plargs):
        algo      = "classical"
        ma_len    = plopt([126, 252],
                          doc="A list of moving average lenghts to be used "
                          "during the preprocessing.")
        n_inputs  = plopt(10, doc="The number of inputs to be used.")

    print >>sys.stderr, repr(plargs.algo), repr(plargs.n_inputs)
    assert plargs.n_inputs==Misc.n_inputs

    print >>sys.stderr, Misc.ma_len
        
would print (as first line) C{"classical", 10}, a string and an int, if
no command-line arguments override those L{plargs}. One can always
access the I{plarg} through C{plargs} or C{Misc} (i.e. the assertion
never fails).

Note the use of L{plopt} instances. While these are not mandatory, they
are very useful and powerful tools which one can use to make his
scripts clearer and more user-friendly when used within
U{plide<http://plearn.berlios.de/plide>}. Note that list can be
provided to as command-line argument in the CSV format, that is::

    prompt %> plearn my_script.pyplearn ma_len=22,63,126,252
    "classical", 10
    [22, 63, 126, 252]

While I{binders} allow one to define default values for L{plargs},
these are limited in the sens that clashes can occur::

    # 
    # complex_script.pyplearn
    #
    from plearn.pyplearn.plargs import *
    
    class macd(plargs):
        ma_len = plopt(252,
                    doc="The moving average length to be used in the macd model.")

    class PCA(plargs):
        algo      = "classical"
        ma_len    = plopt([126, 252],
                          doc="A list of moving average lenghts to be used "
                          "during the preprocessing.")
        n_inputs  = plopt(10, doc="The number of inputs to be used.")

    ...
    #####
    prompt %> plearn complex_script.pyplearn 
    Traceback (most recent call last):
    File "TEST.pyplearn", line 7, in ?
        class PCA(plargs):
    File "/home/dorionc/PLearn/python_modules/plearn/pyplearn/plargs.py", line 447, in __new__
        plopt.define(cls, option, value)
    File "/home/dorionc/PLearn/python_modules/plearn/pyplearn/plargs.py", line 193, in define
        raise KeyError(
    KeyError: "A script should not contain two options of the same name. Clashing
    definition of plarg 'ma_len' in 'macd' and 'PCA'"

To avoid this type of error, one can L{namespace<plnamespace>} the
arguments of his script::

    # 
    # namespaced.pyplearn
    #
    from plearn.pyplearn.plargs import *
    
    class macd(plnamespace):
        ma_len = plopt(252,
                    doc="The moving average length to be used in the macd model.")

    class PCA(plnamespace):
        algo      = "classical"
        ma_len    = plopt([126, 252],
                          doc="A list of moving average lenghts to be used "
                          "during the preprocessing.")
        n_inputs  = plopt(10, doc="The number of inputs to be used.")

    print >>sys.stderr, macd.ma_len, PCA.algo, PCA.ma_len

    try:
        print >>sys.stderr, plargs.n_inputs
    except AttributeError:
        print >>sys.stderr, \
              "n_inputs is namespaced, you must access it through 'PCA'"

    #####
    prompt %> plearn namespaced.pyplearn macd.ma_len=126 PCA.algo=weird
    252 weird [126, 252]
    n_inputs is namespaced, you must access it through 'PCA'

Finally, B{note that} the value of C{plargs.expdir} is generated automatically and
B{can not} be assigned a default value through binders. This behaviour
aims to standardize the naming of experiment directories. For debugging
purpose, however, one may provide on command-line an override to
C{plargs.expdir} value. Otherwise, one can also provide the I{expdir_root}
command-line argument as a (relative) path where the expdir should be
found, e.g.::

    prompt %> plearn my_script.pyplearn expdir_root=Debug

would cause experiment to use the expdir::

    Debug/expdir_2006_05_10_07_58_10
    
for instance.
"""
import copy, inspect, logging, new, re, sys
from plearn.pyplearn.context import *
from plearn.utilities.Bindings import Bindings

# Helper functions

def neglected_member(name, value):
    return (name.startswith('_') 
            or inspect.ismethod(value) or inspect.isfunction(value)
            or inspect.isroutine(value) or inspect.isclass(value) )

def list_cast(slist, elem_cast):
    """Intelligently casts I{slist} string to a list.

    The I{slist} argument can have the following forms::

        - CSV::
            slist = "1,2,3" => casted = [1, 2, 3]
        - CSV with brackets::
            slist = "[hello,world]" => casted = ["hello", "world"]
        - List of strings::
            slist = [ "100.0", "102.5" ] => casted = [ 100.0, 102.5 ]

    The element cast is made using the I{elem_cast} argument.
    """
    # CSV (with or without brackets)
    slist = slist.strip()
    #print >>sys.stderr, repr(slist), type(slist)
    if isinstance(slist,str):
        if slist=="":
            slist = []
        elif slist.startswith("["):
            assert slist.endswith("]")
            slist = eval(slist)
        else:
            slist = slist.split(",")
        return [ elem_cast(e) for e in slist ]

    # List of strings
    elif isinstance(slist, list):
        return [ elem_cast(e) for e in slist ]

    else:
        raise ValueError, "Cannot cast '%s' into a list", str(slist)

def warn(message, category=UserWarning, stacklevel=0):
    import os
    pytest_state = os.environ.get("PYTEST_STATE", "")
    if pytest_state!="Active":        
        from warnings import warn
        warn(message, category, stacklevel=stacklevel+3)

#######  Classes to Manage Command-Line Arguments  ############################

class plopt(object):
    """Typed command-line options with constraints for PLearn.
    
    This class provides support for default values, strong type checking,
    conversion from a string representation, documentation, and constraints
    on the possible values that the option can take.  The syntax to follow
    is along the lines of::
    
        class MyOptions( plargs_namespace ):
            plot_stuff  = plopt(True, doc='Whether a cute graph should be plotted')
            K           = plopt(10, min=5, max=25, doc='Number of neighbors to consider')
            method      = plopt('classical', choices=['new-kind','classical','crazy'])
            lags        = plopt([1,22,252], doc='Lags to incorporate as inputs, in days')
            weight_cols = plopt([], elem_type=int)

    The supported keywords are

        - B{doc}: The help string for that option.

        - B{choices}: A list of values accepted for that option. Trying to
          set this option's value to anything which is not in that list will cause
          a ValueError to be raised.

        - B{min, max}: Bounds for numeric values. You can specify only one
          of the two. Trying to set this option to value out of these bounds will
          cause a ValueError to be raised.

        - B{free_choices}: A list of suggestions as to what the field should contain.
          Still, this option can be set to whatever value you wish and a ValueError
          will never be raised.

        - B{type}: This keyword can and must only be used when the default
          value is None. Otherwise, it is infered using 'type(value)'.

        - B{elem_type}: For list options, the type of the elements is
          usually infered from the first element of the list. This keyword
          may however be used to specify the type for the elements of a
          list which defaults as empty. If the list defaults as empty
          I{and} 'elem_type' keyword is not provided, then the elements' type is
          assumed to be 'str'.

    A plopt I{holder} refers to either a plargs binder or a plnamespace.
    """
    
    unnamed = "### UNNAMED ###"

    def __init__(self, value, **kwargs):
        self._name  = kwargs.pop("name", self.unnamed)
        self._doc   = kwargs.pop("doc", '')
        self._gui   = kwargs.pop("gui", True)

        # type: This keyword can and must only be used when the default
        # value is None. Otherwise, it is infered using 'type(value)'.
        assert not ("type" in kwargs and value is not None)
        if value is None:
            if "type" not in kwargs:
                warn("When a plopt value defaults to None, a valid type must be "
                     "provided using the 'type' keyword argument. "
                     "String is assumed for now...", FutureWarning, 2)
                kwargs['type'] = str
            self._type = kwargs.pop("type")
        else:
            self._type = type(value)

        # Keep the remaining kwargs for further use.
        self._kwargs = kwargs

        # Sanity checks
        self.checkBounds(value)
        self.checkChoices(value)
        self.__default_value = value # The check did not raise: 'value' is valid

    def __str__(self):
        """Short string representation of a plopt instance.

        Either::
            plopt(value = DEFAULT_VALUE)
        or::
            plopt(value = VALUE [default: DEFAULT_VALUE])

        where the second occurs if the current value is not equal to the default value.
        """
        value = self.get()
        value_str = repr(value)
        if value != self.__default_value:
            value_str += " [default: %s]"%repr(self.__default_value)
        return "plopt(value = %s)"%value_str

    def cast(self, value):
        casted = None
        
        # Special string to bool treatment
        if self._type is bool and isinstance(value, str):
            if value == "True":
                casted = True
            elif value == "False":
                casted = False
            else:
                raise ValueError("Trying to set value %s to bool-typed option %s"
                                 %(value, self._name))

        # Special treatment for list option
        elif self._type is list :
            elem_type = self._kwargs.pop("elem_type", None) 
            if elem_type is None:
                got= self.get()
                if got!=None and len(got) > 0:
                    elem_type = type(self.get()[0])
                else:
                    elem_type = str
            casted = list_cast(value, elem_type)

        elif self._type is dict:
            if isinstance(value, str):
                casted = eval(value)
            else:
                casted = value
            
        # Simple type cast
        else:
            casted = self._type(value)

        return casted
    
    def checkBounds(self, value):
        """Checks that value lies between 'min' and 'max' bounds parsed from self.kwargs."""
        minimum = self._kwargs.get("min", None)
        if minimum is not None and value < minimum:
            raise ValueError("Option %s (=%s) should greater than %s"
                             %(self._name, repr(value), repr(minimum)))

        maximum = self._kwargs.get("max", None)
        if maximum is not None and value > maximum:
            raise ValueError("Option %s (=%s) should lower than %s"
                             %(self._name, repr(value), repr(minimum)))

    def checkChoices(self, value):
        """Checks that 'value' is one of the 'choices' parsed from self.kwargs."""
        choices = self._kwargs.get("choices", None)
        if choices is not None and value not in choices:
            name = ""
            if self._name != self.unnamed:
                name = self._name+" " 
            raise ValueError(
                "Option %sshould be in choices=%s"%(name, choices))

    def get(self):
        """Returns the current context override, if any, o/w returns the default value."""
        plopt_overrides = actualContext(plopt).plopt_overrides
        if self in plopt_overrides:
            return plopt_overrides[self]
        return self.__default_value

    def getBounds(self):
        minimum = self._kwargs.get("min", None)
        maximum = self._kwargs.get("max", None)
        return minimum, maximum

    def getChoices(self):
        return self._kwargs.get("choices", None)

    def getDefault(self):
        return self.__default_value

    def getFreeChoices(self):
        return self._kwargs.get("free_choices", None)

    def getName(self):
        assert self._name != self.unnamed
        return self._name

    def getType(self):
        return self._type

    def getDoc(self):
        return self._doc

    def getGui(self):
        return self._gui

    def reset(self):
        """Simply deletes any override for this plopt in the current context.
        
        Note that the actual implementation of C{define()} leads to
        forgetting the command-line override if C{reset()} is called on the
        plopt instance!
        """
        actualContext(plopt).plopt_overrides.pop(self, None)
        
    def set(self, value):
        """Sets an override for this plopt in the current context"""
        if not isinstance(value, self._type):
            value = self.cast(value)
        # Sanity checks 
        self.checkBounds(value)
        self.checkChoices(value)
    
        # The previous didn't raise exeption, the 'value' is valid
        actualContext(plopt).plopt_overrides[self] = value

    def setDefault(self, default):
        self.__default_value = default

    def setDoc(self, doc):
        self._doc= doc

    #######  Static methods  ######################################################

    def addCmdLineOverride(holder_name, optname, value):
        context = actualContext(plopt)
        hldr_key = (holder_name, optname)
        # Use the 'unique' key to store command-line override's value
        context.plopt_cmdline_overrides[hldr_key] = value
    addCmdLineOverride = staticmethod(addCmdLineOverride)

    def popCmdLineOverride(actual_holder, optname):
        context = actualContext(plopt)
        hldr_key = (actual_holder.__name__, optname)
        override_value = context.plopt_cmdline_overrides.pop(hldr_key, None)

        # If the option is not namespaced, it may have been provided
        # without the binder name as prefix. Try accessing the key with
        # None instead of the holder's name
        if override_value is None \
           and not issubclass(actual_holder, plnamespace):
            hldr_key = (None, optname)
            override_value = context.plopt_cmdline_overrides.pop(hldr_key, None)

        # Will return None if no option named 'optname' were overridden on
        # the command line
        return override_value
    popCmdLineOverride = staticmethod(popCmdLineOverride)

    def buildClassContext(context):
        assert not hasattr(context, 'plopt_binders')
        context.plopt_binders = {}

        assert not hasattr(context, 'plopt_namespaces')
        context.plopt_namespaces = {}

        assert not hasattr(context, 'plopt_overrides')
        context.plopt_overrides = {}

        assert not hasattr(context, 'plopt_cmdline_overrides')
        context.plopt_cmdline_overrides = {}
    buildClassContext = staticmethod(buildClassContext)

    def closeClassContext(context):
        exceptions = [ 'FILEBASE', 'FILEPATH', 'TIME', 'DATETIME',
                       'FILEEXT', 'DIRPATH', 'DATE', 'HOME', 'FILENAME' ]

        del context.plopt_binders
        del context.plopt_namespaces
        del context.plopt_overrides

        unused = []
        for (hldr_name, optname), value in context.plopt_cmdline_overrides.iteritems():
            if optname in exceptions:
                continue
            elif hldr_name is None:
                unused.append("%s=%s"%(optname, value))
            else:
                unused.append("%s.%s=%s"%(hldr_name, optname, value))
                
        if unused:
            from plearn.pyplearn import PyPLearnError
            raise PyPLearnError(
                "The following command-line arguments were not expected "
                "(Misspelled? Or namespaced?): %s" % ", ".join(unused))

        # Finally, delete the list
        del context.plopt_cmdline_overrides
    closeClassContext = staticmethod(closeClassContext)    

    def define(holder, option, value):
        """Typical pattern to set a plopt instance member in 'holder' for the first time."""
        context = actualContext(plopt)

        # Check if an override was defined through parsing before the
        # holder existed. Returns None in the case no override exists.
        cmdline_override = plopt.popCmdLineOverride(holder, option)

        ### Holder-type specific management
        plopt._inner_define(holder, option, value)

        # Command-line MUST override mandatory plopts
        if isinstance(value, mandatory_plopt) and cmdline_override is None:
            from plearn.pyplearn import PyPLearnError
            raise PyPLearnError(
                "Mandatory argument %s.%s was not received on command line."
                %(holder.__name__, value.getName()) )
        
        # Overrides the default with the command-line override if any. Note
        # that this way to proceed leads to forgetting the command-line
        # override if reset() is called on the plopt instance!!!
        if cmdline_override is not None:
            plopt.override(holder, option, cmdline_override)            
    define = staticmethod(define)

    def _inner_define(holder, option, value):
        """Holder-type specific management."""
        context = actualContext(plopt)
            
        if issubclass(holder, plargs):
            # A script should not contain two options of the same name
            if option in context.plopt_binders:
                raise KeyError(
                    "A script should not contain two options of the same name. "
                    "Clashing definition of plarg '%s' in '%s' and '%s'"%
                    (option,context.plopt_binders[option].__name__,holder.__name__))
        
            # Keep a pointer to the binder in which the option is defined
            context.plopt_binders[option] = holder

        elif issubclass(holder, plnamespace):
            # Keep a pointer to the namespace in which the option is defined
            context.plopt_namespaces[option] = holder

        else:
            raise TypeError("Holder '%s' is of an unknown type: %s"
                            % (holder.__name__, type(holder)) )

        # Enforce all 'holder' members to be (named) plopt instances
        if isinstance(value, plopt):
            value._name = option
        else:
            value = plopt(value, name=option)

        # Acutally sets the plopt in the holder
        type.__setattr__(holder, option, value)        
    _inner_define = staticmethod(_inner_define)

    def getHolder(plopt_name):        
        return actualContext(plopt).plopt_binders.get(plopt_name)
    getHolder = staticmethod(getHolder)

    def iterator(holder):
        """Returns an iterator over the plopt instances contained in the I{holder}"""
        keys = holder.__dict__.keys()
        keys.sort()
        return iter([ holder.__dict__[key] for key in keys
                      if isinstance(holder.__dict__[key], plopt) ])
    iterator = staticmethod(iterator)

    def optdict(holder):
        """
        returns a dict of {plopt_name: plopt_value} for all options in holder
        """
        return dict([ (opt.getName(), opt.get()) for opt in plopt.iterator(holder) ])
    optdict = staticmethod(optdict)

    def optdict2(holder):
        """
        returns a dict of {plopt_name, plopt} for all options in holder
        """
        return dict([ (opt.getName(), opt) for opt in plopt.iterator(holder) ])
    optdict2 = staticmethod(optdict2)

    def copyOptionFrom(holder, option):
        """
        returns a [deep] copy of holder's option 'option'
        """
        return copy.deepcopy(plopt.optdict2(holder)[option])
    copyOptionFrom= staticmethod(copyOptionFrom)

    def override(holder, option, value):
        """Typical pattern to override the value of an existing plopt instance."""
        plopt_instance = type.__getattribute__(holder, option)
        plopt_instance.set(value)
    override = staticmethod(override)

class mandatory_plopt(plopt):
    def __init__(self, type, **kwargs):
        self._type  = type
        self._name  = kwargs.pop("name", self.unnamed)
        self._doc   = kwargs.pop("doc", '')        

        # Keep the remaining kwargs for further use.
        self._kwargs = kwargs

        # For __str__ use only
        self.__default_value = None
    
class plargs(object):
    """Values read from or expected for PLearn command-line variables.

    The core class of this module. See modules documentation for details on
    possible uses.
    """
    _extensible_ = False

    #######  Static methods  ######################################################

    def buildClassContext(context):
        assert not hasattr(context, 'binders')
        context.binders = {}
    buildClassContext = staticmethod(buildClassContext)

    def getBinders():
        """Returns a list of all binders in the current context."""
        context = actualContext(plargs)
        binder_names = context.binders.keys()
        binder_names.sort()

        binders = []
        for bname in binder_names:
            binders.append(context.binders[bname])        
        return binders
    getBinders = staticmethod(getBinders)

    def getContextBindings():
        """Returns a L{Bindings} instance of plopt name-to-value pairs.

        The bindings could thereafter be used to re-create the current
        context from a command-line::

            bindings = plargs.getContextBindings()
            cmdline = [ "%s=%s"%(opt, bindings['opt']) for opt in bindings ]
            actual_command_line = " ".join(cmdline)
        """
        context = actualContext(plargs)
        bindings = Bindings( )

        for binder in plargs.getBinders():
            for opt in plopt.iterator(binder):
                bindings[opt.getName()] = opt.get()

        for namespace in plargs.getNamespaces():
            for opt in plopt.iterator(namespace):
                key = "%s.%s"%(namespace.__name__, opt.getName())
                bindings[key] = opt.get()

        return bindings
    getContextBindings = staticmethod(getContextBindings)

    def getNamespaces():
        """Returns a list of all namespaces in the current context."""
        # Note the (hackish?) use of plnamespace. Needed to ensure the
        # existance of context.namepaces...
        context = actualContext(plnamespace) 
        nsp_names = context.namespaces.keys()
        nsp_names.sort()        
        return [ context.namespaces[nsp] for nsp in nsp_names ]
    getNamespaces = staticmethod(getNamespaces)

    def getHolder(holder_name):
        holder = plnamespace.getHolder(holder_name)
        if holder is None:
            holder = actualContext(plargs).binders.get(holder_name)
        return holder
    getHolder = staticmethod(getHolder)

    def parse(*args):
        """Parses a list of argument strings."""
        if len(args)==1 and isinstance(args[0], list):
            args = args[0]

        context = actualContext(plargs)
        for statement in args:
            assert isinstance(statement, str), statement

            option, value = statement.split('=', 1)

            option = option.strip()
            value  = value.strip()

            if option=="expdir":
                context._expdir_ = value
                continue

            if option=="expdir_root":
                context._expdir_root_ = value
                continue            

            try:
                holder_name, option = option.split('.')
                holder = plargs.getHolder(holder_name)

            # option.split('.') did not return a pair, i.e. not dot in option
            except ValueError:
                holder_name, holder = None, plopt.getHolder(option)

            # Was the holder for that option defined?
            if holder is None:
                plopt.addCmdLineOverride(holder_name, option, value)
            else:
                setattr(holder, option, value)
    parse = staticmethod(parse)
    
    #######  Metaclass  ###########################################################
    
    class __metaclass__(type):
        """Overrides the attribute management behavior."""
        def __new__(metacls, clsname, bases, dic):
            cls = type.__new__(metacls, clsname, bases, dic)
            plargs = cls
            if clsname != "plargs":
                plargs = globals()['plargs']

            context = actualContext(plargs)
    
            # Keep track of binder subclass
            if clsname != "plargs":
                context.binders[clsname] = cls

            # Introspection of the subclasses
            if cls is not plargs:
                for option, value in dic.iteritems():                    
                    if neglected_member(option, value):
                        continue
                    
                    # Define the plopt instance
                    plopt.define(cls, option, value)

            return cls

        def __setattr__(cls, option, value):
            if option == "expdir":
                raise AttributeError("Cannot modify the value of 'expdir'.")

            plargs = cls
            if cls.__name__ != "plargs":
                plargs = globals()['plargs']

            context = actualContext(plargs)
            if cls is plargs:
                raise AttributeError(
                    "Can't set option '%s' directly on plargs. "
                    "Proceed trough the binder or namespace." % option)

            try:                    
                plopt.override(cls, option, value)
            except AttributeError, err:
                if cls._extensible_:
                    plopt.define(cls, option, value)
                else:
                    raise AttributeError(
                        "Binder %s does not contain a plopt instance named %s. "
                        "One can't set a value to an undefined option. (%s)"
                        %(cls.__name__, option, err))

        def __getattribute__(cls, key):
            attr = None
            try:
                attr = type.__getattribute__(cls, key)
                if key.startswith('_') or not isinstance(attr, plopt):
                    return type.__getattribute__(cls, key)
            except AttributeError:
                pass

            # The key should map to a plopt instance, we need the context...
            plargs = cls
            if cls.__name__ != "plargs":
                plargs = globals()['plargs']

            # Special management of expdir plarg
            if key == "expdir":
                return actualContext(plargs).getExpdir()
            elif key == "expdir_root":
                return actualContext(plargs).getExpdirRoot()

            # Find to holder to which option belongs
            holder = cls
            if cls is plargs:
                try:
                    holder = actualContext(plargs).plopt_binders[key]
                except KeyError:
                    raise AttributeError("Unknown option '%s'. Is it namespaced?"%key)

            # We now have a valid holder: dig out the plopt *value*
            plopt_instance = type.__getattribute__(holder, key)
            assert isinstance(plopt_instance, plopt)
            return plopt_instance.get()

# For backward compatibility
class _plarg_defaults:
    """Magic class to L{contextualize<context>} I{plarg_defaults}.

    The I{plarg_defaults} object exists mainly for backward compatibility
    reasons. It was (is) meant to store a default value for some command
    argument so that::

        plarg_defaults.algo = "classical"
        print plargs.algo

    would print "classical" even if no command line argument I{algo} is encountered.

    For clarity sakes, we suggest that one now regroup all default values
    he wishes to set for "plargs" in some L{binder<plargs>}, e.g.::

        class Misc(plargs):
            algo      = "classical"
            n_inputs  = 10
            n_outputs = 2

        print plargs.algo

    The behavior will be the same, but this syntax allows to I{highlight}
    the definition of default values while being more aligned with the new
    generation of U{plargs}.
    """
    def _getBinder(self):
        binders = actualContext(plargs).binders
        if 'plarg_defaults' not in binders:
            binders['plarg_defaults'] = \
                new.classobj('plarg_defaults', (plargs,), {'_extensible_' : True})
        return binders['plarg_defaults']

    def __getattribute__(self, option):
        return getattr(self._getBinder(), option)

    def __setattr__(self, option, value):
        defaults = self._getBinder()

        # Say the default was value of a given was set to 'D'. It, the
        # *default* value, can well be set modified later to 'E'. But we
        # want this to modify the script behaviour IFF the option's value
        # was *not* overriden from the command line. A simple call to
        # 'setattr' would discard the command-line override!
        if hasattr(defaults, option):
            attr_plopt = object.__getattribute__(defaults,option)
            attr_plopt.setDefault(value)
            return

        setattr(defaults, option, value)
plarg_defaults = _plarg_defaults()
        
class plnamespace:
    """Avoiding name clashes for L{plargs}.

    Alike binders, L{plnamespace} subclasses allow one to define the value
    of expected L{plargs}, but plarg names are encapsulated in namespaces. This
    allows many options to have the same name, as long as they are not in
    the same namespace::

        # 
        # namespaced.pyplearn
        #
        from plearn.pyplearn.plargs import *
        
        class macd(plnamespace):
            ma_len = plopt(252,
                        doc="The moving average length to be used in the macd model.")

        class PCA(plnamespace):
            algo      = "classical"
            ma_len    = plopt([126, 252],
                              doc="A list of moving average lenghts to be used "
                              "during the preprocessing.")
            n_inputs  = plopt(10, doc="The number of inputs to be used.")

        print >>sys.stderr, macd.ma_len, PCA.algo, PCA.ma_len

        try:
            print >>sys.stderr, plargs.n_inputs
        except AttributeError:
            print >>sys.stderr, \
                  "n_inputs is namespaced, you must access it through 'PCA'"

        #####
        prompt %> plearn namespaced.pyplearn macd.ma_len=126 PCA.algo=weird
        252 weird [126, 252]
        n_inputs is namespaced, you must access it through 'PCA'
    """    
    def buildClassContext(context):
        assert not hasattr(context, 'namespaces')
        context.namespaces = {}
    buildClassContext = staticmethod(buildClassContext)

    def getHolder(holder_name):
        return actualContext(plnamespace).namespaces.get(holder_name)
    getHolder = staticmethod(getHolder)

    def getPlopt(cls, optname):
        return super(cls, cls).__getattribute__(cls, optname)
    getPlopt = classmethod(getPlopt)

    def inherit(namespace):
        """A deep-copy driven inheritance-like mechanism.

        In the context of plnamespace, usual (Python) inheritance is not
        satisfactory. Indeed, the options defined in some base class will be
        shared among subclasses. However, when one would want to subclass a
        plnamespace, it is more likely the he want the 'subclass' to have
        options 'of the same name' than the ones in the base-class but
        still independent.

        This method provide a concise way to enact this inheritance-like
        relationship between some New and some Existing namespaces

            class New(plnamespace):
                __metaclass__ = plnamespace.inherit(Existing)

                other_option  = plopt("Other",
                                      doc="An option that is not in the base class.")

        Note that if 'Existing.some_option=VALUE' is overriden through the command-line,
        the value of 'New.some_option' will be VALUE unless explicitely overrode. 
        """
        META = namespace.__metaclass__
        class __metaclass__(META):
            def __new__(metacls, clsname, bases, dic):
                # Do not use plopt.optdict: the documentation, choices and other
                # property would be lost...
                for opt in plopt.iterator(namespace):
                    if not opt.getName() in dic:
                        inh_opt = copy.deepcopy(opt)
                        inh_opt.set( opt.get() )
                        dic[inh_opt.getName()] = inh_opt
                    
                #OLD: optdict = dict([ (
                #OLD:     opt.getName(), opt) for opt in plopt.iterator(namespace) ])
                #OLD: dic.update( copy.deepcopy(optdict) )
                cls = META.__new__(metacls, clsname, bases, dic)
                return cls        
        return __metaclass__
    inherit = staticmethod(inherit)
        
    class __metaclass__(type):
        def __new__(metacls, clsname, bases, dic):
            cls = type.__new__(metacls, clsname, bases, dic)
            if clsname != "plnamespace":
                context = actualContext(globals()["plnamespace"])
                context.namespaces[clsname] = cls

                for option, value in dic.iteritems():
                    if neglected_member(option, value):
                        continue
                    # Define the plopt instance
                    plopt.define(cls, option, value)
            return cls

        def __getattribute__(cls, key):
            if key.startswith('_'):
                return type.__getattribute__(cls, key)

            attr = type.__getattribute__(cls, key)
            try:
                plopt_instance = attr
                assert isinstance(plopt_instance, plopt)
                return plopt_instance.get()
            except AssertionError:
                assert inspect.ismethod(attr) or inspect.isfunction(attr)
                return attr

        def __setattr__(cls, key, value):
            if key.startswith('_') \
               or (hasattr(cls, key) and callable(getattr(cls,key))):
                type.__setattr__(cls,key,value)
            else:
                try:
                    plopt.override(cls, key, value)
                except AttributeError:
                    raise AttributeError(
                        "Namespace %s does not contain a plopt instance named %s. "
                        "One can't set a value to an undefined option."%(cls.__name__, key))

class _TmpHolder:
    def __init__(self, holder_name):
        self._name = holder_name

    def checkConsistency(self, holder):
        from plearn.pyplearn import PyPLearnError
        if self._name is None:
            assert not issubclass(holder, plnamespace)
        else:
            assert holder.__name__ == self._name

# class _TmpHolderMap(dict):
#     def __getitem__(self, key):
#         if not key in self:
#             self.__setitem__(key, [])
#         return super(ListMap, self).__getitem__(key)
    

#######  For backward compatibily: will be deprecated soon  ###################

class plargs_binder(plargs):
    class __metaclass__(plargs.__metaclass__):
        def __new__(metacls, clsname, bases, dic):            
            if clsname=="plargs_binder":
                return type.__new__(metacls, clsname, bases, dic)
            warn("plargs_binder is deprecated. Inherit directly from plargs instead.",
                 DeprecationWarning)
            return plargs.__metaclass__.__new__(metacls, clsname, bases, dic)

class plargs_namespace(plnamespace):
    class __metaclass__(plnamespace.__metaclass__):
        def __new__(metacls, clsname, bases, dic):
            if clsname=="plargs_namespace":
                return type.__new__(metacls, clsname, bases, dic)
            warn("plargs_namespace is deprecated. Inherit from plnamespace instead.",
                 DeprecationWarning)
            return plnamespace.__metaclass__.__new__(metacls, clsname, bases, dic)


#######  Unit Tests: invoked from outside  #####################################

def printCurrentContext(out=sys.stdout):
    print >>out, "Expdir:", plargs.expdir
    
    for binder in plargs.getBinders():
        print >>out, "Binder:", binder.__name__
        for opt in plopt.iterator(binder):
            print >>out, '   ',opt.getName()+':', opt
        print >>out, ''
    
    for namespace in plargs.getNamespaces():
        print >>out, "Namespace:", namespace.__name__
        for opt in plopt.iterator(namespace):
            print >>out, '   ',opt.getName()+':', opt

def test_plargs():                
    print "#######  Binders  #############################################################\n"
    class binder(plargs):
        c = "c"
        d = "d"
        e = "e"
        f = "f"

    def bCheck(attr):
        plargs_attr = getattr(plargs, attr)
        binder_attr = getattr(binder, attr)
        assert not isinstance(binder_attr, plopt)

        print "Access through plargs:", plargs_attr        
        print "Direct access:", binder_attr
        assert plargs_attr==binder_attr

        # binder_plopt = binder.getPlopt(attr)
        # print "Access to plopt:", "TO BE DONE" # binder_plopt
        print 
        

    ### Untouched
    print "+++ Untouched plarg\n"
    bCheck('c')

    ### Standard assignment
    print "+++ Standard assignment through binder\n"
    binder.d = "Youppi"
    bCheck('d')

    ### Subclass propagation
    print "+++ Setting binded option using 'dotted' key\n"
    plargs.parse("binder.e = ** E **")
    bCheck('e')

    print "#######  Namespaces  ##########################################################\n"
    class n(plnamespace):    
        namespaced = "within namespace n"

    def nCheck():
        assert not isinstance(n.namespaced, plopt)
        print "Direct access:", n.namespaced

        # n_plopt = n.getPlopt('namespaced')
        # print "Access to plopt:", "TO BE DONE" # n_plopt
        print 

    ### Untouched
    nCheck()
    
    ### Standard assignments
    print "+++ Standard assignment through namespace\n"
    n.namespaced = "WITHIN NAMESPACE n"
    nCheck()
    
    ### Subclass propagation
    plargs.parse("n.namespaced=FROM_SETATTR")
    nCheck()    

    print "#######  Contexts Management  #################################################\n"

    print "+++ Context 1"
    first_context = getCurrentContext()
    printCurrentContext()
    print 

    print "+++ Context 2"        
    second_context = createNewContext()
    plarg_defaults.algo = "classical"

    print "-- Before creation of the new 'n' plnamespace:"
    print n.namespaced
    printCurrentContext()
    print
    
    class n(plnamespace):
        namespaced = "NEW NAMESPACED ATTR"

    print "-- After creation of the new 'n' plnamespace:"
    print n.namespaced
    printCurrentContext()
    print


    print "+++ Back to Context 1"
    setCurrentContext(first_context)
    printCurrentContext()
    print 

def test_plargs_parsing():
    def readScript(*command_line):
        context_handle = createNewContext()
        
        plargs.parse(*command_line)

        class MyBinder(plargs):
            binded1 = "binded1"
            binded2 = "binded2"
            binded3 = "binded3"

        class MyNamespace(plnamespace):
            namespaced1 = "namespaced1"
            namespaced2 = "namespaced2"
            namespaced3 = "namespaced3"

        header = "Context %d"%context_handle
        header += '\n'+("="*len(header))
        print header
        printCurrentContext()
        print        
        return context_handle, header
    
    contexts = [ readScript() ]
    contexts.append(
        # list argument
        readScript(["binded1=BINDED1", "binded2=BINDED2",
                    "MyNamespace.namespaced3=NAMESPACED3"]) )
    contexts.append(
          # string arguments
          readScript("binded3=BINDED3", "MyNamespace.namespaced1=NAMESPACED1") )

    print 
    print "Reprint all contexts to ensure nothing was lost or corrupted."
    print "-------------------------------------------------------------"
    print 
    for c, header in contexts:
        setCurrentContext(c)
        print header
        printCurrentContext()
        closeCurrentContext()
        print 

    print 
    print "Misspelled command-line argument"
    print "-------------------------------------------------------------"
    print
    from plearn.pyplearn import PyPLearnError
    try:
        readScript(["bindd1=BINDED1", "binded2=BINDED2",
                    "MyNamespace.namespaced3=NAMESPACED3"])
        closeCurrentContext()        
    except PyPLearnError, err:
        print 
        print 'PyPLearnError:', err


def test_mandatory_plargs(*command_line):
    from plearn.pyplearn import PyPLearnError
    plargs.parse(*command_line)

    try:
        class MyBinder(plargs):
            mandatory = mandatory_plopt(int,
                                        doc="Some mandatory int command line argument")
        
        print MyBinder.mandatory

    except PyPLearnError, err:
        print err

def test_misspelled_plargs():
    from plearn.pyplearn import PyPLearnError

    plargs.parse("binded1=[]", "binded2=[ 2 ]", "namespace='misspelled'")

    class Binder(plargs):
        binded1 = [ 1 ]
        binded2 = plopt([], elem_type=int)

    class Namespace(plnamespace):
        namespaced = "N"

    print Binder.binded1, Binder.binded2, Namespace.namespaced

    from plearn.pyplearn import PyPLearnError
    try:
        closeCurrentContext()
    except PyPLearnError, err:
        print err
