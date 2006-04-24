"""PyPLearn's core"""
__cvs_id__ = "$Id$"

import os, string, types
from plearn.utilities import metaprog, toolkit
from plearn.pyplearn import config
from plearn.pyplearn.plearn_repr import plearn_repr

__all__ = [ 'plvar',
            'plargs', 'generate_expdir', 'plarg_defaults',
            'bind_plargs', 'plargs_binder', 'plargs_namespace',
            'include',
            'pyplearn_intelligent_cast',

            ## Exceptions
            'PyPLearnError',
            'UnknownArgumentError'
            ]

#
#  Helper functions
#
def pyplearn_intelligent_cast( default_value, provided_value ):
    if default_value is None:
        cast = lambda val: val

    else:
        cast = type(default_value)

        ## Special treatment for lists: recursively apply the intelligent
        ## cast up to one level (nested lists not supported)
        if cast is list:
            elem_cast = str
            if default_value:
                elem_cast = type(default_value[0])
                if elem_cast == list:
                    raise ValueError, "Nested lists are not supported by pyplearn_intelligent_cast"

            def list_cast( s ):
                ## Potentially strip left-hand [ and right-hand ]
                s = s.lstrip(' [').rstrip(' ]')
                if s:
                    return [ elem_cast(e) for e in s.split(",") ]
                return []
            cast = list_cast

    return cast(provided_value)
    

def bind_plargs(obj, field_names = None, plarg_names = None):
    """Binds some command line arguments to the fields of an object.

    In short, given::

        class MiscDefaults:
            pca_comp              = 10
            pca_norm              = 1   # True
            sigma                 = 2.4

    this call::

        bind_plargs( MiscDefaults, [ "pca_comp", "pca_norm", "sigma" ] )

    is strictly equivalent to::

        plarg_defaults.pca_comp              = MiscDefaults.pca_comp
        plarg_defaults.pca_norm              = MiscDefaults.pca_norm
        plarg_defaults.sigma                 = MiscDefaults.sigma

        MiscDefaults.pca_comp                = plargs.pca_comp
        MiscDefaults.pca_norm                = plargs.pca_norm 
        MiscDefaults.sigma                   = plargs.sigma

    Therefore, you can configure the I{MiscDefaults} values from the
    command line. Note that, if you want bind all class variables of a
    class, you can simply declare that class to be a L{plargs_binder} to
    avoid the call to C{bind_plargs}::

        class MiscDefaults( plargs_binder ):
            pca_comp              = 10
            pca_norm              = 1   # True
            sigma                 = 2.4

    Also note that B{homogenous} list bindings are understood, so that::

        #
        # testScript.py
        #
        from plearn.pyplearn import plargs_binder
        class Test( plargs_binder ):
            some_list = [ 0.0 ]

        print 'List is:', Test.some_list

        # Prompt
        prompt %> python testScript.py some_list=1,2,3.5
        List is: [1.0, 2.0, 3.5]

    where the type of the elements in the list is given by the type of the
    first element in the default list.
    
    @param obj: Either a class (fields => static members) or an
    instance from which to get the default values for plarg_defaults
    and in which to set the user provided values.
    @type  obj: Class or instance.

    @param field_names: The name of the fields within I{obj}.
    @type  field_names: List of strings.

    @param plarg_names: The desired names for the plargs. If it is let
    to None, the I{field_names} values will be used.
    @type  plarg_names: List of strings.
    """
    if field_names is None:
        field_names = metaprog.public_members(obj).keys()

    if plarg_names is None:
        plarg_names = field_names

    for i, field in enumerate(field_names):
        arg_name = plarg_names[i]
        
        ## First set the argument's default value to the one provided
        ## by obj
        default_value  = getattr(obj, field)
        if default_value is None:
            setattr( plarg_defaults, arg_name, None )
        elif isinstance( default_value, list ):
            setattr( plarg_defaults, arg_name, ",".join([ str(e) for e in default_value ]) )
        else:
            setattr( plarg_defaults, arg_name, str(default_value) )

        ## binding: Then set the obj value to the one returned by
        ## plarg. If it was not provided by the user, the value will
        ## be set exactly to what it was when this funtion was
        ## entered. Otherwise, it will be set to the user provided value
        provided_value = pyplearn_intelligent_cast( default_value, getattr(plargs, arg_name) )
        setattr( obj, field, provided_value )

def generate_expdir( ):
    """Generates a standard experiment directory name."""
    from plearn.utilities.toolkit import date_time_string    

    expdir = "expdir"        
    if (os.environ.get('PYTEST_STATE', '') != 'Active'):
        expdir = '%s_%s' % ( expdir, date_time_string('_','_') )

    return expdir

