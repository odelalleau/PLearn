#!/usr/bin/env python
import copy, inspect, operator
from plearn.pyplearn.plearn_repr import plearn_repr, python_repr, format_list_elements
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
_clash_prone_methods = [ 'inherited_options',
                         'class_options',
                         'allow_unexpected_options',
                         'instances',
                         'classname',
                         'plearn_repr' ]

def _checkForNameClashes(key, value):
    if key in _clash_prone_methods and not callable(value):
        PyPLearnError("It seems you are trying to set an PLOption %s "
                      "which clashes with the %s internal method. "
                      "Contact support.")
        

#
#  Classes
#
class PLOptionWarning( Warning ): pass
class PLOptionError( AttributeError ): pass

class PLOption:
    __option_id = 0
    def __init__(self, value, *args, **kwargs):
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
        assert isinstance(opt2, PLOption)
        return cmp(self._id, opt2._id)

class MetaPLOptionDict( type ):
    """Manages the list of option names associated to a class.

    Note that this metaclass provides classes using it with the class
    method class_options.
    """
    __options_slot = '_%s__class_options'
    def __new__( metacls, clsname, bases, dic ):
        newcls = type.__new__( metacls, clsname, bases, dic )

        inherited = newcls.inherited_options()
        options_slot = metacls.__options_slot%clsname
        if options_slot not in dic:
            reversed_option_pairs = [
                (optval,optname)
                for optname,optval in dic.iteritems() 
                if isinstance(optval, PLOption) and optname not in inherited
                ]
            reversed_option_pairs.sort()
            setattr(newcls, options_slot, [ optname for optval,optname in reversed_option_pairs ])

        return newcls

    ## HERE: AND WHAT IF THE OPTION ALREADY EXISTS???
    def __getattribute__(self, name):
        value = type.__getattribute__(self, name)
        if isinstance(value, PLOption):
            return value()
        return value
        
    def __setattr__(self, name, value):
        _checkForNameClashes(name, value)
        super(MetaPLOptionDict, self).__setattr__(name, value)
        if not name.startswith('_'):
            options_slot = MetaPLOptionDict.__options_slot%self.__name__
            option_list = self.__dict__[options_slot]
            if name not in option_list:
                option_list.append(name)
            #self.__class_options.append(name)            

    def __delattr__(self, name):
        super(MetaPLOptionDict, self).__delattr__(name)
        if not name.startswith('_'):
            options_slot = MetaPLOptionDict.__options_slot%self.__name__
            self.__dict__[options_slot].remove(name)
            #self.__class_options.remove(name)

    def inherited_options(self):
        inhoptions = []
        for cls in self.__mro__[1:]:
            if cls is object:
                continue
            
            options_slot = MetaPLOptionDict.__options_slot%cls.__name__
            try:
                inhoptions.extend( cls.__dict__[options_slot] )
            except KeyError, kerr:
                indent = " "*8
                raise RuntimeError( "In MetaPLOptionDict: %s<-%s \n (mro = %s) \n %s"
                                    % (cls.__name__, self.__class__.__name__,
                                       ("\n"+indent).join([ KLS.__name__ for KLS in self.__mro__]),
                                       str(kerr)))
                                    
        return inhoptions

    def class_options(self):
        options_slot = MetaPLOptionDict.__options_slot%self.__name__
        class_options = copy.deepcopy(self.__dict__[options_slot])       
        return class_options+self.inherited_options()

