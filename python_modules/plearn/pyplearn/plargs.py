import inspect, logging, new, re
from plearn.pyplearn.context import *
from plearn.pyplearn.pyplearn import list_cast

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
        
        # type: This keyword can and must only be used when the default
        # value is None. Otherwise, it is infered using 'type(value)'.
        assert not ("type" in kwargs and value is not None)
        if value is None:
            if "type" not in kwargs:
                raise ValueError(
                    "When a plopt value defaults to None, a valid type must be "
                    "provided using the 'type' keyword argument.")
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
            if elem_type is None and len(self.get()) > 0:
                elem_type = type(self.get()[0])
            else:
                elem_type = str
            casted = list_cast(value, elem_type)

        # Simple type cast
        else:
            casted = self._type(value)

        # Sanity checks
        self.checks(casted)
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
        if choices is not None and value in choices:
            raise ValueError(
                "Option %s should be in choices=%s"%(self._name, choices))

    def get(self):
        """Returns the current context override, if any, o/w returns the default value."""
        plopt_overrides = actualContext(self.__class__).plopt_overrides
        if self in plopt_overrides:
            return plopt_overrides[self]
        return self.__default_value

    def getBounds(self):
        minimum = self._kwargs.get("min", None)
        maximum = self._kwargs.get("max", None)
        return minimum, maximum

    def getChoices(self):
        return self._kwargs.get("choices", None)

    def getFreeChoices(self):
        return self._kwargs.get("free_choices", None)

    def getName(self):
        assert self._name != self.unnamed
        return self._name

    def getType(self):
        return self._type

    def reset(self):
        """Simply deletes any override for this plopt in the current context."""
        actualContext(self.__class__).plopt_overrides.pop(self, None)
        
    def set(self, value):
        """Sets an override for this plopt in the current context"""
        # Sanity checks
        self.checkBounds(value)
        self.checkChoices(value)
    
        # The previous didn't raise exeption, the 'value' is valid
        actualContext(self.__class__).plopt_overrides[self] = value

    #######  Static methods  ######################################################

    def buildClassContext(context):
        assert not hasattr(context, 'plopt_binders')
        context.plopt_binders = {}

        assert not hasattr(context, 'plopt_overrides')
        context.plopt_overrides = {}
    buildClassContext = staticmethod(buildClassContext)

    def define(holder, option, value):
        """Typical pattern to set a plopt instance member in 'holder' for the first time."""
        context = actualContext(plopt)

        if issubclass(holder, plargs):
            # A script should not contain two options of the same name
            if option in context.plopt_binders:
                raise KeyError(
                    "A script should not contain two options of the same name. "
                    "Clashing definition of plarg '%s' in '%s' and '%s'"%
                    (option,context.plopt_binders[option].__name__,holder.__name__) )

            # Keep a pointer to the binder in which the option is defined
            context.plopt_binders[option] = holder

        # Enforce all 'holder' members to be (named) plopt instances
        if isinstance(value, plopt):
            value._name = option
        else:
            value = plopt(value, name=option)

        # Acutally sets the plopt in the holder
        type.__setattr__(holder, option, value)
    define = staticmethod(define)

    def iterator(holder):
        """Returns an iterator over the plopt instances contained in the I{holder}"""
        return iter([ value
                      for option, value in holder.__dict__.iteritems()
                      if isinstance(value, plopt) ])
    iterator = staticmethod(iterator)

    def override(holder, option, value):
        """Typical pattern to override the value of an existing plopt instance."""
        plopt_instance = type.__getattribute__(holder, option)
        plopt_instance.set(value)
    override = staticmethod(override)

class plargs(object):
    """Values read from or expected for PLearn command-line variables.

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

    Those command-line arguments are widely refered as I{plargs}, on
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
    
    Finally, B{note that} the value of plargs.expdir is generated automatically and
    B{can not} be assigned a default value through binders. This behaviour
    aims to standardize the naming of experiment directories. For debugging
    purpose, however, one may provide on command-line an override to
    plargs.expdir value. Otherwise, one can also provide the I{expdir_root}
    command-line argument as a (relative) path where the expdir should be
    found, e.g.::

        prompt %> plearn my_script.pyplearn expdir_root=Debug

    would cause experiment to use the expdir::

        Debug/expdir_2006_05_10_07_58_10
        
    for instance.
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

    def getNamespaces():
        """Returns a list of all namespaces in the current context."""
        context = actualContext(plargs)
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
        """Parses a list of argument strings.

        ...
        """
        if len(args) == 1 and isinstance(args[0], list):
            args = args[0]

        context = actualContext(plargs)
        for statement in args:
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
            except (ValueError, TypeError):
                holder_name = None
                holder = context.plopt_binders.get(option)

            if holder is None:
                msg = "Unkown option '%s'"%option
                if holder_name:                    
                    msg = "No namespace or binder named '%s'. %s"%(holder_name, msg)                    
                raise KeyError("Parsing '%s': %s"%(statement, msg))
                    
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
                    if option.startswith('_'):
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
            if key.startswith('_'):
                return type.__getattribute__(cls, key)

            plargs = cls
            if cls.__name__ != "plargs":
                plargs = globals()['plargs']

            if key == "expdir":
                return actualContext(plargs).getExpdir()

            holder = cls
            if cls is plargs:
                try:
                    holder = actualContext(plargs).plopt_binders[key]
                except (AttributeError, KeyError):
                    # Otherwise
                    try:
                        return type.__getattribute__(cls, key)
                    except:
                        raise AttributeError(
                            "Unknown option '%s'. Is it namespaced?"%key)

            try:
                plopt_instance = type.__getattribute__(holder, key)
                assert isinstance(plopt_instance, plopt)
                return plopt_instance.get()

            # Unknown option
            except AttributeError, err:
                raise AttributeError(
                    "Unknown option '%s' in '%s' (%s)"%(key, cls.__name__, err))

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
        setattr(self._getBinder(), option, value)
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
    
    class __metaclass__(type):
        def __new__(metacls, clsname, bases, dic):
            cls = type.__new__(metacls, clsname, bases, dic)
            if clsname != "plnamespace":
                context = actualContext(globals()["plnamespace"])
                context.namespaces[clsname] = cls

                for option, value in dic.iteritems():
                    if option.startswith('_'):
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
            try:
                plopt.override(cls, key, value)
            except AttributeError:
                raise AttributeError(
                    "Namespace %s does not contain a plopt instance named %s. "
                    "One can't set a value to an undefined option."%(cls.__name__, key))

if __name__ == "__main__":
                
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

    def printContext():
        print "Expdir:", plargs.expdir
        
        for binder in plargs.getBinders():
            print "Binder:", binder.__name__
            for opt in plopt.iterator(binder):
                print '   ',opt.getName()+':', opt
            print
        
        for namespace in plargs.getNamespaces():
            print "Namespace:", namespace.__name__
            for opt in plopt.iterator(namespace):
                print '   ',opt.getName()+':', opt
        
    print "+++ Context 1"
    first_context = getCurrentContext()
    printContext()
    print 

    print "+++ Context 2"        
    second_context = createNewContext()
    plarg_defaults.algo = "classical"

    print "-- Before creation of the new 'n' plnamespace:"
    print n.namespaced
    printContext()
    print
    
    class n(plnamespace):
        namespaced = "NEW NAMESPACED ATTR"

    print "-- After creation of the new 'n' plnamespace:"
    print n.namespaced
    printContext()
    print


    print "+++ Back to Context 1"
    setCurrentContext(first_context)
    printContext()
    print 
