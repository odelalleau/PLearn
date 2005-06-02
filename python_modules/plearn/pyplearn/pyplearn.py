"""Pyplearn is a python preprocessor for .plearn files.

We use Python function definitions, loops and variables to ease the building
of .plearn files with lots of repetition. Because of the (almost) compatible
syntaxes of PLearn and Python, one can write the contents of a Python function
that generates PLearn pretty much as straight PLearn.

Here is a little introductory tutorial, which assumes that you are
somewhat familiar with .plearn scripts.

X{Tutorial on pyplearn scripts}
===============================

    The $X{DEFINE} statement
    ------------------------

    First lets say you have a .plearn file that looks like::

        $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                       inputsize     = 10; 
                                       targetsize    = 1              ) }
    
        PTester( expdir = "constant";
                 dataset = ${DATASET};
                 statnames = ["E[train.E[mse]]" "V[test.E[mse]]"];
                 save_test_outputs = 1;
                 provide_learner_expdir = 1;
                 
                 learner = SomeLearnerClassTakingNoOption();
                 ) # end of PTester
    
    Then, the .pyplearn file will be::
    
        from plearn.pyplearn import *
    
        dataset = pl.AutoVMatrix( specification = "somefile.pmat",
                                  inputsize     = 10, 
                                  targetsize    = 1              )
    
        def main():
            return pl.PTester( expdir = "constant",
                               dataset = dataset,
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                           
                               learner = pl.SomeLearnerClassTakingNoOption();
                               ) # end of PTester
        
    First, note that the python affectation works just like the $DEFINE
    plearn statement. Also note that the python syntax requires the use of
    commas (,) instead of semicolons (;) to separate the options
    provided. You can also see that python lists [] are mapped to plearn
    TVec's and that there elements must be separated by commas rather that by
    spaces. Finally, any .pyplearn file must contain a single main()
    function that returns the representation of a runnable PLearn object. 
    
    Now, one may wonder what does the 'pl' stands for in 'pl.AutoVMatrix',
    'pl.PTester' or 'pl.SomeLearnerClassTakingNoOption()'.  To answer that
    question, we must first say that the pyplearn mecanism acts as an
    additionnal layer over the plearn scripts mecanism, which means that
    the pyplearn driver simply reads the .pyplearn file and generates the
    corresponding .plearn script to be read by plearn. The 'pl' object emulates
    a module that, for any PLearn class, accepts all options you may wish
    to provide. 
    
    Remember that the pyplearn mecanism IS NOT a bridge between the C++
    PLearn library and python, it simply is another parsing layer that
    provides us with the power of a programming language. Therefore, the 'pl'
    instance is not really awared of the PLearn classes, which means that
    you could write::
    
        pl.NameOfAClassThatDoesNotExist( any_option_you_like = "bla",
                                         foo                 = ["bar"] )
    
    and the pyplearn driver would generate the following .plearn script::
    
        NameOfAClassThatDoesNotExist( any_option_you_like = "bla";
                                      foo                 = ["bar"] )
    
    that would fail when plearn would try to create a
    NameOfAClassThatDoesNotExist object. Therefore, we do not have to
    maintain any list of 'known classes' and every new class you may write
    will be managed by the pyplearn mecanism since 'pl' simply acts as a
    parser.
    
    The $X{INCLUDE} statement
    -------------------------
    
    If you are used to program complex .plearn scripts, you may have
    developed a tendency to break down your script in simpler and smaller
    .plearn files that are to be included in the final .plearn script. Lets
    say that you have a file, dataset.plearn, that defines your dataset::
    
        ## dataset.plearn
        $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                       inputsize     = 10; 
                                       targetsize    = 1              ) }
        ## end of dataset.plearn
    
    and that you used to include this file in your final .plearn script::
    
        ## final_script.plearn
        $INCLUDE{dataset.plearn}
     
        PTester( expdir = "constant";
                 dataset = ${DATASET};
                 statnames = ["E[train.E[mse]]" "V[test.E[mse]]"];
                 save_test_outputs = 1;
                 provide_learner_expdir = 1;
                
                 learner = SomeLearnerClassTakingNoOption();
                 ) # end of PTester
        ## end of final_script.plearn
    
    Being now under the python syntax, you have to create a python module::
    
        ## dataset.py
        dataset = pl.AutoVMatrix( specification = "somefile.pmat",
                                  inputsize     = 10, 
                                  targetsize    = 1              )
        ## end of dataset.py
    
    and include it in the python fashion::
    
        ## final_script.pyplearn
        from plearn.pyplearn import *
        from dataset import *
       
        def main():
            return pl.PTester( expdir = "constant",
                               dataset = dataset,
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                               
                               learner = pl.SomeLearnerClassTakingNoOption();
                               ) # end of PTester
        ## end of final_script.pyplearn
    
    There is another way to generate the same .plearn script and that is::
    
        ## final_script2.pyplearn
        from plearn.pyplearn import *
        
        def main():
            pl_script  = include("dataset.plearn")
            pl_script += pl.PTester( expdir = "constant",
                                     dataset = plvar("DATASET"),
                                     statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                                     save_test_outputs = 1, 
                                     provide_learner_expdir = 1,
                                     
                                     learner = pl.SomeLearnerClassTakingNoOption();
                                     ) # end of PTester
            return pl_script
        ## end of final_script2.pyplearn
    
    where the plvar(variable_name) function emulates the behavior of the
    plearn ${variable_name} statement. Note that the include(filename)
    function used above also allows one to import 'inline' some .plearn
    chunk of code, that is::
    
        ## dataset2.plearn
        AutoVMatrix( specification = "somefile.pmat";
                     inputsize     = 10; 
                     targetsize    = 1              )
        ## end of dataset.plearn
        
        ## final_script3.pyplearn
        from plearn.pyplearn import *

        def main():
            return pl.PTester( expdir = "constant",
                               dataset = include("dataset2.plearn"),
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                              
                               learner = pl.SomeLearnerClassTakingNoOption()
                               ) # end of PTester
        ## end of final_script3.pyplearn
    
       
    X{Bindings}
    -----------
    
     When designing a .plearn script, one may want multiple object to
    refer to a single object. In a .plearn file, it may be done by doing
    the following::
    
        ## example.plearn
        SomeRunnableObject( dataset  = *1 -> AutoVMatrix( specification = "somefile.pmat";
                                                          inputsize     = 10;
                                                          targetsize    = 1              );
                            
                            internal = SomeObject( dataset = *1; );
                            );
        ## end of example.plearn
    
    The corresponding .pyplearn script uses the bind(name, x) and ref(name) functions::
    
        ## example.pyplearn
        from plearn.pyplearn import *

        bind( "dataset",
              pl.AutoVMatrix( specification = "somefile.pmat",
                              inputsize     = 10,
                              targetsize    = 1              ) )
        
        def main():
            return pl.SomeRunnableObject( dataset  = ref("dataset"),
                                          internal = SomeObject( dataset = ref("dataset") ) )
        ## end of example.pyplearn
        
    Remember that the following::
    
        ## example2.pyplearn
        from plearn.pyplearn import *

        dataset = pl.AutoVMatrix( specification = "somefile.pmat",
                                  inputsize     = 10,
                                  targetsize    = 1              )
        
        def main():
            return pl.SomeRunnableObject( dataset  = dataset,
                                          internal = SomeObject( dataset = dataset ) )
        ## end of example2.pyplearn
    
    would have created two different VMatrix instances.
    
    
    X{Command Line Arguments}
    -------------------------
    
    Another functionnality that one may want to use is accepting, on the
    command line, arguments whose values may influence the behavior of the
    script. The plearn program supports that in the following way::
    
        prompt %> plearn command_line.plearn ON_CMD_LINE="somefile.pmat"
    
    with the command_line.plearn script being::
    
        ## command_line.plearn
        SomeRunnableObject( dataset  = *1 -> AutoVMatrix( specification = ${ON_CMD_LINE};
                                                          inputsize     = 10;
                                                          targetsize    = 1              );
    
                            internal = SomeObject( dataset = *1; );
                            );
        ## end of command_line.plearn
    
    The same can be done under the pyplearn framework::
            
        prompt %> plearn command_line.pyplearn ON_CMD_LINE="somefile.pmat"
    
    with the command_line.pyplearn script being::
    
        ## command_line.pyplearn
        from plearn.pyplearn import *

        bind( "dataset",
              pl.AutoVMatrix( specification = plargs.ON_CMD_LINE,
                              inputsize     = 10,
                              targetsize    = 1                ) )
    
        def main():
            return pl.SomeRunnableObject( dataset  = ref("dataset"),
                                          internal = SomeObject( dataset = ref("dataset") ) )
        ## end of command_line.pyplearn

    The plargs arguments given on the command line are interpreted
    as strings, so if you want to pass an integers (int) or a
    floating-point values (float), you will have to cast them using
    python's synthax. So instead of::

        ## command_line.plearn
        SomeRunnableObject( dataset  = *1 -> AutoVMatrix( specification = ${ON_CMD_LINE};
                                                          inputsize     = ${INPUT_SIZE};
                                                          targetsize    = 1              );
                            internal = SomeObject( dataset = *1; );
                            );
        ## end of command_line.plearn

    you will have::

        ## command_line.pyplearn
        from plearn.pyplearn import *

        bind( "dataset",
              pl.AutoVMatrix( specification = plargs.ON_CMD_LINE,
                              inputsize     = int( plargs.INPUT_SIZE ),
                              targetsize    = 1                       ) )
    
        def main():
            return pl.SomeRunnableObject( dataset  = ref("dataset"),
                                          internal = SomeObject( dataset = ref("dataset") ) )
        ## end of command_line.pyplearn

    which you can launch with::

        prompt %> plearn command_line.pyplearn ON_CMD_LINE="somefile.pmat" INPUT_SIZE=10


    Note that one can use plarg_defaults to set default values for any
    expected plearn agument (plargs), e.g.::
    
        ## command_line_with_default.pyplearn
        from plearn.pyplearn import *

        plarg_defaults.ON_CMD_LINE = "some_default_file.pmat"
        bind( "dataset",
              pl.AutoVMatrix( specification = plargs.ON_CMD_LINE,
                              inputsize     = 10,
                              targetsize    = 1                ) )
        
        def main():
            return pl.SomeRunnableObject( dataset  = ref("dataset"),
                                          internal = SomeObject( dataset = ref("dataset") ) )
        ## end of command_line_with_default.pyplearn
        
    which won't fail and use "some_default_file.pmat" if::
    
        prompt %> plearn command_line.pyplearn
    
    is entered.
    
    
And Now What?
=============

Under the pyplearn mecanism, you can use the power of a incredibly
vast programming language to build your plearn experiments. Some
.plearn users had already started to expend to set of macros
$MACRONAME to be supported by the plearn scripts parser; but each
functionality you had makes you realize that you'd like to use another
one as well. Finally, we would have had to implement a whole new
language. The pyplearn mecanism uses python as an answer to that
problem. Not only can you use any U{control flow
tools<http://docs.python.org/tut/node6.html>} you may need but you can
also define functions (even objects! -- L{PLearnObject}) to generate
chunks of code you often reproduce in your experiments.

As an example, if you of often use PTester in the following way::

    ## often.pyplearn
    from plearn.pyplearn import *

    def main():
        pl.PTester( expdir = "constant",
                    dataset = include("dataset2.plearn"),
                    statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                    save_test_outputs = 1, 
                    provide_learner_expdir = 1,
                    
                    learner = pl.SomeLearnerClassTakingNoOption()
                    ) # end of PTester
    ## end of often.pyplearn

you could well write a small module::

    ## my_functions.py
    from plearn.pyplearn import *

    def myPTester( learner ):
        return pl.PTester( expdir = "constant",
                           dataset = include("dataset2.plearn"),
                           statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                           save_test_outputs = 1, 
                           provide_learner_expdir = 1,
                           
                           learner = learner
                           ) # end of PTester
    ## end of my_functions.py

and the have .pyplearn files looking like::

    ## shorter_often.pyplearn
    from plearn.pyplearn import *
    from my_functions import *

    def main():
        return myPTester( pl.SomeLearnerClassTakingNoOption() )
    ## end of shorter_often.pyplearn


For X{emacs users}, be sure to append your I{auto-mode-list} (in your .emacs)
the two following mappings::
    ("\\.py$"           . python-mode)	
    ("\\.pyplearn$"     . python-mode)
to enable the very useful I{python-mode} tools and highlighting.

X{Extending the pyplearn toolkit}
=================================

Most of the current (and, we hope, future) modules essentially have the
same shape which is::
    
    ## <module_name>.py
    import plearn.pyplearn, module_to_import1, module_to_import2 
    from some_other_module import *
    
    ##########################################
    ### Helper functions 
    ##########################################
  
    def some_helper_function():
        \"""Helps in any way\"""
        return pl.Something()
    
    ########################################################
    ###  Default values for MainClass attributes
    ########################################################
    
    class MainClassDefaults:
        \"""Contains default values for a python MainClass object.\"""

        some_field_default_value = None
    
    ########################################################
    ### The main class of the module: 
    ###  Emulates a MainClass PLearn object
    ########################################################
        
    class MainClass( PLearnObject ):
        def __init__(self, mandatory_option1, mandatory_option2, defaults=SubclassDefaults, **overrides):
            PLearnObject.__init__(self, defaults, overrides)
            self._set_attribute('mandatory_option1', mandatory_option1)
            self._set_attribute('mandatory_option2', mandatory_option2)

        def plearn_repr(self):
            return pl.MainClass( **self.plearn_options() )
        
    ## <module_name>.py

B{Note} that we B{STRONGLY} suggest that a class emulating some PLearn
class should be named the same than its PLearn cousin B{and} a that
module, if it contains such a class, should have the same name (The
example given above should therefore be named MainClass.py).

Moreover, any module that you want to add to the pyplearn toolkit
should be documented according to the U{Epytext Markup
Language<http://epydoc.sourceforge.net/epytext.html>} in order to be
parsed by U{epydoc<http://epydoc.sourceforge.net/>}. The documentation
you are currently using was generated by
U{epydoc<http://epydoc.sourceforge.net/>} and it would be in the interest
of all users if the documentation extended as fast as the code.

The basic rules for documenting python code are the following:

    1. All packages (__init__.py), modules, classes, methods and
       functions should start with a docstring, that is a
       triple-quoted string.

    2. A docstring can be a single-line sentence, meaning it ends with
    a period::

        def function():
            \"""This function does nothing.\"""
            pass

    with the triples quotes all on the same line. This case
    occurs when one line I{really} says it all. 
           
    3. A docstring can be composed of a single-line short description
    followed by a multi-lines explanation::

        def complex_function(n, a_string):
            \"""Returns an array of length B{n} whose elements are all equal to B{a_string}.
            
            Remark that an empty line was let before the explanation.
            
            @param n: The desired length of the returned array
            @type  n: Int
            
            @param a_string: The string to be used to fill the array.
            @type  a_string: String
            
            @return: An array of length B{n} filled with the
            string B{a_string}
            \"""
            if not isinstance(a_string, types.StringType):
                raise TypeError(\"Note that you are not forced to always check the type of args. \"
                                \"For instance, we won't check the type of 'n'.\")
            return [ a_string for i in range(0, n) ]
                 
    where the last triple quotes are on there own line. Remark that we
    bolded the names of the arguments and that we documented their use
    according to I{epytext} standards.
             
Once you know that, reading the
U{Epytext Markup Language Manual<http://epydoc.sourceforge.net/epytext.html>}
should not take you more than 15 minutes and you will be ready to document your
code.
"""
__cvs_id__ = "$Id: pyplearn.py,v 1.32 2005/06/02 14:24:25 chapados Exp $"