class PLOptionDict( object ):
    __metaclass__ = MetaPLOptionDict
    
    def class_options(cls):
        """Forwarding call so that class_options() can be called on an instance."""
        return MetaPLOptionDict.class_options(cls)
    class_options = classmethod( class_options )

    def __init__(self, **overrides):
        self.__instance_option_names = self.class_options()
        for optname in self.__instance_option_names:
            # There is no need to manage options that are overriden, hence,
            # for sake of efficiency, we don't
            if not optname in overrides:
                optval = getattr(self,optname)

                # Even if the list contains only option names, if the
                # option was inherited it will already have been expended
                if isinstance(optval, PLOption):
                    setattr(self, optname, optval())

        keys = overrides.keys()
        keys.sort()
        for key in keys:
            setattr(self, key, overrides[key])

    def __addoption__(self, key):
        if key.startswith('_') or hasattr(self, key):
            return
        
        if self.allow_unexpected_options():
            self.__instance_option_names.append( key )
        else:
            raise PLOptionError("Disallowed attribute %s"%key)
            
    def __setattr__(self, key, value):
        _checkForNameClashes(key, value)
        self.__addoption__(key)
        super(PLOptionDict, self).__setattr__(key, value)

    def __delattr__(self, key):
        try:
            super(PLOptionDict, self).__delattr__(key)
        except AttributeError, err:
            # May be erasing an option defined at class level: we will only
            # remove it from option list
            pass
        
        if key in self.__instance_option_names:
            self.__instance_option_names.remove(key)


    def allow_unexpected_options(self):
        """May overrides contain undefined options?

        The __init__ method accepts any keyword arguments. If a given
        keyword is not class variable, the default behavior is to consider
        that keyword argument as being an extra option to add to the
        instance::

            class Foo( PLOptionDict ): 
                foo = "the foo option"

            f = Foo( bar = "the bar option" )

            print f.foo         ## prints "the foo option"
            print f.bar         ## prints "the bar option"

        To change that behavior, one may simply override this method and return False::

            class FooNoBar( PyPLearnObject ): 
                foo = "the foo option"
                def allow_unexpected_options(self): return False 

            f = FooNoBar( bar = "the bar option" )
            ## PLOptionError: Trying to set undefined options {'bar': 'the bar option'} on a FooNoBar
        """
        return True

    def getOptionNames(self):
        return copy.deepcopy(self.__instance_option_names)

    ### Behaviours Emulating dict    ###########################################

    def __len__(self):
        return len( self.getOptionNames() )

    def __iter__(self):
        return self.iterkeys()

    def __deepcopy__(self, memo):
        options = copy.deepcopy(dict([ (optname,getattr(self, optname)) for optname in self.iterkeys() ]), memo)
        return self.__class__(**options)

    def iterkeys(self):
        return iter(self.__instance_option_names)
            
    def itervalues(self):
        return iter([ getattr(self, optname) for optname in self.iterkeys() ])
    
    def iteritems(self):
        return iter([ (optname, getattr(self, optname)) for optname in self.iterkeys() ])

class PyPLearnObject( PLOptionDict ):
    """A class from which to derive python objects that emulate PLearn ones.

    This class provides any of its instances with a plearn_repr() method
    recognized by PyPLearn's plearn_repr mechanism. The plearn
    representation is defined to be::

        Classname(
            plearn_option1 = option_value1,
            plearn_option2 = option_value2,
            ...
            plearn_optionN = option_valueN
            )

    where <Classname> is the name you give to the PyPLearnObject's
    subclass. The L{PLearn} options are considered to be all attributes
    whose names do not start with an underscore. Those are said public,
    while any attribute starting with at least one underscore is considered
    internal (protected '_' or private '__').

    Protected and private attributes are not affected by the option mecanisms.
    """
    __instances = []

    #
    #  Classmethods:
    #   See
    #    U{http://www.python.org/doc/2.3.4/lib/built-in-funcs.html}
    #    for an introduction to the classmethod concept.
    #
    def classname( cls ):
        """Classmethod to access the class name. 

        @returns: The (instance) class' name as a string. B{Note that} the
        value returned by that method is the one used in plearn_repr().
        """
        return cls.__name__
    classname = classmethod(classname)

    def instances( cls ):
        """Returns a list of this I{cls} instances.

        Notes that if the class has subclass, instances of the subclasses
        will be returned too.
        """
        ilist = []
        for instance in cls.__instances:
            if isinstance( instance, cls ):
                ilist.append(instance)
        return ilist
    instances = classmethod(instances)

    #
    #  PyPLearnObject's metaclass
    #
    _subclass_filter = classmethod( lambda cls: not cls.__name__.startswith('_') )
    class __metaclass__( MetaPLOptionDict ):
        """Implements some support mecanisms.

        This metaclass defaults right operators to their left counterpart
        if it exists. It also manages the _subclasses option if any;
        subclasses created are added (name -> class object) to the
        dictionnary if _subclass_filter returns True (default version
        neglect classes whose name starts with an underscore.
        """        
        _rop_names = [
        '__radd__', '__rsub__', '__rmul__', '__rdiv__',
        '__rtruediv__', '__rfloordiv__', '__rmod__',
        '__rdivmod__', '__rpow__', '__rlshift__',
        '__rrshift__', '__rand__', '__rxor__', '__ror__'
        ]
        
        def __init__(cls, name, bases, dict):
            super(type, cls).__init__(name, bases, dict) 
            for rop_name in cls._rop_names:
                lop_name = rop_name[0:2] + rop_name[3:]
                if ( not dict.has_key(rop_name)
                     and dict.has_key(lop_name) ):
                    setattr( cls, rop_name, dict[lop_name] )
                    
            if hasattr(cls, '_subclasses'):
                assert isinstance( cls._subclasses, type(dict) ), type( cls._subclasses )
                if cls._subclass_filter( ):
                    cls._subclasses[name] = cls

    def __init__(self, **overrides):
        super(PyPLearnObject, self).__init__(**overrides)
        self.__instances.append( self )
        self.__serial_number = len(self.__instances)

    def __str__( self ):
        """Calls plearn_repr global function over itself.""" 
        # It is most important no to call this instance's plearn_repr
        # method!!! Indeed, if we did so, we'd neglect to add the current
        # instance in the representations map...
        return plearn_repr( self, indent_level = 0 )

    def __repr__( self ):
        # The exact protocol is not fixed yet. Repr should be as
        # plearn_repr but in strict Python... This means that not only
        # options should be printed but all instance variables. No PLearn
        # references should be printed.
        return python_repr( self, indent_level = 0 )

    def _optionFormat(self, option_pair, indent_level, inner_repr):
        k, v = option_pair
        try:
            return '%s = %s' % ( k, inner_repr(v, indent_level+1) )
        except Exception, e:
            raise PLOptionError( 'Option %s in %s instance caused an error in %s:\n  %s: %s'
                                 % ( k, self.classname(), inner_repr.__name__,
                                     e.__class__.__name__, str(e) ) )                 
        
    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        """PLearn representation of this python object.

        Are considered as options any 'public' instance attributes.
        """
        options = format_list_elements(
            [ (optname,optval) for (optname,optval) in self.iteritems() ],
            lambda opt_pair: self._optionFormat(opt_pair, indent_level, inner_repr),
            indent_level+1
            )
        return "%s(%s)" % (self.classname(), options)
        # def elem_format( elem ):
        #     k, v = elem
        #     try:
        #         return '%s = %s' % ( k, inner_repr(v, indent_level+1) )
        #     except Exception, e:
        #         raise PLOptionError( 'Option %s in %s instance caused an error in %s:\n  %s: %s'
        #                              % ( k, self.classname(), inner_repr.__name__,
        #                                  e.__class__.__name__, str(e) ) ) 
        # 
        # return "%s(%s)" % ( self.classname(),
        #                     format_list_elements([ (optname,optval) for (optname,optval) in self.iteritems() ],
        #                                          elem_format, indent_level+1 ) )

    def _serial_number(self):
        return self.__serial_number

