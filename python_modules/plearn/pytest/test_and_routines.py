__cvs_id__ = "$Id: test_and_routines.py,v 1.20 2005/02/10 21:17:37 dorionc Exp $"

import os, shutil, string

from   plearn.utilities.version_control  import  is_under_version_control
import plearn.utilities.version_control  as      version_control

import plearn.utilities.toolkit          as     toolkit

from   plearn.tasks.Task                 import *
from   plearn.utilities.verbosity        import *
from   plearn.utilities.FrozenObject     import *

## In plearn.pytest
from   programs                          import *
from   BasicStats                        import BasicStats
from   IntelligentDiff                   import *          

__all__ = [
    ## Functions
    'config_file_path', 'print_stats', 

    ## Exceptions
    'PyTestUsageError',

    ## Classes
    'Test', 
    'Routine',
    'CompilationRoutine', 'ResultsCreationRoutine', 'RunTestRoutine'
    ]

def config_file_path( directory = None ):
    """The path to a pytest configuration file.

    @param directory: If provided, the returned value is the absolute
    path to a possible pytest configuration within I{directory}.
    @type  directory: String

    @return: A path (string) to a possible pytest configuration file.
    """
    if directory is None:
        return 'pytest.config'
    return os.path.join( os.path.abspath(directory), 'pytest.config' )

def print_stats():
    Test.statistics.print_stats()

class DuplicateName( PyTestUsageError ):
    def __init__(self, test1, test2):
        PyTestUsageError.__init__(
            self, "Two tests have the same name: %s and %s."
            % ( test1.get_path(), test2.get_path() )
            )        

class TestDefaults:
    name              = None
    description       = ''
    program           = None
    arguments         = ''
    resources         = []
    disabled          = False
##    comparable_psaves = []

    __declare_members__ = [ ('name',              types.StringType),
                            ('description',       types.StringType),
                            ('program',           Program),
                            ('arguments',         types.StringType),
                            ('resources',         types.ListType),
                            ('disabled',          types.BooleanType)##,
##                             ('comparable_psaves', types.ListType)
                            ]

class Test(FrozenObject):
    """Test is a class regrouping the elements that define a test for PyTest.

    @ivar name: The name of the Test must uniquely determine the
    test. Among others, it will be used to identify the test's results
    ( pytest/*_results/I{name}/ ) and to report test informations.
    @type name: String

    @ivar description: The description must provide other users an
    insight of what exactly is the Test testing. You are encouraged
    to used triple quoted strings for indented multi-lines
    descriptions.
    @type description: String

    @ivar program: The proram to be run by the Test. The L{Program}
    class currently has four subclasses: L{LocalProgram},
    L{GlobalProgram}, L{LocalCompilableProgram} and
    L{GlobalCompilableProgram}. The I{Local} prefix specifies that the
    program executable should be found in the same directory than the
    I{pytest.config} file, while I{Global} means that the program is a
    global command. Currently, this mecanism only support commands that
    are in one of the <plearn_branch>/commands/ directory.

    The I{Compilable} tag specifies that the program must be compiled.
    The default compiler is L{pymake}, but this behaviour can be changed
    and/or compile options may be specified.

    @type program: L{Program}

    @ivar arguments: The command line arguments to be passed to the program
    for the test to proceed.
    @type arguments: String

    @ivar resources: A list of resources that are used by your program either
    in the command line or directly in the code (plearn or pyplearn files, databases, ...).
    The elements of the list must be string representations of the path, absolute or relative,
    to the resource.
    @type resources: List of Strings
    """
    
    instances_map    = {}
    families_map     = {}
    
    statistics       = BasicStats("Test statistics")
    expected_results = os.path.join(ppath.pytest_dir, "expected_results")
    run_results      = os.path.join(ppath.pytest_dir, "run_results")

    def current_stats(cls):
        return cls.statistics.current_stats()
    current_stats = classmethod(current_stats)
    
    def __init__(self, defaults=TestDefaults, **overrides):
        FrozenObject.__init__(self, defaults, overrides)        

        self.set_attribute( 'test_directory', os.getcwd() )

        metaprotocol = string.replace( "PYTEST__%s__RESULTS" %
                                       self.name, ':', '_'
                                       )
        self.set_attribute( 'metaprotocol', metaprotocol )

        if Test.instances_map.has_key( self.name ):
            raise DuplicateName( Test.instances_map[self.name], self )
        else:
            Test.statistics.new_test( self.test_directory, self.name )
            Test.instances_map[self.name] = self
            if Test.families_map.has_key( self.test_directory ):
                Test.families_map[self.test_directory].append(self)
            else:
                Test.families_map[self.test_directory] = [self]
                
        self.set_str_spacer( '\n' )

