import logging, re
from plearn.pyplearn.context import *

if "plargs" in globals():
    logging.info("Using new style plargs")
    del plargs
    del plarg_defaults

def list_cast(slist, elem_cast):
    ## Potentially strip left-hand [ and right-hand ] if it's a string
    if isinstance(slist,str):
        slist = slist.lstrip(' [').rstrip(' ]')
        return [ elem_cast(e) for e in slist.split(",") ]
    elif isinstance(s,list):
        return [ elem_cast(e) for e in slist ]
    else:
        raise ValueError, "Cannot cast '%s' into a list", str(slist)

class plopt(object):
    """Typed command-line options with constraints for PLearn.
    
    This class provides support for default values, strong type checking,
    conversion from a string representation, documentation, and constraints
    on the possible values that the option can take.  The syntax to follow
    is along the lines of:
    
    class MyOptions( plargs_namespace ):
        plot_stuff  = plopt(True, doc='Whether a cute graph should be plotted')
        K           = plopt(10, min=5, max=25, doc='Number of neighbors to consider')
        method      = plopt('classical', choices=['new-kind','classical','crazy'])
        lags        = plopt([1,22,252], doc='Lags to incorporate as inputs, in days')
        weight_cols = plopt([], elem_type=int)

    The supported keywords are:

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
    """
    unnamed = "### UNNAMED ###"

    def __init__(self, value, **kwargs):
        self._name  = kwargs.pop("name", self.unnamed)
        self._doc   = kwargs.pop("doc", '')

        assert not ("type" in kwargs and value is not None)
        if value is None:
            if "type" not in kwargs:
                raise ValueError(
                    "When a plopt value defaults to None, a valid type must be "
                    "provided using the 'type' keyword argument.")
            self._type = kwargs.pop("type")
        else:
            self._type = type(value)

        self._kwargs = kwargs

        # Actually sets the value of the plopt instance
        # self.set(value)

        # Sanity checks
        self.checkBounds(value)
        self.checkChoices(value)
        self.__default_value = value

    def __str__(self):
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
        minimum = self._kwargs.get("min", None)
        if minimum is not None and value < minimum:
            raise ValueError("Option %s (=%s) should greater than %s"
                             %(self._name, repr(value), repr(minimum)))

        maximum = self._kwargs.get("max", None)
        if maximum is not None and value > maximum:
            raise ValueError("Option %s (=%s) should lower than %s"
                             %(self._name, repr(value), repr(minimum)))

    def checkChoices(self, value):
        choices = self._kwargs.get("choices", None)
        if choices is not None and value in choices:
            raise ValueError(
                "Option %s should be in choices=%s"%(self._name, choices))

    def get(self):
        plopt_overrides = actualContext().plopt_overrides
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
        actualContext().plopt_overrides.pop(self, None)
        
    def set(self, value):
        # Sanity checks
        self.checkBounds(value)
        self.checkChoices(value)
    
        # The previous didn't raise exeption, the value is coherent
        actualContext().plopt_overrides[self] = value

    #######  Static methods  ######################################################

    def define(holder, option, value):
        # Enforce all 'holder' members to be (named) plopt instances
        if isinstance(value, plopt):
            raise
            value._name = option
        else:
            type.__setattr__(holder, option, plopt(value, name=option))

        # Keep a pointer to the subclass defining the option
        actualContext().plopt_holders[option] = holder
    define = staticmethod(define)

    def iterator(holder):
        return iter([ value
                      for option, value in holder.__dict__.iteritems()
                      if isinstance(value, plopt) ])
    iterator = staticmethod(iterator)

    def override(holder, option, value):
        plopt_instance = type.__getattribute__(holder, option)
        plopt_instance.set(value)
    override = staticmethod(override)    

class plargs(object):
    _extensible_ = False

    def getBinders():
        context = actualContext()
        binder_names = context.binders.keys()
        binder_names.sort()

        binders = []
        for bname in binder_names:
            if bname != "plargs":
                binders.append(context.binders[bname])        
        return binders
    getBinders = staticmethod(getBinders)

    def getNamespaces():
        return actualContext().namespaces.values()
    getNamespaces = staticmethod(getNamespaces)
    
    def parse(*args):
        if len(args) == 1 and isinstance(args[0], list):
            args = args[0]

        context = actualContext()
        for statement in args:
            option, value = statement.split('=', 1)
            option = option.strip()
            value  = value.strip()
            
            try:
                holder_name, option = option.split('.')
                holder = context.namespaces.get(holder_name, None)
                if holder is None:
                    holder = context.binders.get(holder_name, None)
            except TypeError:
                holder = context.plopt_holders.get(option, None)

            if holder is None:
                raise KeyError("Parsing '%s': No namespace or binder named '%s'"
                               %(statement, holder_name))
                    
            setattr(holder, option, value)
    parse = staticmethod(parse)
    
    class __metaclass__(type):
        def __new__(metacls, clsname, bases, dic):
            cls = type.__new__(metacls, clsname, bases, dic)

            context = actualContext()
    
            # Keep track of binder subclass
            context.binders[clsname] = cls

            # Introspection of the subclasses
            plargs = context.binders["plargs"]
            if cls is not plargs:
                for option, value in dic.iteritems():
                    if option.startswith('_'):
                        continue

                    # A script should not contain two options of the same name
                    if option in context.plopt_holders:
                        raise KeyError(
                            "A script should not contain two options of the same name. "
                            "Clashing definition of plarg '%s' in '%s' and '%s'"%
                            (option,clsname,context.plopt_holders[option]) )

                    # Define the plopt instance
                    plopt.define(cls, option, value)

            return cls

        def __setattr__(cls, option, value):
            context = actualContext()
            plargs  = context.binders["plargs"]
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

            holder = cls
            plargs = actualContext().binders["plargs"]
            if cls is plargs:
                try:
                    holder = actualContext().plopt_holders[key]
                except KeyError:
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
class plarg_defaults(plargs):
    _extensible_ = True

class plnamespace: 
    class __metaclass__(type):
        def __new__(metacls, clsname, bases, dic):
            cls = type.__new__(metacls, clsname, bases, dic)
            if clsname != "plnamespace":
                context = actualContext()
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

            plopt_instance = type.__getattribute__(cls, key)
            assert isinstance(plopt_instance, plopt)
            return plopt_instance.get()

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
        print "Access to plopt:", "TO BE DONE" # binder_plopt
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

    print "#######  Namespaces  ##########################################################"
    class n(plnamespace):    
        namespaced = "within namespace n"

    def nCheck():
        assert not isinstance(n.namespaced, plopt)
        print "Direct access:", n.namespaced

        # n_plopt = n.getPlopt('namespaced')
        print "Access to plopt:", "TO BE DONE" # n_plopt
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

    print "#######  Internal  ############################################################"        
    def printMap(map):
        keys = map.keys()
        keys.sort()
        for k in keys:
            print '   ',k,':',repr(map[k])
    
    print "\n*** PLArgs"    
    printMap(plargs.__dict__)
        
    print "\n*** Binder 'b'"
    printMap(binder.__dict__)
    
    print "\n*** Namespace 'n'"
    printMap(n.__dict__)

    print "#######  Contexts Management  #################################################"

    def printContext():
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