#
#  Support Functions: mixing PLearn and PyPLearn scripts
#
def include( filename, replace_list = [] ):
    """Includes the content of a .plearn file.

    Some users that have developed complex chunks of PLearn scripts may not
    be keen to rewrite those in PyPLearn promptly. Hence, this function
    (combined with plvar()) allows to do both the followings:

      1. Including a .plearn file::
        #
        # script.pyplearn
        #
        def main():
            pl_script  = include("dataset.plearn")
            pl_script += pl.PTester( expdir = plargs.expdir,
                                     dataset = plvar("DATASET"),
                                     statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                                     save_test_outputs = 1, 
                                     provide_learner_expdir = 1,
                                     
                                     learner = pl.SomeLearnerClassTakingNoOption();
                                     ) # end of PTester
            return pl_script

        where::
          #
          # dataset.plearn
          #
          $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                         inputsize     = 10; 
                                         targetsize    = 1              ) }

      2. Importing 'inline' some plearn chunk of code, that is::

        #
        # dataset2.plearn
        #
        AutoVMatrix( specification = ${DATAPATH};
                     inputsize     = 10; 
                     targetsize    = 1              )

        #
        # script2.pyplearn
        #
        def main():
            plvar( 'DATAPATH', 'somefile.pmat' )
            return pl.PTester( expdir = plargs.expdir,
                               dataset = include("dataset2.plearn"),
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                               
                               learner = pl.SomeLearnerClassTakingNoOption();
                               )
    """
    f = open(filename, 'U')
    try:
        include_content = f.read()
        for (search,replace) in replace_list:
            include_content = include_content.replace(search,replace)
    finally:
        f.close()
    return PLearnSnippet( include_content )

def plvar( varname, value ):
    """Emulating PLearn's $DEFINE statement.
    
    An old .plearn script (included with include()) may depend on some
    variables to be defined externally. If one wants to define that
    variable within a PyPLearn script, he must use this function with two
    arguments::

        plvar( 'DATASET',
               pl.AutoVMatrix( specification = "somefile.pmat",
                               inputsize     = 10, 
                               targetsize    = 1
                               )
               )

    which is strictly equivalent to the old::

        $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                       inputsize     = 10; 
                                       targetsize    = 1              ) }
    
    If, on the other hand, a variable is defined within the PLearn script
    and must be referenced within PyPLearn, the simple C{plvar('DATASET')}
    will refer to the variable just as C{${DATASET}} would have.
    """
    if value is None:
        snippet = '${%s}' % varname
    else:
        snippet = '$DEFINE{%s}{ %s }' % (
            varname, plearn_repr( value, indent_level+1 )
            )
        
    return PLearnSnippet( snippet )
        
#
#  Core classes
#
class PyPLearnError( Exception ):
    """For unexpected or incorrect use of some PyPLearn mechanism."""
    pass

class UnknownArgumentError( PyPLearnError ):
    """Attribute known by neither plargs or plarg_defaults.

    This exception is raised when attempting to use a PLearn argument
    that was not defined, either on the command-line or with
    plarg_defaults.
    """
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown PyPLearn argument: '%s'." % self.arg_name

class PLearnSnippet:
    """Wrapper for PLearn code snippets.

    Objects of this class are used to wrap the parts of the Python code
    that have already be converted to a PLearn string, so they don't
    get the same treatment as Python strings (which will get wrapped in
    double quotes by plearn_repr() ).
    """
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return self.s

    def __repr__(self):
        return self.s

    def __add__(self, o):
        raise NotImplementedError("See dorionc")
    
    def __iadd__(self, snippet):
        """Overrides the operator+= on plearn_snippet instances.

        snippet <- plearn_snippet
        -> plearn_snippet
        """
        if not isinstance(snippet, plearn_snippet):
            raise TypeError("plearn_snippet.__add__ expects a plearn_snippet instance")
        self.s += snippet.s
        return self

    def _unreferenced( self ):
        return True

    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        return self.s

class _plargs_storage_fallback( object ):
    """Storing default values for plargs.

    A singleton instance of this class is instanciated by the package
    to store the default values for PLearn command-line variables.
    """
    def __init__( self ):
        self.expdir_root          = config.get_option( 'EXPERIMENTS', 'expdir_root' )
        self.__dict__['_expdir_'] = generate_expdir( )
        
    def __setattr__(self, k, v):
        if k in [ 'expdir', '_expdir_' ]:
            raise AttributeError( "Cannot modify the value of '%s'." % k )
        self.__dict__[k] = v
plarg_defaults = _plargs_storage_fallback()