##         if len(self.comparable_psaves) > 0:
##             assert isinstance(self.program, GlobalCompilableProgram)
            
    def sanity_check(self):
        if self.name == '':
            raise PyTestUsageError(                
                "Test must be named. Directory %s contains an unnamed test." 
                % self.test_directory
                )
        
        check = ( string.find(self.name, ' ') +
                  string.find(self.name, '-') +
                  string.find(self.name, '/') +
                  string.find(self.name, '<') +
                  string.find(self.name, '>') 
                  )

        if check != -5:
            raise PyTestUsageError(
                "%s\n Test.name should contain none of the following chars: "
                "' ', '-', '/', '<', '>'."
                % self.get_path()
                )
            
    def compilation_succeeded(self):
        """Forwards compilation status request to the program.

        B{Note} that the call will raise an exception if the program
        is not compilable.
        """
        return self.program.compilation_succeeded()

    def compile(self):
        """Forwards compilation request to the program.

        B{Note} that the call will raise an exception if the program
        is not compilable.
        """
        self.sanity_check()        
        self.program.compile()

    def ensure_results_directory(self, results):
        if os.path.exists( results ):
            if is_under_version_control( results ):
                answer = None
                while not answer in ['yes', 'no']:
                    answer = raw_input( "Results %s already are under version control! Are you sure "
                                        "you want to generate new results (yes or no)? " % results )

                if answer == 'no':
                    raise PyTestUsageError("Results creation interrupted by user")

                ## YES
                version_control.recursive_remove( results )
                version_control.commit( '.', 'Removal of %s for new results creation.' )

            shutil.rmtree( results )

        ## Make a fresh directory
        os.makedirs( results )

    def formatted_description(self):
        fdesc = [ "In %s"%self.test_directory, "" ]
        
        if string.lstrip(self.description, ' \n') != "":
            fdesc.extend( toolkit.boxed_lines(self.description, 50, indent='    ')+[""] )

        return string.join(["    "+line for line in fdesc], '\n')
        
    def get_name(self):
        return self.name
    
    def get_path(self):
        return os.path.join(
            self.test_directory,
            self.name
            )

    def is_disabled(self):
        return self.disabled

    def test_results(self, results):
        if results in [Test.expected_results, Test.run_results]:
            return os.path.join( results, self.name )
        return results
##     def test_results(self, results):
##         if results not in [Test.expected_results, Test.run_results]:
##             raise ValueError(
##                 "%s.run expects its results argument to be either "
##                 "%s.expected_results or %s.run_results"
##                 % (self.classname(), self.classname(), self.classname())
##                 )
        
##         return os.path.join( results, self.name )
        
    def link_resources(self, results):
        """Sets up all appropriate links and environment variables required by a test.

        @param results: Either Test.expected_results or Test.run_results
        (checked in test_results()).

        @returns: Returns the test's results directory path.
        @rtype: StringType.
        """
        self.sanity_check()

        ## Validates the results value and return this test's results path.
        test_results = self.test_results( results )        
        self.ensure_results_directory( test_results )

        ## Link 'physical' resources
        resources = []
        resources.extend( self.resources )
        resources.append( self.program.path )        
        Resources.link_resources( self.test_directory, resources, test_results )

        ## What remains of this method is used to make the following
        ## binding visible to the test program. It must be removed after
        ## the test ran (done in unlink_resources()).        
        metapath = os.path.abspath(test_results)
        ppath.add_binding(self.metaprotocol, metapath)

        ## This PLEARN_CONFIGS directory specific to this test::
        ##     - It must not be in the test_results (diff...) BUT;
        ##     - It must not be in a directory shared with other test's (multithread access...)
        internal_plearn_configs =\
            os.path.join( os.path.abspath(results), ## It must not be in the test_results 

                          ".plearn_%s" % self.name  ## It must not be in a
                                                    ## directory shared
                                                    ## with other test's
                          )

        ## Ensure that the directory exists
        if not os.path.exists( internal_plearn_configs ):
            os.mkdir( internal_plearn_configs )

        ## Create a new ppath.config file
        internal_ppath_config = os.path.join( internal_plearn_configs, "ppath.config" )
        internal_config_file  = open(internal_ppath_config, "w")

        ## This method writes the previously defined and the added
        ## metaprotocol to the internal_ppath_config
        ppath.write_bindings( internal_config_file.write )

        ## This process now consider the internal_plearn_configs
        os.putenv( "PLEARN_CONFIGS",   internal_plearn_configs )
        os.putenv( "PLEARN_DATE_TIME", "NO"  )

        ## Returns the test's results directory path
        return test_results

    def unlink_resources(self, test_results):
        """Removes links made by and environment variables set by I{link_resources}.

        @param test_results: B{Must} be the value returned by I{link_resources}.
        """
        Resources.unlink_resources( test_results )
        ppath.remove_binding(self.metaprotocol)
        os.putenv( "PLEARN_DATE_TIME", "YES" )

class RoutineDefaults(TaskDefaults):
    test                  = None
    
