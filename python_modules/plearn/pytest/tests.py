#!/usr/bin/env python

# tests.py
# Copyright (C) 2004-2006 Christian Dorion
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

import os, shutil 
import plearn.utilities.toolkit as toolkit
from plearn.pyplearn.plearn_repr import python_repr

from core import *
from programs import *
from IntelligentDiff import *          

from plearn.utilities import pldiff
from plearn.utilities.moresh import *
from plearn.utilities.Bindings import Bindings

# Version control
from plearn.utilities import version_control
from plearn.utilities.version_control import is_under_version_control



# Eventually remove Test's static methods
EXPECTED_RESULTS = "expected_results"
RUN_RESULTS      = "run_results"
SVN_RESULTS      = "expected_results.svn"

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
                + '\n' + '(SKIPPED implies a PyTest internal error)'.rjust(rjust) )
    headerLegend = classmethod(headerLegend)

    def summary(cls):
        return str(cls._stats_book)
    summary = classmethod(summary)
    
    def summaryHeader(cls):
        return '-'.join([ s[0] for s in cls._completion_types ])
    summaryHeader = classmethod(summaryHeader)

    # Options
    status = PLOption('NEW')
    directory = PLOption(None)
    log = PLOption('')
    test = PLOption('')

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
            # Exit code management
            if status == "FAILED" and not self.test.compilationSucceeded():
                updateExitCode("COMPILATION FAILED")
            else:
                updateExitCode(status)
                
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
            if self.test.compilationSucceeded():
                return "** FAILED **"
            else:
                return "*C FAILED C*"            
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
    print repr(s)