class _plargs_storage_readonly( object ):
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

    Note that arguments given on the command line are interpreted as
    strings, so if you want to pass integers (int) or floating-point
    values (float), you will have to cast them as above.

    To set default values for some arguments, one can use
    plarg_defaults. For instance::

        # 
        # command_line_with_defaults.pyplearn
        #
        from plearn.pyplearn import *

        plarg_defaults.on_cmd_line = "some_default_file.pmat"
        plarg_defaults.input_size  = 10
        dataset = pl.AutoVMatrix( specification = plargs.on_cmd_line,
                                  inputsize     = int( plargs.input_size ),
                                  targetsize    = 1
                                  )
    
        def main( ):
            return pl.SomeRunnableObject( dataset  = dataset,
                                          internal = SomeObject( dataset = dataset ) )
        
    which won't fail and use C{"some_default_file.pmat"} with C{input_size=10} if::
    
        prompt %> plearn command_line.pyplearn
    
    is entered. If you are using a lot of command-line arguments, it is
    suggested that you use the L{bind_plargs} function of the
    L{plargs_binder} and L{plargs_namespace} classes.

    B{Note} that the value of plargs.expdir is generated automatically and
    B{can not} be assigned a default value through plarg_defaults. This
    behaviour aims to standardize the naming of experiment directories
    (L{xperiments}). For debugging purpose, however, one may provide on
    command-line an override to plargs.expdir value.
    """
    class namespace_overrides( dict ):
        pass
    
    def _parse_( self, args ):
        """Parsing and storing plargs.

        Parses PLearn command-line arguments (which look like foo=1 bar=2)
        and stores the value of the arguments as attributes to plargs.
        """
        for a in args:
            k, v = a.split('=', 1)

            scoped = k.split('.', 1)
            if len(scoped) > 1:
                name, attr = scoped

                # Existing namespace
                if name in self.__dict__:
                    nspace = self.__dict__[name]
                    assert isinstance( nspace, self.namespace_overrides )

                # New namespace
                else:
                    nspace = self.namespace_overrides()
                    self.__dict__[name] = nspace                    

                # Adding the attribute to the namespace
                nspace[attr] = v
                
            elif k == 'expdir':
                self.__dict__['_%s_'%k] = v
                
            else:
                self.__dict__[k] = v
    
    def __setattr__(self, k, v):
        raise AttributeError('Cannot modify plargs')

    def __getattr__( self, key ):
        if key.startswith( '__' ):
            raise AttributeError, key
        
        if key == 'expdir':
            expdir = self._expdir_
            root   = self.expdir_root
            if root:
                expdir = os.path.join( root, expdir )
                if not os.path.exists( root ):
                    # Possible set by parallel thread
                    try: 
                        os.makedirs( root )
                    except OSError, err:
                        if not os.path.exists( root ):
                            raise                        
            return expdir

        # Else, use default value
        try:
            return getattr(plarg_defaults, key)
        except AttributeError:
            raise UnknownArgumentError(key)        

plargs = _plargs_storage_readonly()

class plargs_binder:
    """Subclasses will have there class variables binded to plargs.

    The plarg name will be the exact field name, e.g.::

        class Preproc( plargs_binder ):
            start_year       = 2000
            last_year        = 2004
            
        print plargs.start_year      # prints 2000
        print plargs.last_year       # prints 2004

    Note that Preproc.start_year == int(plargs.start_year) will always be True.    
    """
    class __metaclass__( type ):
        def __init__(cls, name, bases, dict):
            bind_plargs( cls )

class plargs_namespace( object ):
    """Subclasses will have there class variables binded to B{prefixed} plargs.

    PLEASE UPDATE!!!
    
    The plarg will be prefixed by the classname, e.g.::

        class MLM( plargs_namespace ):
            ma               = 252
            sdmult           = 1
            
        print plargs.MLM_ma          # prints 252
        print plargs.MLM_sdmult      # prints 1

    Note that MLM.ma == int(plargs.MLM_ma) will always be True.    
    """
    class __metaclass__( type ):
        _subclasses = {}
        def __init__(cls, name, bases, dict):
            super(cls, cls).__init__(cls, name, bases, dict) 
            if cls.__name__ != 'plargs_namespace':
                cls._subclasses[cls.__name__] = cls
        
        def __new__( metacls, clsname, bases, dic ):
            overrides = {}
            if hasattr( plargs, clsname ):
                overrides = getattr( plargs, clsname )

            for attr_name, value in overrides.iteritems():
                default        = dic[attr_name]
                dic[attr_name] = globals()['pyplearn_intelligent_cast']( default, value )
            
            dic['__accessed'] = False
            return type.__new__( metacls, clsname, bases, dic )

        def __getattribute__( cls, attr ):            
            a = type.__getattribute__( cls, attr )
            if metaprog.public_attribute_predicate( attr, a ):
                setattr( cls, '__accessed', True )
            return a
        
