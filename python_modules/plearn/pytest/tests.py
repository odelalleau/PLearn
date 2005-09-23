__version_id__ = "$Id:  $"

import os, shutil 
import plearn.utilities.toolkit as toolkit
from plearn.pyplearn.plearn_repr import python_repr

from programs                   import *
from PyTestCore                 import *
# STATS: from BasicStats                 import BasicStats
from IntelligentDiff            import *          
from plearn.utilities.verbosity import *
from plearn.utilities.version_control import is_under_version_control

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

# STATS: def print_stats():
# STATS:     Test._statistics.print_stats()


class DuplicateName( PyTestUsageError ):
    def __init__(self, test1, test2):
        PyTestUsageError.__init__(
            self, "Two tests have the same name: %s and %s."
            % ( test1.get_path(), test2.get_path() )
            )        

class StatsBook(dict):
    def __init__(self, keys):
        self._keys = keys
        for s in keys:
            self[s] = 0

    def __str__(self):
        return '-'.join([ str(self[k]) for k in self._keys ])

    def inc(self, key):
        self[key] += 1 
        
class TestStatus(PyPLearnObject):
    _logfile_basename = 'STATUS.log'
    _completion_types = [ "PASSED", "DISABLED", "FAILED", "SKIPPED" ]
    _status_types     = [ "NEW", "STARTED" ] + _completion_types
    _stats_book       = StatsBook(_completion_types)    

    def headerLegend(cls, rjust=0):
        return ('   '.join([ "%s: %s"%(s[0],s) for s in cls._completion_types ]).rjust(rjust)
                + '\n' + '(SKIPPED implies a PyTest internal error!)'.rjust(rjust) )
    headerLegend = classmethod(headerLegend)

    def summary(cls):
        return str(cls._stats_book)
    summary = classmethod(summary)
    
    def summaryHeader(cls):
        return '-'.join([ s[0] for s in cls._completion_types ])
    summaryHeader = classmethod(summaryHeader)

    # Options
    status = 'NEW'
    directory = None
    log = ''
    test = ''

    def __init__(self, test):        
        directory = test.directory()
        super(TestStatus, self).__init__(directory=directory, test=test)

        self._test = test
        self._logfname = os.path.join(self.directory,self._logfile_basename)
        if os.path.exists(self._logfname):
            os.remove(self._logfname)

    def isCompleted(self):        
        return self.status in self._completion_types

    def setStatus(self, status, log=None):
        if self.isCompleted():
            raise RuntimeError('Attempt to set status on a completed test.')
            
        if not status in self._status_types:
            raise ValueError(status)

        self.status = status
        if self.isCompleted():
            self.log = log
            self._stats_book.inc(status)
            # logfile = open(self._logfname, 'w')
            # logfile.write(python_repr(self))
            # logfile.write('\n')
            # logfile.close()
        else:
            assert log is None

    def __str__(self):
        if self.status == "FAILED":
            return "** FAILED **"
        return self.status
        
if __name__ == '__main__':
    class T:
        def directory(self):
            return 'BID'
    t = T()
    
    s = TestStatus(t)
    print s
    print repr(s)
    s.setStatus('SKIPPED')

class Test(PyTestObject):
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

    @ivar program: The program to be run by the Test. The L{Program}
    class currently has four subclasses: L{LocalProgram},
    L{GlobalProgram}, L{LocalCompilableProgram} and
    L{GlobalCompilableProgram}. The I{Local} prefix specifies that the
    program executable should be found in the same directory than the
    I{pytest.config} file, while I{Global} means that the program is a
    global command. Currently, this mechanism only support commands that
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
    # Class variables
    _test_count    = 0
    _log_count     = 0
    _logged_header = False
    
    # Options
    name        = PLOption(None)
    description = PLOption('')
    program     = PLOption(None)
    arguments   = PLOption('')
    resources   = PLOption([])
    disabled    = PLOption(False)

    ## Internal attributes
    _directory        = None
    _metaprotocol     = None
        
    _instances_map    = {}
    _families_map     = {}
    
    # STATS: _statistics       = BasicStats("Test statistics")
    _expected_results = os.path.join(ppath.pytest_dir, "expected_results")
    _run_results      = os.path.join(ppath.pytest_dir, "run_results")