class Test(PyTestObject):
    """Test is a class regrouping the elements that define a test for PyTest.

    For each Test instance you declare in a config file, a test will be ran
    by PyTest.

    @ivar name: The name of the Test must uniquely determine the
    test. Among others, it will be used to identify the test's results
    (.PyTest/I{name}/*_results/) and to report test informations.
    @type name: String

    @ivar description: The description must provide other users an
    insight of what exactly is the Test testing. You are encouraged
    to used triple quoted strings for indented multi-lines
    descriptions.
    @type description: String

    @ivar category: The category to which this test belongs. By default, a
    test is considered a 'General' test.

    It is not desirable to let an extensive and lengthy test as 'General',
    while one shall refrain abusive use of categories since it is likely
    that only 'General' tests will be ran before most commits...

    @type category: string
    
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

    @ivar resources: A list of resources that are used by your program
    either in the command line or directly in the code (plearn or pyplearn
    files, databases, ...).  The elements of the list must be string
    representations of the path, absolute or relative, to the resource.
    @type resources: List of Strings

    @ivar precision: The precision (absolute and relative) used when comparing
    floating numbers in the test output (default = 1e-6)
    @type precision: float

    @ivar pfileprg: The program to be used for comparing files of psave &
    vmat formats. It can be either::
        - "__program__": maps to this test's program if its compilable;
                         maps to 'plearn_tests' otherwise (default); 
        - "__plearn__": always maps to 'plearn_tests' (for when the program
                        under test is not a version of PLearn);
        - A Program (see 'program' option) instance

    @ivar disabled: If true, the test will not be ran.
    @type disabled: bool
    """
    # Class variables
    _test_count    = 0
    _log_count     = 0
    _logged_header = False

    _instances_map  = Bindings()
    _families_map   = Bindings()
    _categories_map = Bindings()

    _restrict_to = None
    _restrict_to_category = None

    def restrictTo(cls, test_name):        
        assert cls._restrict_to is None
        assert cls._restrict_to_category is None        
        logging.debug("\nRestriction to name %s"%test_name)
        cls._restrict_to = test_name
    restrictTo = classmethod(restrictTo)
    
    def restrictToCategory(cls, category):
        assert cls._restrict_to is None
        assert cls._restrict_to_category is None
        logging.debug("\nRestriction to category %s"%category)
        cls._restrict_to_category = category
    restrictToCategory = classmethod(restrictToCategory)

    # Names of test for which the results were removed from version control
    # by ensure_results_directory
    _results_vc_removed = []

    # Options
    name        = PLOption(None)
    description = PLOption('')
    category    = PLOption('General')
    program     = PLOption(None)
    arguments   = PLOption('')
    resources   = PLOption([])
    precision   = PLOption(1e-6)
    pfileprg    = PLOption("__program__")
    disabled    = PLOption(False)
    
    def _optionFormat(self, option_pair, indent_level, inner_repr):
        optname, val = option_pair
        if optname == "description" and self.description.find('\n')!=-1:
            return 'description = \"\"\"%s\"\"\"'%self.description
        return super(Test, self)._optionFormat(option_pair, indent_level, inner_repr)
        
    def __init__(self, **overrides):
        PyTestObject.__init__(self, **overrides)        

        self._directory = os.getcwd()
        self._results_directory = os.path.join(self._directory, ppath.pytest_dir, self.name)
        
        self._metaprotocol = string.replace( "PYTEST__%s__RESULTS" %
                                             self.name, ':', '_'
                                             )

        if Test._instances_map.has_key( self.name ):
            raise DuplicateName( Test._instances_map[self.name], self )

        if self.toBeNeglected():
            return
        
        # Instances
        Test._instances_map[self.name] = self

        # Families
        if Test._families_map.has_key( self.directory() ):
            Test._families_map[self.directory()].append(self)
        else:
            Test._families_map[self.directory()] = [self]

        # Categories
        if Test._categories_map.has_key( self.category ):
            Test._categories_map[self.category].append(self)
        else:
            Test._categories_map[self.category] = [self]

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
        # Checks name format
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

        # Interprets pfileprg option's value
        if self.pfileprg == "__program__":
            if isinstance(self.program, Compilable):
                self.pfileprg = self.program
            else:
                self.pfileprg = GlobalCompilableProgram(name='plearn_tests')
        elif self.pfileprg == "__plearn__":
            self.pfileprg = GlobalCompilableProgram(name='plearn_tests')
            
        assert isinstance(self.pfileprg, Compilable), """
        Option 'pfileprg': The program to be used for comparing files of psave &
        vmat formats. It can be either::
        - "__program__": maps to this test's program if its compilable;
                         maps to 'plearn_tests' otherwise (default); 
        - A Program (see 'program' option) instance
        """
            
    def compilationSucceeded(self):
        """Forwards compilation status request to the program.

        B{Note} that the call will raise an exception if the program
        is not compilable.
        """
        try:
            return self.program.compilationSucceeded()
        except AttributeError:
            return True # Non-compilable program

    def compile(self):
        """Forwards compilation request to the program.

        B{Note} that the call will raise an exception if the program
        is not compilable.
        """
        self.sanity_check()        
        self.program.compile()

    def ensureResultsDirectory(self, results_path):
        if os.path.exists( results_path ):
            if is_under_version_control( results_path ):
                answer = None
                while not answer in ['yes', 'no']:
                    answer = raw_input( "Results %s already are under version control! Are you sure "
                                        "you want to generate new results (yes or no)? " % results_path )

                if answer == 'no':
                    raise PyTestUsageError("Results creation interrupted by user")

                ## YES
                #BEFORE_UNIT_COMMIT: version_control.recursive_remove(results_path)
                #BEFORE_UNIT_COMMIT: version_control.commit(ppath.pytest_dir, 'Removal of %s for new results creation.'%results_path)
                #BEFORE_UNIT_COMMIT: ## Need to update the directory that was just committed: this is
                #BEFORE_UNIT_COMMIT: ## important e.g. with SubVersion to ensure it is up-to-date.
                #BEFORE_UNIT_COMMIT: version_control.update(ppath.pytest_dir)
                #BEFORE_UNIT_COMMIT: self._results_vc_removed.append(self.getName())

                svn_results = self.resultsDirectory(SVN_RESULTS)
                assert not os.path.exists(svn_results)
                logging.debug("os.rename(results_path, self.resultsDirectory(SVN_RESULTS))")
                os.rename(results_path, svn_results)
                logging.warning("Be sure to 'confirm' your results afterwards...")

            ## Will have been removed under svn
            if ( os.path.exists(results_path) ):
                shutil.rmtree(results_path)

        ## Make a fresh directory
        os.makedirs( results_path )

    def formatted_description(self):
        fdesc = [ "In %s"%self.directory(), "" ]
        
        if string.lstrip(self.description, ' \n') != "":
            fdesc.extend( toolkit.boxed_lines(self.description, 50, indent='    ')+[""] )

        return string.join(["    "+line for line in fdesc], '\n')
        
    def getName(self):
        return self.name
    
    def get_path(self):
        return os.path.join(
            self.directory(),
            self.name
            )

    def is_disabled(self):
        return self.disabled
    
    def resultsDirectory(self, path=''):
        assert path in ['', EXPECTED_RESULTS, RUN_RESULTS, SVN_RESULTS], path
        return os.path.join(self._results_directory, path)

    def setStatus(self, status, log=''):
        self._status.setStatus(status, log)

        statsHeader = TestStatus.summaryHeader()

        C = 6; N = 60; S = 12; H = len(statsHeader)*3
        vpformat = lambda c,n,s, h: logging.info(
            c.ljust(C) + n.ljust(N) + s.center(S) + h.center(H))        
        
        if self._status.isCompleted():
            if not self._logged_header:
                logging.info(TestStatus.headerLegend(C+N+S+H)+'\n')
                vpformat("N/%d"%self._test_count, "Test Name", "Status", statsHeader)
                self.__class__._logged_header = True

            if self._log_count%5 == 0:
                logging.info("-"*(C+N+S+H))
                
            self.__class__._log_count += 1
            vpformat(str(self._log_count), self.getName(),
                     str(self._status), TestStatus.summary())

    def toBeNeglected(self):
        neglect = False
        if self._restrict_to is not None:            
            neglect = (self._restrict_to != self.name)
            if neglect:
                logging.debug(
                    "\nNeglecting %s due to name restriction: %s"
                    %(self.name,self._restrict_to)
                    )

        elif self._restrict_to_category is not None:
            neglect = self._restrict_to_category != self.category
            if neglect:
                logging.debug(
                    "\nNeglecting %s due to category restriction: %s"
                    %(self.name,self._restrict_to_category)
                    )

        return neglect
        
    def linkResources(self, results_path):
        """Sets up all appropriate links and environment variables required by a test.

        @param results: Either EXPECTED_RESULTS or RUN_RESULTS

        @returns: Returns the test's results directory path.
        @rtype: StringType.
        """
        self.sanity_check()

        try:
            # Validates the results value and return this test's results
            # path.
            test_results = self.resultsDirectory(results_path)
        except AssertionError:
            # linkResources may be used from outside the test's standard
            # structure (see IntelligentDiff for an example)
            test_results = results_path
            
        self.ensureResultsDirectory(test_results)

        ## Link 'physical' resources
        Resources.link_resources(self.directory(), self.resources, test_results)

        ## What remains of this method is used to make the following
        ## binding visible to the test program. It must be removed after
        ## the test ran (done in unlink_resources()).        
        metapath = os.path.abspath(test_results)
        ppath.add_binding(self.metaprotocol(), metapath)

        ## This PLEARN_CONFIGS directory specific to this test::
        ##     - It must not be in the test_results (diff...) BUT;
        ##     - It must not be in a directory shared with other test's (multithread access...)
        internal_plearn_configs =\
            os.path.join(os.path.abspath(self._results_directory), ".plearn")

        ## Ensure that the directory exists
        if not os.path.exists(internal_plearn_configs):
            os.mkdir( internal_plearn_configs )

        ## Create a new ppath.config file
        internal_ppath_config = os.path.join( internal_plearn_configs, "ppath.config" )
        internal_config_file  = open(internal_ppath_config, "w")

        ## This method writes the previously defined and the added
        ## metaprotocol to the internal_ppath_config
        ppath.write_bindings( internal_config_file.write )

        ## This process now consider the internal_plearn_configs
        os.environ[ 'PLEARN_CONFIGS'   ] =  internal_plearn_configs
        os.environ[ 'PLEARN_DATE_TIME' ] = 'NO'

        ## Returns the test's results directory path
        logging.debug("Resources linked in %s."%test_results)
        return test_results

    def unlinkResources(self, test_results):
        """Removes links made by and environment variables set by I{linkResources}.

        @param test_results: B{Must} be the value returned by I{linkResources}.
        """
        Resources.unlink_resources( self.resources, test_results )
        ppath.remove_binding(self.metaprotocol())
        os.environ[ 'PLEARN_DATE_TIME' ] = 'YES'
        logging.debug("Resources unlinked.")
        
