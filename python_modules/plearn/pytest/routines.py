__cvs_id__ = "$Id: routines.py,v 1.1 2005/03/11 02:49:05 dorionc Exp $"

import os 
import plearn.utilities.toolkit as toolkit

from   programs                       import *
from   BasicStats                     import BasicStats
from   IntelligentDiff                import *          
from   plearn.utilities.verbosity     import *
from   plearn.pyplearn.PyPLearnObject import *

## New
from Test import *

__all__ = [
    ## Functions
    'config_file_path', 'print_stats', 

    ## Exceptions
    ## Classes
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
    Test._statistics.print_stats()

## Old (from tasks): Upgrade!!!
class TaskStatus:
    completion_types = ["Succeeded", "Failed", "Skipped"]
    status_types = ["New", "Ongoing"]+completion_types

    def __init__(self, status="New"):
        self.set_status(status)

    def set_status(self, status):
        if not status in self.status_types:
            raise ValueError(status)
        self.status = status

    def is_completed(self):
        return self.status in self.completion_types

    def __str__(self):
        return self.status

    def __repr__(self):
        return str(self)

class Routine( PyPLearnObject ):
    _report_traceback = False

    class Defaults:
        test = None
        completion_hook             = None
        status                      = TaskStatus()
        
    def __init__( self, **overrides ):
        PyPLearnObject.__init__( self, **overrides ) 
        os.chdir( self.test.directory() )
        
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
    def start(self):
        self.format_n_print("LAUCHED %s" % self.classname())
        self.format_n_print("%s" % self.test.get_name())
        vprint(self.test.formatted_description(), 1)
        
        try:
            if self.test.is_disabled():
                vprint("Test %s is disabled." % self.test.name, 2)
                self.set_status("Skipped")
                self.signal_completion()
            else:                
                self.routine()

        except PyTestUsageError, e: 
            if Routine._report_traceback:
                raise
            else:
                e.print_error()
            self.set_status("Skipped")            
            self.signal_completion()

    def set_status(self, status):
        self.status.set_status(status)

        if status in TaskStatus.completion_types:
            Test._statistics.set_status( self.test.get_name(), status )

    def signal_completion(self):
        tname            = self.test.get_name()
        self.format_n_print( "FINISHED %s" % self.classname() )
        self.format_n_print( "%s -- %s" % (tname, str(self.status)) )
        self.format_n_print( Test.current_stats() )
        vprint("\n", 1)

        if self.completion_hook is not None:
            self.completion_hook( self )
                    
class CompilationRoutine(Routine):
    """Launches the compilation of target tests' compilable files."""    
    def routine(self):
        if self.compile_program():
            self.set_status( "Succeeded" )


class ResultsRelatedRoutine(Routine):
    """Base class for ResultsCreationRoutine && RunTestRoutine.

    Subclasses should only implement status hook and there routine
    should look like::

        def routine(self):
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

    def routine(self):
        self.run_test( Test._expected_results )

    def status_hook(self):
        self.set_status( "Succeeded" )

class RunTestRoutine( ResultsRelatedRoutine ):        
    """Compares current results to expected ones.
    
    B{Note that}, before a test can be ran, it must defines its
    expected results. In the current context, all outputs (standard
    output and files) of a test are stored by the results mode in a
    'pytest' subdirectory directory.

    The run mode will thereafter be able to compare results obtained
    on a new run to the ones kept in the results directory.

    B{Do not modify} the results directory manually.
    """
    class Defaults( ResultsRelatedRoutine.Defaults ):
        expected_results = None
        run_results      = None

    def __init__( self, **overrides ):
        ResultsRelatedRoutine.__init__( self, **overrides )
        self.expected_results = self.test.test_results( Test._expected_results )
        self.run_results      = self.test.test_results( Test._run_results )
        
        if ( not self.test.disabled and
             not os.path.exists( self.expected_results ) ):
            raw_input( (os.getcwd(), self.expected_results) )
            raise PyTestUsageError(
                "%s\n Expected results must be generated by the 'results' mode "
                "prior to any use of the 'run' mode."
                % self.test.get_path()
                )

    def routine(self):        
        self.run_test( Test._run_results )
    
    def status_hook(self):
        idiff  =  IntelligentDiff( self.test )
        diffs  =  idiff.diff( self.expected_results, self.run_results )
        if diffs == []:
            self.set_status( "Succeeded" )
        else:
            report_path = os.path.join( Test._run_results,
                                        self.test.get_name()+'.failed' )
            toolkit.lines_to_file( diffs, report_path )
            self.set_status( "Failed" )
        