import string, types
import plearn.utilities.metaprog as metaprog

__all__ = [ 'PyPlearnError', 'plvar',
            '_parse_plargs', 'plargs', 'generate_expdir', 'plarg_defaults',
            'bind_plargs', 'plargs_binder', 'plargs_namespace',
            'include',

            ## Exceptions
            'UnknownArgumentError'
            ]

class plearn_snippet:
    """Objects of this class are used to wrap the parts of the Python code
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

class PyPlearnError(Exception):
    """For unexpected or incorrect use of some pyplearn mecanism."""
    pass

class DuplicateBindingError(PyPlearnError):
    """This exception is raised when attemping to bind more than one PLearn
    expression to the same variable name."""
    def __init__(self, binding_name):
        self.binding_name = binding_name

    def __str__(self):
        return "Binding '%s' already defined." % self.binding_name

class UnknownArgumentError(PyPlearnError):
    """This exception is raised when attempting to use a PLearn argument that
    was not defined, either on the command-line or with plarg_defaults."""
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown pyplearn argument: '%s'." % self.arg_name

def plvar(variable_name):
    """Emulates the behavior of the plearn ${variable_name} statement.

    variable_name <- string
    """
    if type(variable_name) != type(""):
        raise TypeError("The plvar function expects a string representing the "
                        "name of the .plearn variable that you want to refer to.")
    return plearn_snippet('${%s}' % variable_name)

def include(filename, replace_list = []):
    """Includes the contents of a .plearn file.
    
    This function allows to do both the followings:

    1) Including a plearn file::
        ## script.pyplearn
        def main():
            pl_script  = include("dataset.plearn")
            pl_script += pl.PTester( expdir = "constant",
                                     dataset = plvar("DATASET"),
                                     statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                                     save_test_outputs = 1, 
                                     provide_learner_expdir = 1,
                                     
                                     learner = pl.SomeLearnerClassTakingNoOption();
                                     ) # end of PTester
            return pl_script
        ## end of final_script2.pyplearn

    where::

        ## dataset.plearn
        $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                       inputsize     = 10; 
                                       targetsize    = 1              ) }
        ## end of dataset.plearn

    2) Importing 'inline' some plearn chunk of code, that is::

        ## dataset2.plearn
        AutoVMatrix( specification = "somefile.pmat";
                     inputsize     = 10; 
                     targetsize    = 1              )
        ## end of dataset.plearn

        ## script2.pyplearn
        def main():
            return pl.PTester( expdir = "constant",
                               dataset = include("dataset2.plearn"),
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                               
                               learner = pl.SomeLearnerClassTakingNoOption();
                               ) # end of PTester
        ## end of script2.pyplearn
        """
    f = open(filename, 'U')
    try:
        include_contents = f.read()
        for (search,replace) in replace_list:
            include_contents = include_contents.replace(search,replace)
    finally:
        f.close()
    return plearn_snippet(include_contents)

def generate_expdir( ):
    import os
    from plearn.utilities.toolkit       import date_time_string

    expdir = "expdir"
    if os.getenv('PyTest', '') != 'Running':                        
        expdir = '%s_%s' % ( expdir, date_time_string() )

    return expdir

class _plargs_storage_fallback:
    """A singleton instance of this class is instanciated by the package
    to store the default values for PLearn command-line variables."""
    def __init__( self ):
        self.__dict__['expdir'] = generate_expdir()
        
    def __setattr__(self, k, v):
        if k == 'expdir':
            raise AttributeError("Cannot modify the value of 'expdir'.")
        self.__dict__[k] = v
plarg_defaults = _plargs_storage_fallback()


class _plargs_storage_readonly:
    def __setattr__(self, k, v):
        raise AttributeError('Cannot modify plargs')

    def __getattr__(self, k):
        if k.startswith('__'):
           raise AttributeError

        try:
            return getattr(plarg_defaults, k)
        except AttributeError:
            raise UnknownArgumentError(k)
plargs = _plargs_storage_readonly()

def _parse_plargs(args):
    """Parses PLearn command-line arguments (which look like foo=1 bar=2)
    and stores the value of the arguments as attributes to plargs."""
    for a in args:
        k, v = a.split('=', 1)
        plargs.__dict__[k] = v

def bind_plargs(obj, field_names = None, plarg_names = None, prefix = None):
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

    Therefore, you can configure the I{MiscDefaults} values from the command line.
    
    @param obj: Either a class (fields => static members) or an
    instance from which to get the default values for plarg_defaults
    and in which to set the user provided values.
    @type  obj: Class or instance.

    @param field_names: The name of the fields within I{obj}.
    @type  field_names: List of strings.

    @param plarg_names: The desired names for the plargs. If it is let
    to None, the I{field_names} values will be used.
    @type  plarg_names: List of strings.

    @param prefix: If provided, the plarg_names will be overridden to be
    [ "%s_%s" % (prefix, f) for f in field_names ].
    @type prefix: str
    """
    if field_names is None: field_names = metaprog.public_members(obj).keys()

    if prefix is None:
        if plarg_names is None:
            plarg_names = field_names
    else:
        plarg_names = [ "%s_%s" % (prefix, f) for f in field_names ]

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
        provided_value = getattr(plargs, arg_name)

        if default_value is None:
            setattr(obj, field, provided_value)
        else:           
            cast = type(default_value)
            if cast is list:
                elem_cast = str
                if default_value:
                    elem_cast = type(default_value[0])

                def list_cast( s ):
                    if s:
                        return [ elem_cast(e) for e in s.split(",") ]
                    return []
                cast = list_cast
            setattr(obj, field, cast(provided_value))

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

class plargs_namespace:
    """Subclasses will have there class variables binded to PREFIXED plargs.

    The plarg will be prefixed by the classname, e.g.::

        class MLM( plargs_namespace ):
            ma               = 252
            sdmult           = 1
            
        print plargs.MLM_ma          # prints 252
        print plargs.MLM_sdmult      # prints 1

    Note that MLM.ma == int(plargs.MLM_ma) will always be True.    
    """
    class __metaclass__( type ):
        def __init__(cls, name, bases, dict):
            bind_plargs( cls, prefix = cls.__name__ )
