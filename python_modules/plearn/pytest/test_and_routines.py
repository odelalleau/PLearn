__cvs_id__ = "$Id: test_and_routines.py,v 1.17 2004/12/21 15:31:49 dorionc Exp $"

import os, string

import plearn.utilities.cvs           as     cvs
import plearn.utilities.toolkit       as     toolkit

from   plearn.tasks.Task              import *
from   plearn.utilities.verbosity     import *
from   plearn.utilities.FrozenObject  import *

## In plearn.pytest
##from   threading                      import *
from   programs                       import *
from   BasicStats                     import BasicStats
from   IntelligentDiff                import *          

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
    
    statistics = BasicStats("Test statistics")
    expected_results = os.path.join(plpath.pytest_dir, "expected_results")
    run_results = os.path.join(plpath.pytest_dir, "run_results")

    def current_stats(cls):
        return cls.statistics.current_stats()
    current_stats = classmethod(current_stats)
    
    def __init__(self, defaults=TestDefaults, **overrides):
        FrozenObject.__init__(self, defaults, overrides)        
        self.set_attribute( 'test_directory', os.getcwd() )

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
        backup = None
        if os.path.exists( results ):
            if os.path.exists( os.path.join(results, plpath.cvs_directory) ):
                answer = None
                while not answer in ['yes', 'no']:
                    answer = raw_input( "Results %s already are under version control! Are you sure\n"
                                        "you want to generate new results (yes or no)? " % results )

                if answer == 'no':
                    raise PyTestUsageError("Results creation interrupted by user")

                ## YES
                os.system( "cvs remove -Rf %s" % results )
                cvs.commit( '.',
                            'Removal of %s for new results creation.'
                            % results )                
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
        if results not in [Test.expected_results, Test.run_results]:
            raise ValueError(
                "%s.run expects its results argument to be either "
                "%s.expected_results or %s.run_results"
                % (self.classname(), self.classname(), self.classname())
                )
        
        return os.path.join( results, self.name )
        
    def run(self, results):
        os.putenv("PLEARN_DATE_TIME", "NO")
        self.sanity_check()


        test_results  = self.test_results( results )
        backup        = self.ensure_results_directory( test_results )

        self.link_resources( test_results )
        run_command   = ( "./%s %s >& %s"
                          % ( self.program.get_name(), self.arguments, self.name+'.run_log' )
                          )
        vprint(run_command, 2)

        cwd = os.getcwd()
        os.chdir( test_results )
        os.system(run_command)
        os.chdir( cwd )
        
        Resources.unlink_resources( test_results )
        os.putenv("PLEARN_DATE_TIME", "YES")

    def link_resources(self, target_directory):
        resources = []
        resources.extend( self.resources )
        resources.append( self.program.path )        
        Resources.link_resources( self.test_directory, resources, target_directory )

    def unlink_resources(self, target_directory):
        Resources.unlink_resources( target_directory )

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
            
class ResultsCreationRoutine(Routine):
    """Generates the expected results target tests.

    Before a test can be ran, it must defines its expected results. In the
    current context, all outputs (standard output and files) of a test are
    stored by the results mode in a 'pytest' subdirectory.

    The run mode -- L{RunTestRoutine} -- will thereafter be able to
    compare results obtained on a new run to the ones kept in the
    results directory.

    B{Do not modify} the results directory manually.
    """

    no_compile_option = False
    
    def body_of_task(self):
        if not ResultsCreationRoutine.no_compile_option:
            compilation_succeeded = self.compile_program()
            if not compilation_succeeded:
                vprint("Results creation bails out.", 2)
                return
            
        vprint("\nResults creation:", 2)
        vprint("-----------------", 2)
        
        self.test.run( Test.expected_results )

        mappings = { os.path.abspath(Test.expected_results): "$RESULTS" }
        mappings.update( plpath.env_mappings )
        mappings.update( Resources.name_resolution )
        plpath.process_with_mappings( Test.expected_results, mappings )
        
        vprint("", 2)
        self.set_status( "Succeeded" )

class RunTestRoutine(Routine):        
    """Compares current results to expected ones.
    
    B{Note that}, before a test can be ran, it must defines its
    expected results. In the current context, all outputs (standard
    output and files) of a test are stored by the results mode in a
    'pytest' subdirectory directory.

    The run mode will thereafter be able to compare results obtained
    on a new run to the ones kept in the results directory.

    B{Do not modify} the results directory manually.
    """

    no_compile_option = False

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
    
    def body_of_task(self):        
        if not RunTestRoutine.no_compile_option:
            compilation_succeeded = self.compile_program()
            if not compilation_succeeded:
                vprint("Running bails out.", 2)
                return
        
        vprint("\nRunning the test:", 2)
        vprint("-----------------", 2)
        
        self.test.run( Test.run_results )

        mappings = { os.path.abspath(Test.run_results): "$RESULTS" }
        mappings.update( plpath.env_mappings )
        mappings.update( Resources.name_resolution )
        plpath.process_with_mappings( Test.run_results, mappings )

        idiff  =  IntelligentDiff( self.test )
        diffs  =  idiff.diff( self.expected_results, self.run_results )
        if diffs == []:
            self.set_status( "Succeeded" )
        else:
            report_path = os.path.join( Test.run_results,
                                        self.test.get_name()+'.failed' )
            toolkit.lines_to_file( diffs, report_path )
            self.set_status( "Failed" )
