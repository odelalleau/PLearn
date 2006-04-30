import logging, re

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
    def __init__(value, **kwargs):
        self._value = value
        self._name  = kwargs.pop("name")
        self._doc   = kwargs.pop("doc", self._name)

        assert not ("type" in kwargs and value is not None)
        if value is None:
            self._type = kwargs.pop("type", lambda val: val)
        else:
            self._type = type(value)

        self._kwargs = kwargs

        # Sanity checks
        self.checks(value)

    def cast(self, value):
        casted = None
        
        # Special string to bool treatment
        if isinstance(self._value,bool) and isinstance(value, str):
            if value == "True":
                casted = True
            elif value == "False":
                casted = False
            else:
                raise ValueError("Trying to set value %s to bool-typed option %s"
                                 %(value, self._name))

        # Special treatment for list option
        elif isinstance(self._value,list):
            elem_type = self._kwargs.pop("elem_type", None) 
            if elem_type is None and len(self._value) > 0:
                elem_type = type(self._value[0])
            casted = list_cast(value, elem_type)

        # Simple type cast
        else:
            casted = self._type(value)

        # Sanity checks
        self.checks(casted)
        return casted
    
    def checks(self, value):
        self.checkBounds(value)
        self.checkChoices(value)

    def checkBounds(self, value):
        minimum = self._kwargs.get("min", None)
        if minimum is not None and value < minimum:
            raise ValueError("Option %s (=%s) should greater then %s"
                             %(self._name, repr(value), repr(minimum)))

        maximum = self._kwargs.get("min", None)
        if maximum is not None and value > maximum:
            raise ValueError("Option %s (=%s) should lower then %s"
                             %(self._name, repr(value), repr(minimum)))

    def checkChoices(self, value):
        choices = self._kwargs.get("choices", None)
        if choices is not None and value in choices:
            raise ValueError(
                "Option %s should be in choices=%s"%(self._name, choices))

class plnamespace: 
    class __metaclass__(type):
        def __init__(cls, clsname, bases, dic):            
            type.__init__(cls, clsname, bases, dic)
            # Namespaces are managed as public inner classes of the plargs
            # class
            if clsname != "plnamespace":
                plargs.addNamespace(cls)

        def __getattribute__(cls, key):
            if key.startswith('_'):
                return type.__getattribute__(cls, key)

            # First try to access the override, if any, stored in plargs
            try:
                return type.__getattribute__(plargs, '.'.join([cls.__name__, key]) )

            # Otherwise return the default value
            except AttributeError:
                return type.__getattribute__(cls, key)

        def __setattr__(cls, key, value):
            setattr(plargs, '.'.join([cls.__name__, key]), value)
        