class Routine( PyTestObject ):
    test = PLOption(None)

    def __init__(self, **overrides):        
        PyTestObject.__init__(self, **overrides) 
        os.chdir( self.test.directory() )
        
    def compile_program(self):
        if not isinstance(self.test.program, Compilable):
            return True

        logging.debug("\nCompilation:")
        logging.debug("------------")
        
        self.test.compile()
        if not self.test.compilationSucceeded():
            logging.debug("Compilation failed.")
            self.test.setStatus("FAILED", "Compilation failed.")
            return False
        logging.debug("Compilation succeedded.")
        return True

    def start(self):
        raise NotImplementedError("Abstract method: please implement.")
                    
class CompilationRoutine(Routine):
    """Launches the compilation of target tests' compilable files."""    
    def start(self):
        if self.compile_program():
            self.test.setStatus("PASSED")


class ResultsRelatedRoutine(Routine):
    """Base class for ResultsCreationRoutine && RunTestRoutine.

    Subclasses should only implement status hook and there routine
    should look like::

        def start(self):
            self.run_test( APPROPRIATE_DIRECTORY )
    """    
    no_compile_option = PLOption(False)
    run_log = PLOption('RUN.log')

    def clean_cwd( self ):
        dirlist = os.listdir( os.getcwd() )
        for fname in dirlist:
            if fname.endswith( '.pyc' ):
                os.remove( fname )

    def compiled(self):    
        if not self.__class__.no_compile_option:
            compilation_succeeded = self.compile_program()
            if not compilation_succeeded:
                logging.debug("%s bails out." % self.classname())
                return False
        return True

    def run_test(self, results):
        if not self.compiled():
            return

        logging.debug("\nRunning program:")
        logging.debug("----------------")

        test_results  = self.test.linkResources(results)

        #run_command   = ( "./%s %s >& %s"
        #                  % ( self.test.program.getName(), self.test.arguments, self.run_log )
        #                  )        
        run_command   = ( "%s %s >& %s" \
                         % ( os.path.join(os.path.dirname(self.test.program.get_path()),
                                          self.test.program.getName()),
                             self.test.arguments, self.run_log )
                        )


        ## Run the test from inside the test_results directory and return
        ## to the cwd
        pushd(test_results)
        logging.debug("In %s: %s"%(os.getcwd(), run_command))
        os.system(run_command)
        self.clean_cwd()
        popd()

        ## Unlink all resources.
        self.test.unlinkResources(test_results)

        ## Set the status and quit
        self.status_hook()

    def status_hook(self):
        """Overriden by subclasses to set the test's status."""
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
    def start(self):
        self.run_test(EXPECTED_RESULTS)
        if Test._results_vc_removed:
            logging.critical(
                "\n*** Results were changed for the following test%s: %s. "
                "Once you've checked the results validity, DO NOT FORGET to do a 'pytest vc_add'."
                % (toolkit.plural(len(Test._results_vc_removed)), ', '.join(Test._results_vc_removed)),
                )

    def status_hook(self):
        self.test.setStatus("PASSED")

