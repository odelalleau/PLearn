"""PyPLearn Tutorial.

This tutorial introduces you to the various tools provided by the
PyPLearn mechanism. Essentially, the PyPLearn mechanism allows you to
instanciate complex experimental schemes using a powerful and highly
readable language: Python.

Section 1: Introductory Examples
================================

    Let's see some examples of what you can do with PyPLearn
    scripts. First, assume that you want to perform a linear regression
    over some data contained in 'data.amat'. The following script::

        #
        #  linear_regressor.pyplearn
        #

        import os.path
        from plearn.pyplearn import *
        
        plarg_defaults.data    = "data.amat"
        
        dataset = pl.AutoVMatrix(
            specification = plargs.data,
            inputsize = 2,
            targetsize = 2,
            weightsize = 0
            )
        
        learner = pl.LinearRegressor()
        
        splitter = pl.FractionSplitter(
            splits = TMat(1,2, [ (0,0.75) , (0.75,1) ])
            )
        
        tester = pl.PTester(
            expdir = plargs.expdir,
            dataset = dataset,
            splitter = splitter,
            learner = learner,
            statnames = ['E[train.E[mse]]', 'E[train.E[aic]]', 'E[train.E[bic]]', 'E[train.E[mabic]]',
                         'E[test.E[mse]]',  'E[test.E[aic]]',  'E[test.E[bic]]',  'E[test.E[mabic]]'],
            provide_learner_expdir = 1,
            save_test_costs = 1,
            save_test_outputs = 1,
            save_test_confidence = 1
            )
        
        def main():
            return tester

        
        #
        #  End of linear_regressor.pyplearn        
        #

    which will yield::

        #
        #  PLearn representation (serialization mechanism)
        #

        *4 -> PTester(
            dataset = *1 -> AutoVMatrix(
                inputsize = 2,
                specification = "data.amat",
                targetsize = 2,
                weightsize = 0
                ),
            expdir = "expdir_2006_01_12_15_27_44",
            learner = *2 -> LinearRegressor( ),
            provide_learner_expdir = 1,
            save_test_confidence = 1,
            save_test_costs = 1,
            save_test_outputs = 1,
            splitter = *3 -> FractionSplitter(
                splits = 1 2 [
                        0:0.75,
                        0.75:1
                        ]
                ),
            statnames = [
                "E[train.E[mse]]",
                "E[train.E[aic]]",
                "E[train.E[bic]]",
                "E[train.E[mabic]]",
                "E[test.E[mse]]",
                "E[test.E[aic]]",
                "E[test.E[bic]]",
                "E[test.E[mabic]]"
                ]
            )

        
Section 2: What's Wrong with PLearn Scripts?
============================================

    As a matter of fact, nothing's wrong with the PLearn scripts,
    especially in the above example. Hence why not use those directly?
    First, PLearn serialization format while having a syntax close to the
    Python still have particularities of its own that can easily make it
    hard to edit and affect readability. For instance, emacs comes with a
    sympathetic python-mode while the plearn-mode is still to come...

    This said, remember that the above example shows very simple experiment
    settings. As settings get more complex, the use of references --- *3 ->
    FractionSplitter... --- and their management can get confusing, while
    the use the Python objects is totally intuitive
    (L{tutorial_stuff.tuto_complex_scheme}).

    Furthermore, when writing complex experiments, you may want your script
    to manage command-line options. Ok, PLearn script accepted command-line
    arguments too, but some (brave) PLearn developers ended up augmenting
    the serialization format with statements like IF, SWITCH, DIVIDE,
    INCLUDE, ...

    In short, they ended up building a new language within the
    serialization format... Using Python was quite an obvious alternative,
    since the syntactic similarities, and provides us with a world of
    possibilities. 
    

    Section 2.1: A More Complex Example
    -----------------------------------

    The main advantage of PyPLearn scripts is that they are Python all the
    way. Hence, among other things, it's easy to package functions in
    modules to be reused in later experiments::

        #
        #  My module
        #

        import random 
        from plearn.pyplearn import pl
        
        def LearnerWithSomeSettingsIOftenUse( learning_rate, dataset_mng ):
            option1 = pl.Option1( val = 10 )
            option2 = pl.Option2( foo = 'bar' )
        
            return pl.SomeLearner(
                learning_rate = learning_rate,
                dataset_mng   = dataset_mng,
                o1            = option1,
                o2            = option2
                )                           
            
        def MyCombiner( underlying_learners ):
            weights = [ random.random()
                        for n in range( len(underlying_learners) ) ]
        
            wsum = sum( weights )
            normalized = [ w/wsum for w in weights ]
        
            return pl.CombinerLearner(
                underlying_learners = underlying_learners,
                weights             = normalized
                )
        

        #
        #  My script
        #

        from plearn.pyplearn import *
        from simple_module   import *
        
        class Model( plargs_namespace ):
            data           = "data.amat"
            learning_rates = [ float( A * 10**(-B) )
                               for A in [7, 3, 1]
                               for B in range(0, 6) ]  
        
        #
        #  Dataset 
        #
        dataset  = pl.AutoVMatrix(
            specification = Model.data,
            inputsize     = 2,
            targetsize    = 2,
            weightsize    = 0
            )
        
        splitter = pl.FractionSplitter(
            splits = TMat(1,2, [ (0,0.75) , (0.75,1) ])
            )
        
        dataset_manager = pl.DatasetManager( dataset  = dataset,
                                             splitter = splitter )
        
        #
        #  Using a combiner over simple learners
        #
        underlying_learners = [
            LearnerWithSomeSettingsIOftenUse( lr, dataset_manager )
            for lr in Model.learning_rates
            ]
        
        top_learner = MyCombiner( underlying_learners )
        
        #
        #  Top Level
        #
        tester = pl.MyWeardTester(
            expdir    = plargs.expdir,
            learner   = top_learner,
            provide_learner_expdir = 1,
            )
        
        def main():
            return tester


        #
        #  The result 
        #

        *59 -> MyWeardTester(
            expdir = "expdir_2006_01_12_15_27_45",
            learner = *58 -> CombinerLearner(
                underlying_learners = [
                    *6 -> SomeLearner(
                        dataset_mng = *3 -> DatasetManager(
                            dataset = *1 -> AutoVMatrix(
                                inputsize = 2,
                                specification = "data.amat",
                                targetsize = 2,
                                weightsize = 0
                                ),
                            splitter = *2 -> FractionSplitter(
                                splits = 1 2 [
                                        0:0.75,
                                        0.75:1
                                        ]
                                )
                            ),
                        learning_rate = 7.0,
                        o1 = *4 -> Option1( val = 10 ),
                        o2 = *5 -> Option2( foo = "bar" )
                        ),
                    *9 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.7,
                        o1 = *7 -> Option1( val = 10 ),
                        o2 = *8 -> Option2( foo = "bar" )
                        ),
                    *12 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.07,
                        o1 = *10 -> Option1( val = 10 ),
                        o2 = *11 -> Option2( foo = "bar" )
                        ),
                    *15 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.007,
                        o1 = *13 -> Option1( val = 10 ),
                        o2 = *14 -> Option2( foo = "bar" )
                        ),
                    *18 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.0007,
                        o1 = *16 -> Option1( val = 10 ),
                        o2 = *17 -> Option2( foo = "bar" )
                        ),
                    *21 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 7e-05,
                        o1 = *19 -> Option1( val = 10 ),
                        o2 = *20 -> Option2( foo = "bar" )
                        ),
                    *24 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 3.0,
                        o1 = *22 -> Option1( val = 10 ),
                        o2 = *23 -> Option2( foo = "bar" )
                        ),
                    *27 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.3,
                        o1 = *25 -> Option1( val = 10 ),
                        o2 = *26 -> Option2( foo = "bar" )
                        ),
                    *30 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.03,
                        o1 = *28 -> Option1( val = 10 ),
                        o2 = *29 -> Option2( foo = "bar" )
                        ),
                    *33 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.003,
                        o1 = *31 -> Option1( val = 10 ),
                        o2 = *32 -> Option2( foo = "bar" )
                        ),
                    *36 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.0003,
                        o1 = *34 -> Option1( val = 10 ),
                        o2 = *35 -> Option2( foo = "bar" )
                        ),
                    *39 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 3e-05,
                        o1 = *37 -> Option1( val = 10 ),
                        o2 = *38 -> Option2( foo = "bar" )
                        ),
                    *42 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 1.0,
                        o1 = *40 -> Option1( val = 10 ),
                        o2 = *41 -> Option2( foo = "bar" )
                        ),
                    *45 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.1,
                        o1 = *43 -> Option1( val = 10 ),
                        o2 = *44 -> Option2( foo = "bar" )
                        ),
                    *48 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.01,
                        o1 = *46 -> Option1( val = 10 ),
                        o2 = *47 -> Option2( foo = "bar" )
                        ),
                    *51 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.001,
                        o1 = *49 -> Option1( val = 10 ),
                        o2 = *50 -> Option2( foo = "bar" )
                        ),
                    *54 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 0.0001,
                        o1 = *52 -> Option1( val = 10 ),
                        o2 = *53 -> Option2( foo = "bar" )
                        ),
                    *57 -> SomeLearner(
                        dataset_mng = *3;,
                        learning_rate = 1e-05,
                        o1 = *55 -> Option1( val = 10 ),
                        o2 = *56 -> Option2( foo = "bar" )
                        )
                    ],
                weights = [
                    0.105851252189,
                    0.0322607679357,
                    0.125108786906,
                    0.0802142570663,
                    0.00163215588764,
                    0.107783064396,
                    0.039438048979,
                    0.0685751159145,
                    0.0152580197713,
                    0.103744296668,
                    0.0604637010528,
                    0.020912563261,
                    0.0711133493742,
                    0.0733308978533,
                    0.0131280705949,
                    0.0483155184377,
                    0.00936775827022,
                    0.0235023754425
                    ]
                ),
            provide_learner_expdir = 1
            )
        
        
Section 3: And Why Not to Code a Simple Main?
=============================================

    Didn't you ever compile PLearn??? 

    Believe me, you do not want to recompile each time little changes were
    made to your experiment's settings...
    
Section 4: How Does it Work?
============================


    Section 4.1: L{PyPLearn Magic Module.<__pyplearn_magic_module>}
    ---------------------------------------------------------------

    An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like::

        pl.SequentialAdvisorSelector( comparison_type='foo', ... )

    On any attempt to access a function from I{pl}, the magic module creates,
    on the fly, a PLearn-like class (derived from PyPLearnObject) named
    after the function asked for. The named arguments provided to the
    function are forwarded to the constructor of this new class to create
    an instance of the class. Hence, names of the function's arguments are
    considered as the PLearn object's option names.
    
    Implementation::
        class __pyplearn_magic_module:
            \"\"\"PyPLearn Magic Module.
        
            An instance of this class (instanciated as pl) is used to provide
            the magic behavior whereas bits of Python code like::
        
                pl.SequentialAdvisorSelector( comparison_type='foo', ... )
        
            On any attempt to access a function from I{pl}, the magic module creates,
            on the fly, a PLearn-like class (derived from PyPLearnObject) named
            after the function asked for. The named arguments provided to the
            function are forwarded to the constructor of this new class to create
            an instance of the class. Hence, names of the function's arguments are
            considered as the PLearn object's option names.
            \"\"\"
            def __getattr__(self, name):
                if name.startswith('__'):
                    raise AttributeError
        
                def initfunc(**kwargs):
                    klass = new.classobj(name, (PyPLearnObject,), {})
                    assert issubclass( klass, PyPLearnObject )
        
                    obj = klass(**kwargs)
                    assert isinstance(obj, PyPLearnObject)
                    return obj
                
                return initfunc
        

    Section 4.2: L{plearn_repr}
    ---------------------------
    Returns a string that is a valid PLearn representation of I{obj}.

    This function is somehow the core of the whole PyPLearn mechanism. It
    maps most Python objects to a representation understood by the PLearn
    serialization mechanism.
    
Section 5: L{PyPLearnObject}
============================
    A class from which to derive python objects that emulate PLearn ones.

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
    
Section 6: Command-Line Arguments
=================================


    Section 6.1: L{plargs}
    ----------------------
    Values read from or expected for PLearn command-line variables.

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
    

    Section 6.2: L{bind_plargs}
    ---------------------------
    Binds some command line arguments to the fields of an object.

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
    
    

    Section 6.3: L{plargs_binder}
    -----------------------------
    Subclasses will have there class variables binded to plargs.

    The plarg name will be the exact field name, e.g.::

        class Preproc( plargs_binder ):
            start_year       = 2000
            last_year        = 2004
            
        print plargs.start_year      # prints 2000
        print plargs.last_year       # prints 2004

    Note that Preproc.start_year == int(plargs.start_year) will always be True.    
    

    Section 6.4: L{plargs_namespace}
    --------------------------------
    Subclasses will have there class variables binded to B{prefixed} plargs.

    PLEASE UPDATE!!!
    
    The plarg will be prefixed by the classname, e.g.::

        class MLM( plargs_namespace ):
            ma               = 252
            sdmult           = 1
            
        print plargs.MLM_ma          # prints 252
        print plargs.MLM_sdmult      # prints 1

    Note that MLM.ma == int(plargs.MLM_ma) will always be True.    
    
Section 7: How to reuse my old PLearn scripts?
==============================================

    Two simple tools will help you do so.

    Section 7.1: L{include}
    -----------------------
    Includes the content of a .plearn file.

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
    

    Section 7.2: L{plvar}
    ---------------------
    Emulating PLearn's $DEFINE statement.
    
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
##
# DO NOT EDIT THIS MODULE DOCSTRING!
#
#  This docstring is automatically generated by the Tutorial subclass
#  contained in this module. Any change made manually will be lost.
##

#
#  Imports
#
import inspect, os
from plearn.pyplearn                  import *
from plearn.pyplearn                  import _plargs_storage_readonly
from plearn_repr               import plearn_repr
from PyPLearnObject            import PLOption, PyPLearnObject
from plearn.utilities.Tutorial import *

#
#  Global variables
#
from plearn.utilities.ppath import ppath
LINEAR_REGRESSOR_SCRIPT = os.path.join(
    ppath("PLEARNDIR"),
    "plearn_learners/regressors/test/LinearRegressor/linear_regressor.pyplearn"
    )

def pyPLearnMagicModuleSubSection( indent = ' '*4 ):    
    from plearn import pyplearn
    pyplearn_magic_module = pyplearn.__dict__['__pyplearn_magic_module']                
    
    module_source = inspect.getsource( pyplearn_magic_module )
    module_source = module_source.replace( '\n', '\n'+(indent*2) )

    doc   = pyplearn_magic_module.__doc__
    line1 = doc.find('\n')
    return SubSection( 'L{%s<__pyplearn_magic_module>}' % doc[:line1],
                       doc[line1+1:] + '\n' + indent + 'Implementation::\n' +
                       (indent*2) + module_source
                       )

class PyPLearnTutorial( Tutorial ):
    """PyPLearn Tutorial.

    This tutorial introduces you to the various tools provided by the
    PyPLearn mechanism. Essentially, the PyPLearn mechanism allows you to
    instanciate complex experimental schemes using a powerful and highly
    readable language: Python.
    """

    from tutorial_stuff import tuto_complex_scheme
    whatsWrong = """
    As a matter of fact, nothing's wrong with the PLearn scripts,
    especially in the above example. Hence why not use those directly?
    First, PLearn serialization format while having a syntax close to the
    Python still have particularities of its own that can easily make it
    hard to edit and affect readability. For instance, emacs comes with a
    sympathetic python-mode while the plearn-mode is still to come...

    This said, remember that the above example shows very simple experiment
    settings. As settings get more complex, the use of references --- *3 ->
    FractionSplitter... --- and their management can get confusing, while
    the use the Python objects is totally intuitive
    (L{tutorial_stuff.tuto_complex_scheme}).

    Furthermore, when writing complex experiments, you may want your script
    to manage command-line options. Ok, PLearn script accepted command-line
    arguments too, but some (brave) PLearn developers ended up augmenting
    the serialization format with statements like IF, SWITCH, DIVIDE,
    INCLUDE, ...

    In short, they ended up building a new language within the
    serialization format... Using Python was quite an obvious alternative,
    since the syntactic similarities, and provides us with a world of
    possibilities. 
    """
    
    whyNotAMain = """
    Didn't you ever compile PLearn??? 

    Believe me, you do not want to recompile each time little changes were
    made to your experiment's settings...
    """

    def sections( cls ):
        indent = (' '*4)
        def name_doc_pair( obj ):
            doc = obj.__doc__
            at  = doc.find('@') 
            if at != -1:
                doc = doc[:at]
                
            return ( 'L{%s}'%obj.__name__, indent+doc )
        
        return [ Section( "Introductory Examples", cls.introductoryExample() ),
                 
                 Section( "What's Wrong with PLearn Scripts?", cls.whatsWrong,
                          [ SubSection( "A More Complex Example", cls.moreComplexExample() ) ]
                          ),
                 
                 Section( "And Why Not to Code a Simple Main?", cls.whyNotAMain ),
                 
                 Section( "How Does it Work?", "",
                          [ pyPLearnMagicModuleSubSection(),
                            SubSection( *name_doc_pair(plearn_repr) )
                            ]
                          ),
                 
                 Section( *name_doc_pair(PyPLearnObject) ),

                 Section( "Command-Line Arguments", '',
                          [ SubSection( 'L{plargs}',
                                        indent+_plargs_storage_readonly.__doc__ ),
                            SubSection( *name_doc_pair(bind_plargs) ),
                            SubSection( *name_doc_pair(plargs_binder) ),
                            SubSection( *name_doc_pair(plargs_namespace) ),
                            ]),
                                           
                 Section( "How to reuse my old PLearn scripts?",
                          "\n    Two simple tools will help you do so.",
                          [ SubSection( *name_doc_pair(include) ),
                            SubSection( *name_doc_pair(plvar)   ) ]
                          )
                 ]
    sections = classmethod( sections )

    #
    #  Section methods
    #
    def introductoryExample( cls ):
        return """
    Let's see some examples of what you can do with PyPLearn
    scripts. First, assume that you want to perform a linear regression
    over some data contained in 'data.amat'. The following script::

        #
        #  linear_regressor.pyplearn
        #

        %s
        
        #
        #  End of linear_regressor.pyplearn        
        #

    which will yield::

        #
        #  PLearn representation (serialization mechanism)
        #

        %s
        """ % getIndentedScripts( LINEAR_REGRESSOR_SCRIPT, " "*8 )
    introductoryExample = classmethod( introductoryExample )

    def moreComplexExample( cls ):
        from tutorial_stuff import simple_module
        module_source = inspect.getsource( simple_module )
        module_source = module_source.replace('\n', '\n'+' '*8)
        
        pyscript, plscript = getIndentedScripts(
            "tutorial_stuff/simple_script.pyplearn",
            " "*8
            )
        
        return """
    The main advantage of PyPLearn scripts is that they are Python all the
    way. Hence, among other things, it's easy to package functions in
    modules to be reused in later experiments::

        #
        #  My module
        #

        %s

        #
        #  My script
        #

        %s

        #
        #  The result 
        #

        %s        
        """ % ( module_source, pyscript, plscript )
    moreComplexExample = classmethod( moreComplexExample )    


if __name__ == "__main__":
    PyPLearnTutorial.build( __file__ )
    
