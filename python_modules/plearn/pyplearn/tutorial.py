"""PyPLearn Tutorial.""" 

def getIndentedScripts( pyplearn_path, indent ):
    # Reading file from script and indenting
    indented_pyscript = ""
    pyplearn_script = open( pyplearn_path, 'r' )

    first = True
    for line in pyplearn_script:
        if first:
            indented_pyscript += line
            first = False
        else:            
            indented_pyscript += indent + line
    pyplearn_script.close()

    # Parsing the script using the pyplearn_driver
    indented_plscript = ""
    plearn_script   = toolkit.command_output( "pyplearn_driver.py %s" %
                                              pyplearn_path )
    first = True
    for line in plearn_script:
        if first:
            indented_plscript += line
            first = False
        else:
            indented_plscript += indent + line

    return indented_pyscript, indented_plscript

class Tutorial:
    def start( cls ):
        print cls.__doc__

        for chp, chapter in enumerate( cls.chapters() ):
            if isinstance( chapter, tuple ):
                chapter, chapter_name = chapter
            else:
                chapter_name = chapter.__name__

            if not isinstance( chapter, str ):
                chapter = chapter.__doc__
                                
            print "Chapter %d: %s" % (chp+1, chapter_name)
            print chapter
            
    start = classmethod( start )

import inspect, os
from plearn_repr      import plearn_repr
from PyPLearnObject   import PyPLearnObject
from plearn.utilities import toolkit
#
#  Global variables
#
from plearn.utilities.ppath import ppath
LINEAR_REGRESSOR_SCRIPT = os.path.join(
    ppath("PLEARNDIR"),
    "plearn_learners/regressors/test/LinearRegressor/linear_regressor.pyplearn"
    )

class PyPLearnTutorial( Tutorial ):
    """PyPLearn Tutorial.

    This tutorial introduces you to the various tools provided by the
    PyPLearn mecanism. Essentially, the PyPLearn mecanism allows you to
    instanciate complex experimental schemes using a powerful and highly
    readable language: Python.
    """
    
    whatsWrong = """
    As a matter of fact, nothing's wrong with the PLearn scripts,
    especially in the above example. Hence why not use those directly?
    First, PLearn serialization format while having a syntax close to the
    Python still have partilarities of its own that can easily make edit
    hard to edit. For instance, emacs comes with a sympathetic python-mode
    while the plearn-mode is still to come...

    This said, remember that the above example shows very simple experiment
    settings. As settings get more complex, the use of references --- *3 ->
    FractionSplitter... --- and their management can get confusing, while
    the use the Python objects is totally intuitive. 

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

    def chapters( cls ):
        return [ (cls.introductoryExample(), "Introductory Examples"),
                 (cls.whatsWrong, "What's Wrong with PLearn Scripts?"),
                 (cls.moreComplexExample(), "A More Complex Example"),
                 (cls.whyNotAMain, "And Why Not to Code a Simple Main?"),
                 (cls.howDoesItWork(), "How Does it Work?"),
                 PyPLearnObject
                 ]
    chapters = classmethod( chapters )

    #
    #  Chapter methods
    #
    def introductoryExample( cls ):
        return """
    Let's see some examples of what you can do with PyPLearn
    scripts. First, assume that you want to perform a linear regression
    over some data contained in 'data.amat'. The following script

        #
        #  linear_regressor.pyplearn
        #

        %s
        
        #
        #  End of linear_regressor.pyplearn        
        #

    which will yield

        #
        #  PLearn representation (serialization mecanism)
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
    The first advantage of PyPLearn scripts is that they are Python all the
    way. Hence, it's easy to package functions in modules to be reused in
    later experiments.

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

    def howDoesItWork( cls ):
        import plearn.pyplearn as pyplearn
        pyplearn_magic_module = pyplearn.__dict__['__pyplearn_magic_module']                
        
        module_source = inspect.getsource( pyplearn_magic_module )
        module_source = module_source.replace('\n', '\n'+' '*8)

        return "  1. %s\n%s\n  2. Function plearn_repr\n    %s" \
            % ( pyplearn_magic_module.__doc__,
                ' '*8 + module_source,
                plearn_repr.__doc__
                )
    howDoesItWork = classmethod( howDoesItWork )
if __name__ == "__main__":
    PyPLearnTutorial.start( )
    