class Routine(Task):

    report_traceback = False
    
    def __init__( self, test,
                  defaults=RoutineDefaults, **overrides ):
        overrides['task_name'] = test.name
        Task.__init__( self, defaults, **overrides ) 

        self.test = test

    def compile_program(self):
        if not isinstance(self.test.program, Compilable):
            return True

        vprint("\nCompilation:", 2)
        vprint("------------", 2)
        
        self.test.compile()
        if not self.test.compilation_succeeded():
            vprint("Compilation failed.", 2)
            self.set_status("Failed")
            return False
        vprint("Compilation succeedded.", 2)
        return True

    def format_n_print(self, msg, ldelim='[', rdelim=']'):
        formatted = toolkit.centered_square( msg, 70, ldelim, rdelim )
        vprint(formatted, 1)

    ## Overrides run and succeeded
    def run(self):
        self.format_n_print("LAUCHED %s" % self.classname())
        self.format_n_print("%s" % self.test.get_name())
        vprint(self.test.formatted_description(), 1)
        
        try:
            if self.test.is_disabled():
                vprint("Test %s is disabled." % self.test.name, 2)
                self.set_status("Skipped")
                self.signal_completion()
            else:
                os.chdir( self.test.test_directory )
                self.run_body()

        except PyTestUsageError, e: 
            if Routine.report_traceback:
                raise
            else:
                e.print_error()
            self.set_status("Skipped")            
            self.signal_completion()

    def set_status(self, status):
        Task.set_status(self, status)

        if status in TaskStatus.completion_types:
            Test.statistics.set_status( self.test.get_name(), status )

    def signal_completion(self):
        tname            = self.test.get_name()
        self.format_n_print( "FINISHED %s" % self.classname() )
        self.format_n_print( "%s -- %s" % (tname, str(self.status)) )
        self.format_n_print( Test.current_stats() )
        vprint("\n", 1)

        Task.signal_completion(self)
        

class CompilationRoutine(Routine):
    """Launches the compilation of target tests' compilable files."""    
    def body_of_task(self):
        if self.compile_program():
            self.set_status( "Succeeded" )


class ResultsRelatedRoutine(Routine):
    """Base class for ResultsCreationRoutine && RunTestRoutine.

    Subclasses should only implement status hook and there body_of_task
    should look like::

        def body_of_task(self):
            self.run_test( APPROPRIATE_DIRECTORY )
    """
    
    no_compile_option = False

    def compiled(self):    
        if not self.__class__.no_compile_option:
            compilation_succeeded = self.compile_program()
            if not compilation_succeeded:
                vprint("%s bails out." % self.classname(), 2)
                return False
        return True

    def run_test(self, results):
        if not self.compiled():
            return

        vprint("\nRunning program:", 2)
        vprint("----------------", 2)

        test_results  = self.test.link_resources( results )

        run_command   = ( "./%s %s >& %s"
                          % ( self.test.program.get_name(), self.test.arguments, self.test.name+'.run_log' )
                          )
        
        vprint(run_command, 2)

        ## Run the test from inside the test_results directory and return
        ## to the cwd
        cwd = os.getcwd()
        os.chdir( test_results )
        os.system(run_command)
        os.chdir( cwd )

        ## Set the status and quit
        self.status_hook()
        self.test.unlink_resources( test_results )

    def status_hook(self):
        raise NotImplementedError
        
class ResultsCreationRoutine(ResultsRelatedRoutine):
    """Generates the expected results target tests.

    Before a test can be ran, it must defines its expected results. In the
    current context, all outputs (standard output and files) of a test are
    stored by the results mode in a 'pytest' subdirectory.

    The run mode -- L{RunTestRoutine} -- will thereafter be able to
    compare results obtained on a new run to the ones kept in the
    results directory.

    B{Do not modify} the results directory manually.
    """

    def body_of_task(self):
        self.run_test( Test.expected_results )

    def status_hook(self):
        self.set_status( "Succeeded" )

class RunTestRoutine(ResultsRelatedRoutine):        
    """Compares current results to expected ones.
    
    B{Note that}, before a test can be ran, it must defines its
    expected results. In the current context, all outputs (standard
    output and files) of a test are stored by the results mode in a
    'pytest' subdirectory directory.

    The run mode will thereafter be able to compare results obtained
    on a new run to the ones kept in the results directory.

    B{Do not modify} the results directory manually.
    """
    def body_of_task(self):        
        self.run_test( Test.run_results )
    
    def preprocessing(self):
        self.set_attribute( "expected_results",
                            self.test.test_results( Test.expected_results ) )
        self.set_attribute( "run_results",
                            self.test.test_results( Test.run_results ) )        
        
        if not os.path.exists( self.expected_results ):
            raise PyTestUsageError(
                "%s\n Expected results must be generated by the 'results' mode "
                "prior to any use of the 'run' mode."
                % self.test.get_path()
                )

    def status_hook(self):
        idiff  =  IntelligentDiff( self.test )
        diffs  =  idiff.diff( self.expected_results, self.run_results )
        if diffs == []:
            self.set_status( "Succeeded" )
        else:
            report_path = os.path.join( Test.run_results,
                                        self.test.get_name()+'.failed' )
            toolkit.lines_to_file( diffs, report_path )
            self.set_status( "Failed" )
        