## STATS:     def current_stats(cls):
## STATS:         return cls._statistics.current_stats()
## STATS:     current_stats = classmethod(current_stats)
    
    def __init__(self, **overrides):
        PyTestObject.__init__(self, **overrides)        

        self._directory = os.getcwd()
        self._metaprotocol = string.replace( "PYTEST__%s__RESULTS" %
                                             self.name, ':', '_'
                                             )

        if Test._instances_map.has_key( self.name ):
            raise DuplicateName( Test._instances_map[self.name], self )
        else:
            # STATS: Test._statistics.new_test( self.directory(), self.name )
            Test._instances_map[self.name] = self
            if Test._families_map.has_key( self.directory() ):
                Test._families_map[self.directory()].append(self)
            else:
                Test._families_map[self.directory()] = [self]

        # Associate a status to that test and count it
        self.__class__._test_count += 1
        self._status = TestStatus(self)


    ## Accessor
    def directory( self ):
        return self._directory

    ## Accessor
    def metaprotocol( self ):
        return self._metaprotocol

    def sanity_check(self):
        if self.name == '':
            raise PyTestUsageError(                
                "Test must be named. Directory %s contains an unnamed test." 
                % self.directory()
                )
        
        check = ( string.find(self.name, ' ') +
                  string.find(self.name, '/') +
                  string.find(self.name, '<') +
                  string.find(self.name, '>') 
                  )

        if check != -4:
            raise PyTestUsageError(
                "%s\n Test.name should contain none of the following chars: "
                "' ', '/', '<', '>'."
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
                version_control.commit( '.', 'Removal of %s for new results creation.'%results )

            ## Will have been removed under svn
            if ( os.path.exists( results ) ):
                shutil.rmtree( results )

        ## Make a fresh directory
        os.makedirs( results )

    def formatted_description(self):
        fdesc = [ "In %s"%self.directory(), "" ]
        
        if string.lstrip(self.description, ' \n') != "":
            fdesc.extend( toolkit.boxed_lines(self.description, 50, indent='    ')+[""] )

        return string.join(["    "+line for line in fdesc], '\n')
        
    def get_name(self):
        return self.name
    
    def get_path(self):
        return os.path.join(
            self.directory(),
            self.name
            )

    def is_disabled(self):
        return self.disabled
    
    def setStatus(self, status):
        self._status.setStatus(status)

        statsHeader = TestStatus.summaryHeader()

        C = 6; N = 60; S = 12; H = len(statsHeader)*3
        vpformat = lambda c,n,s, h: vprint(
            c.ljust(C) + n.ljust(N) + s.center(S) + h.center(H),
            1 )        
        
        if self._status.isCompleted():
            if not self._logged_header:
                vprint(TestStatus.headerLegend(C+N+S+H)+'\n', 1)
                vpformat("N/%d"%self._test_count, "Test Name", "Status", statsHeader)
                self.__class__._logged_header = True

            if self._log_count%5 == 0:
                vprint("-"*(C+N+S+H), 1)
                
            self.__class__._log_count += 1
            vpformat(str(self._log_count), self.get_name(),
                     str(self._status), TestStatus.summary())

        # STATS: if self._status.isCompleted():
        # STATS:     self._statistics.set_status( self.get_name(), TestStatus.tmpMapper(status) )

    def test_results(self, results):
        if results in [Test._expected_results, Test._run_results]:
            return os.path.join( results, self.name )
        return results
        
    def link_resources(self, results):
        """Sets up all appropriate links and environment variables required by a test.

        @param results: Either Test._expected_results or Test._run_results
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
        resources.append( self.program._path )        
        Resources.link_resources( self.directory(), resources, test_results )

        ## What remains of this method is used to make the following
        ## binding visible to the test program. It must be removed after
        ## the test ran (done in unlink_resources()).        
        metapath = os.path.abspath(test_results)
        ppath.add_binding( self.metaprotocol(), metapath )

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
        ppath.remove_binding( self.metaprotocol() )
        os.putenv( "PLEARN_DATE_TIME", "YES" )

class Routine( PyTestObject ):
    _report_traceback = False

    test            = PLOption(None)
    completion_hook = PLOption(None)
        
    def __init__( self, **overrides ):        
        PyTestObject.__init__( self, **overrides ) 
        os.chdir( self.test.directory() )
        
    def compile_program(self):
        if not isinstance(self.test.program, Compilable):
            return True

        vprint("\nCompilation:", 2)
        vprint("------------", 2)
        
        self.test.compile()
        if not self.test.compilation_succeeded():
            vprint("Compilation failed.", 2)
            self.test.setStatus("FAILED")
            return False
        vprint("Compilation succeedded.", 2)
        return True

    ## Overrides run and succeeded
    def start(self):
        try:
            if self.test.is_disabled():
                vprint("Test %s is disabled." % self.test.name, 2)
                self.test.setStatus("DISABLED")
                self.signal_completion()
            else:                
                self.routine()

        except PyTestUsageError, e: 
            if Routine._report_traceback:
                raise
            else:
                e.print_error()
            self.test.setStatus("SKIPPED")            
            self.signal_completion()

    def signal_completion(self):
        tname = self.test.get_name()
        if self.completion_hook is not None:
            self.completion_hook( self )
                    
class CompilationRoutine(Routine):
    """Launches the compilation of target tests' compilable files."""    
    def routine(self):
        if self.compile_program():
            self.test.setStatus( "PASSED" )


class ResultsRelatedRoutine(Routine):
    """Base class for ResultsCreationRoutine && RunTestRoutine.

    Subclasses should only implement status hook and there routine
    should look like::

        def routine(self):
            self.run_test( APPROPRIATE_DIRECTORY )
    """    
    no_compile_option = PLOption(False)

    def clean_cwd( self ):
        dirlist = os.listdir( os.getcwd() )
        for fname in dirlist:
            if fname.endswith( '.pyc' ):
                os.remove( fname )

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
        self.clean_cwd( )
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
        self.test.setStatus( "PASSED" )

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
    expected_results = PLOption(None)
    run_results      = PLOption(None)

    def __init__( self, **overrides ):
        ResultsRelatedRoutine.__init__( self, **overrides )
        self.expected_results = self.test.test_results( Test._expected_results )
        self.run_results      = self.test.test_results( Test._run_results )
        
        if ( not self.test.disabled and
             not os.path.exists( self.expected_results ) ):
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
            self.test.setStatus( "PASSED" )
        else:
            report_path = os.path.join( Test._run_results,
                                        self.test.get_name()+'.failed' )
            toolkit.lines_to_file( diffs, report_path )
            self.test.setStatus( "FAILED" )
        