class RunTestRoutine( ResultsRelatedRoutine ):        
    """Compares current results to expected ones.
    
    B{Note that}, before a test can be ran, it its expected results must be
    defined. All outputs (standard output and files) of a test are stored
    by the results mode in a .PyTest subdirectory.

    The run mode will thereafter be able to compare results obtained
    on a new run to the ones kept in the expected results directory.

    B{Do not modify} the results directory manually.
    """
    expected_results = PLOption(None)
    run_results      = PLOption(None)
    failure_log      = PLOption("FAILURE.log")

    def __init__( self, **overrides ):
        ResultsRelatedRoutine.__init__(self, **overrides)
        self.expected_results = self.test.resultsDirectory( EXPECTED_RESULTS )
        self.run_results      = self.test.resultsDirectory( RUN_RESULTS )
        if os.path.exists(self.run_results):
            shutil.rmtree(self.run_results)

        self.report_path = os.path.join(self.test.resultsDirectory(), self.failure_log)
        if os.path.exists(self.report_path):
            os.remove(self.report_path)
        
        if ( not self.test.disabled and
             not os.path.exists(self.expected_results) ):
            raise PyTestUsageError(
                "%s\n Expected results must be generated by the 'results' mode "
                "prior to any use of the 'run' mode."
                % self.test.get_path()
                )

    def start(self):        
        self.run_test(RUN_RESULTS)
    
    def status_hook(self):
        idiff = IntelligentDiff( self.test )
        diffs = idiff.diff(self.expected_results, self.run_results)

        # Developping for pldiff -- Issues:
        #   1) Managing Resources
        #
        # pushd(self.test.resultsDirectory())
        # logging.debug("pldiff in %s"%os.getcwd())
        # 
        # program_name = self.test.pfileprg.name
        # compile_errors = not self.test.pfileprg.compile()
        # if compile_errors:
        #     diffs = [ "COMPILATION ERROR on test's pfileprg: %s\n"%program_name ]
        # else:
        #     diffs = pldiff.pldiff(self.expected_results, self.run_results,
        #                           self.test.precision, program_name)
        # popd()
            
        logging.debug("diffs: %s"%str(diffs))
        if diffs == []:
            self.test.setStatus("PASSED")
        else:
            toolkit.lines_to_file(diffs, self.report_path)
            self.test.setStatus("FAILED", log="Should contain diff lines!")
        