class plargs(object):
    _binder_ = {}
    _namespace_ = {}
    _definition_ = {}

    def addNamespace(namespace):
        assert not hasattr(plargs, namespace.__name__)
        plargs._namespace_[namespace.__name__] = namespace
        type.__setattr__(plargs, namespace.__name__, namespace)
    addNamespace = staticmethod(addNamespace)

    def clearOverrides():
        attributes = plargs.__dict__.keys()        
        for attr_name in attributes:
            if attr_name in plargs._definition_:
                delattr(plargs, attr_name)

            else:
                for namespace in plargs._namespace_:
                    pattern = re.compile("%s\..*"%namespace)
                    if pattern.match(attr_name):
                        delattr(plargs, attr_name)        
    clearOverrides = staticmethod(clearOverrides)
    
    class __metaclass__(type):
        def __init__(cls, clsname, bases, dic):
            type.__init__(cls, clsname, bases, dic)

            # Keep track of binder subclass
            cls._binder_[clsname] = cls

            # Introspection of the subclasses
            plargs = cls._binder_["plargs"]
            if cls is not plargs:
                for arg in dic:
                    # A script should not contain two options of the same name
                    if not arg.startswith('_') and hasattr(plargs, arg):
                        raise KeyError(
                            "A script should not contain two options of the same name. "
                            "Clashing definition of plarg '%s' in '%s'"%(arg,clsname) )

                    # Namespaces should not be encountered here
                    try:
                        assert not issubclass(dic[arg], plnamespace)
                    except TypeError: pass # value is not a class

                    # Set the default value in plargs and remember where it was taken from 
                    #type.__setattr__(plargs, arg, dic[arg])
                    #type.__setattr__(plargs, "_def_%s_"%arg, cls)
                    plargs._definition_[arg] = cls

        def __setattr__(cls, dotted_key, value):
            key = dotted_key.split('.', 1)
            plargs = cls._binder_["plargs"]

            # Direct access
            if len(key) == 1:
                try:
                    assert not issubclass(value, plnamespace)
                except TypeError: pass # value is not a class

                # Set the value in plargs only... The definition should never be spoiled.
                type.__setattr__(plargs, key[0], value)

            elif len(key) == 2:
                # plarg binders 
                if key[0] in cls._binder_:
                    #setattr(cls._binder_[key[0]], key[1], value)
                    setattr(plargs, key[1], value)

                # plnamespace's
                else:
                    try:
                        namespace = getattr(cls, key[0])
                        assert issubclass(namespace, plnamespace)
                        # setattr(namespace, key[1], value)
                        type.__setattr__(plargs, dotted_key, value)
                    except AttributeError:
                        raise ValueError(
                            "Trying to set '%s' on '%s' which is neither a plnamespace, "
                            "neither a plargs subclass."%(key[1], key[0]))
            else:
                raise ValueError(key)

        def __getattribute__(cls, key):
            if key.startswith('_'):
                return type.__getattribute__(cls, key)
            plargs = cls._binder_["plargs"]

            # First try to access the override, if any, stored in plargs
            try:
                return type.__getattribute__(plargs, key)

            # No override: Return the default value
            except AttributeError:
                
                # Is the call for a binded plarg made directly on plargs?
                if cls is plargs:                    
                    return type.__getattribute__(plargs._definition_[key], key)

                # Otherwise directly access member
                else:
                    return type.__getattribute__(cls, key)

if __name__ == "__main__":
                
    print "#######  Binders  #############################################################"
    class binder(plargs):
        c = "c"
        d = "d"
        e = "e"
        f = "f"

    ### Untouched
    print "+++ Untouched plarg\n"
    print "Access through plargs:", plargs.c
    print "Access through binder:", binder.c
    assert plargs.c==binder.c
    print 

    ### Subclass propagation
    print "+++ Setting binded option using 'dotted' key\n"
    setattr(plargs, "binder.d", "** D **")    
    print "Access through plargs:", plargs.d
    print "Access through binder:", binder.d
    assert plargs.d==binder.d
    print 

    ### Standard assignments
    print "+++ Standard assignment through binder\n"
    binder.e = "Youppi"
    print "Access through plargs:", plargs.e
    print "Access through binder:", binder.e
    assert plargs.e==binder.e
    print 
    
    print "+++ Standard assignment through plargs\n"
    plargs.f = "Another success!" 
    print "Access through plargs:", plargs.f
    print "Access through binder:", binder.f
    assert plargs.f==binder.f
    print 

    print "#######  Namespaces  ##########################################################"
    class n(plnamespace):    
        namespaced = "within namespace n"

    def nCheck():
        print "Access through plargs:", plargs.n.namespaced
        print "Access through namespace:", n.namespaced
        assert plargs.n.namespaced==n.namespaced
        print 
        

    ### Untouched
    assert plargs.n.namespaced==n.namespaced
    nCheck()
    
    ### Subclass propagation
    setattr(plargs, "n.namespaced", "FROM SETATTR")
    nCheck()
    
    ### Standard assignments
    print "+++ Standard assignment through namespace\n"
    n.namespaced = "WITHIN NAMESPACE n"
    nCheck()
    
    print "+++ Standard assignment through plargs\n"
    plargs.n.namespaced = "THROUGH PLARGS" 
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

    print "#######  Cleared plargs  ######################################################"
    plargs.clearOverrides()

    print "\n*** Cleared plargs"
    printMap(plargs.__dict__)
    
