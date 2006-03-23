"""Very core classes of the pyplearn mechanism.

To be done:

  - PLOptionDict should be generalized to OptionDict. This OptionDict class
    should understand inner classes derived from OptionBase, e.g.

    class OptionDict(dict): # Not sure for dict yet...
        pass

    class PyPLearnObject(OptionDict)
        OptionType = PLOption

    class Other(PyPLearnObject):
        class KWArg(OptionBase): pass
        
    and the MetaOptionDict mechanism should be able to find and SEQUENTIALLY
    (the mechanism is actually recursive) that Other manages PLOption and KWArg
    instances.    
"""
import copy, inspect, re

deprecated_methods = [ 'allow_unexpected_options', ### No more a classmethod
                       'get_members',
                       "option_names", 
                       'option_pairs',
                       'to_list' ### In PyPLearnList
                       ]

# Since those methods were added to PyPLearnObject (or its subclass) using
# the lower_case_with_underscores convention, these are prone to clashing
# with plearn options. An explicit check is made to avoid this
# eventuality. This should be fixed whenever possible.
_clash_prone_methods = [ 'allow_unexpected_options',
                         'instances',
                         'classname',
                         'plearn_repr' ]

def checkForNameClashes(key, value):
    if key in _clash_prone_methods and not callable(value):
        PyPLearnError("It seems you are trying to set an PLOption %s "
                      "which clashes with the %s internal method. "
                      "Contact support.")

def inherited_options(clsinstance, OptionType):
    inhoptions = []
    for cls in clsinstance.__mro__[1:]:
        if cls is object:
            continue

        MetaClass = clsinstance.__metaclass__
        options_slot = MetaClass._options_slot_%(cls.__name__,OptionType.__name__)
        try:
            inhoptions.extend( cls.__dict__[options_slot] )
        except KeyError, kerr:
            pass
    return inhoptions

def class_options(clsinstance, OptionType):
    MetaClass = clsinstance.__metaclass__
    options_slot = MetaClass._options_slot_%(clsinstance.__name__, OptionType.__name__)
    class_options = copy.deepcopy(clsinstance.__dict__[options_slot])       
    return class_options+inherited_options(clsinstance, OptionType)

def non_option_class_variable(clsinstance, variable_name):
    # Class variable must have been defined along with the class
    if not hasattr(clsinstance, variable_name):
        return False
    
    MetaClass = clsinstance.__metaclass__
    candidate_slot = re.compile(MetaClass._options_slot_%(clsinstance.__name__, ".+"))
    for attr_name in clsinstance.__dict__:
        if candidate_slot.search(attr_name):
            options_slot = getattr(clsinstance, attr_name)
            if variable_name in options_slot:
                return False
    return True    

def init_options(instance, OptionType, **overrides):        
    for optname in class_options(instance.__class__, OptionType):
        # There is no need to manage options that are overriden, hence,
        # for sake of efficiency, we don't
        if not optname in overrides:
            optval = getattr(instance,optname)

            # Even if the list contains only option names, if the
            # option was inherited it will already have been expended
            if isinstance(optval, OptionType):
                setattr(instance, optname, optval())
    

class OptionBase:
    __option_id = 0    
    def __init__(self, value, *args, **kwargs):
        assert not isinstance(value, OptionBase)
        self.__class__.__option_id += 1
        self._id = self.__option_id

        if ( inspect.ismethod(value) or inspect.isfunction(value) 
             or inspect.isroutine(value) or inspect.isclass(value) ):                 
            self._callable = value
            self._args     = args
            self._kwargs   = kwargs

        else:
            assert len(args) == 0 and len(kwargs) == 0
            self._callable = copy.deepcopy
            self._args     = [ value ]
            self._kwargs   = kwargs

    def __call__(self):
        return self._callable(*self._args, **self._kwargs)

    def __cmp__(self, opt2):
        assert isinstance(opt2, self.__class__), \
            "__cmp__(%s, %s): the later argument is not an instance of %s" \
            % (self, opt2, self.__class__.__name__)
        return cmp(self._id, opt2._id)

    def __str__(self):
        return "%s(\n    "%self.__class__.__name__ +\
            "\n    ".join([\
            "_id=%d"%self._id,
            "_callable=%s"%self._callable,
            "_args=%s"%str(self._args),
            "_kwargs=%s"%str(self._kwargs)               
            ]) + ")" 

def OptionDictMetaClass(OptionType, SuperMeta=type):
    """Generates a metaclass MetaOptionDict handling OptionBase-like options.

    @param OptionType: The type of the options to be managed by this metaclass.

    @param SuperMeta: The class this metaclass must inherit from. Not that
    super isn't used everywhere in the class definition. Direct reference
    to \I{type} is sometimes used instead. The reason is simple: the
    I{SuperMeta} parameter is meant to allow one to derive a
    I{MetaClassOptionDict} from another. If super was called on such a
    subclass, redundancy would occur.

    PLEASE AVOID MULTIPLE INHERITANCE WITH THIS CLASS...
    """
    assert issubclass(OptionType, OptionBase), OptionType.__class__
    assert SuperMeta is type or SuperMeta._options_slot_

    class _OptionDictMetaClass( SuperMeta ):
        _options_slot_ = '_%s__options__%s_'
        def __new__(metacls, clsname, bases, dic):
            newcls = SuperMeta.__new__(metacls, clsname, bases, dic)
    
            inherited = inherited_options(newcls, OptionType)
            options_slot = metacls._options_slot_%(clsname,OptionType.__name__)
            if options_slot not in dic:
                reversed_option_pairs = [
                    (optval,optname)
                    for optname,optval in dic.iteritems() 
                    if isinstance(optval, OptionType) and optname not in inherited
                    ]
                reversed_option_pairs.sort()
                setattr(newcls, options_slot, [ optname for optval,optname in reversed_option_pairs ])
    
            return newcls

        # Direct reference to type (see docstring)
        def __getattribute__(self, name):
            value = type.__getattribute__(self, name)
            if isinstance(value, OptionBase):
                return value()
            return value
            
        # Direct reference to type (see docstring)
        def __setattr__(self, name, value):
            checkForNameClashes(name, value)

            # Is an option: check that it is wrapped and add it to the list if needed
            if not name.startswith('_') \
                and not non_option_class_variable(self, name):

                # Ensure that it is wrapped...
                if isinstance(value, OptionBase):
                    ActualOptionType = value.__class__
                else:
                    value = OptionType(value)
                    ActualOptionType = OptionType

                # Retreive the appropriate slot                
                options_slot = _OptionDictMetaClass._options_slot_%(self.__name__,ActualOptionType.__name__)
                option_list = self.__dict__[options_slot]

                # ... and add the option's name to the list if needed
                if name not in option_list:
                    option_list.append(name)
    
            # Call inherited __setattr__        
            type.__setattr__(self, name, value)
            
        # Direct reference to type (see docstring)
        def __delattr__(self, name):
            attr = type.__getattribute__(self, name)
            if not name.startswith('_') and isinstance(attr, OptionBase):
                options_slot = _OptionDictMetaClass._options_slot_%(self.__name__,attr.__class__.__name__)
                self.__dict__[options_slot].remove(name)

            # Actual deletion
            type.__delattr__(self, name)            

    return _OptionDictMetaClass