class PyPLearnList( PyPLearnObject ):
    """Emulates a TVec of PyPLearnObject.
    
    This class is to be used whenever the attributes must be considered
    as the elements of a list. Hence, the plearn_repr() returns a list
    representation.

    NOTE THAT THIS CLASS COULD BE DEPRECATED SOON.
    """
    class _non_null_iterator:
        def __init__(self, it):
            self.it = it
        def __iter__(self):
            return self
        def next(self):
            value = self.it.next()
            if value is None:
                return self.next()
            return value
    
    def __iter__(self):
        return PyPLearnList._non_null_iterator(self.itervalues())

    def _unreferenced(self):
        return True
    
    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        """PLearn representation of this python object.

        Are considered as elements any non-None attributes.
        """
        elem_format = lambda elem: inner_repr( elem, indent_level+1 )
        return '[%s]' % format_list_elements([optval for optval in iter(self)],
                                             elem_format, indent_level+1)

if __name__ == '__main__':
    
    class A( PyPLearnObject):
        a = PLOption('a')
        
    class B(A):
        b = PLOption('b')
    
    class C(B):
        c = PLOption('c')
    
    class BB(B):
        bb = PLOption('bb')
   
    class CC(C):
        cc = PLOption('cc')

    print
    print C.class_options()
    print C.__dict__.keys()
    
    print
    print BB.class_options()
        
    print
    ccobj = CC( instance_cc = 'instance_cc' )
    print ccobj.instance_cc
    print CC.class_options()
    print ccobj.getOptionNames()
    
    class ALittleMore(CC):
        optionC = PLOption("optionC")
        optionB = PLOption("optionB")
        listOption = PLOption( list )
    
    print
    moreobj = ALittleMore( instance_more = 'instance_more' )
    moreobj.added_option = '+++option'
    
    print moreobj.added_option
    print ALittleMore.class_options()
    print moreobj.getOptionNames()
    
    print
    moreobj.listOption.append( 1.0 )
    print moreobj.listOption
    print ALittleMore().listOption
    
    # print
    # print python_repr(moreobj)
    
    del moreobj.cc
    print moreobj
    
    print
    newcc = CC( cc = 2 )
    print newcc
    
    class BBC(BB,C):
        b = 'Already declared as an option'
        c = PLOption('But still if you wish to use PLOption, it works.')
        bbc = PLOption(True)

    print BBC.class_options()
    print BBC()
