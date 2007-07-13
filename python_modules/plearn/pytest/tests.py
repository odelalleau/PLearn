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

from plearn.utilities import ppath 
from plearn.utilities import pldiff
from plearn.utilities import moresh 
from plearn.utilities import toolkit
from plearn.utilities.Bindings import Bindings
from plearn.pyplearn.plearn_repr import python_repr

# To be removed
from plearn.utilities.moresh import *

# Version control
from plearn.utilities import version_control
from plearn.utilities.version_control import is_under_version_control

# PyTest Modules
from core import *
from programs import *


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
            % ( test1.getPath(), test2.getPath() )
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

    def nPassed(cls):
        return cls._stats_book["PASSED"]
    nPassed = classmethod(nPassed)

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

class Resources(core.PyTestObject):
    md5_mappings    = PLOption({})
    name_resolution = PLOption({})
    
    # def memorize(self, abspath, fname):
    #     if not self.name_resolution.has_key(abspath):
    #         self.name_resolution[abspath] = fname        

    ## Create a link from target to resource.
    def single_link(self, path_to, resource, target_dir, must_exist=True):
        ## Paths to the resource and target files
        resource_path = ppath.ppath(resource)
        target_path   = target_dir

        ## Absolute versions
        if not os.path.isabs( resource_path ):
            resource_path = os.path.join( path_to, resource )
            target_path = os.path.join( path_to, target_dir, resource )
        else:
            target_path = os.path.join(target_dir, os.path.basename(resource))
            assert not os.path.exists( target_path ), target_path

        
        ## Linking
        linked = False
        if os.path.exists( resource_path ):
            toolkit.symlink(resource_path, target_path)
            linked = True

        elif must_exist:
            raise core.PyTestUsageError(
                "In %s: %s used as a resource but path does not exist."
                % ( os.getcwd(), resource )
                )

        ## Mapping both to the same variable
        # self.memorize( resource_path, resource )                
        # self.memorize( target_path, resource )                

        if linked:
            return (resource_path, target_path)
        else:
            return ()

    ## Class methods
    def link_resources(self, path_to, resources, target_dir): 
        resources_to_append = []
        for resource in resources:
            self.single_link( path_to, resource, target_dir )
                        
            if toolkit.isvmat( resource ):
                metadatadir = resource + '.metadata'
                if metadatadir not in resources:
                    link_result = self.single_link( path_to, metadatadir,
                                                   target_dir,  False )
                    if link_result:
                        ## Link has been successfully performed: we must add the
                        ## metadata directory to the list of resources, so that it
                        ## is correctly unlinked at a later time.
                        resources_to_append.append(metadatadir)
        resources.extend(resources_to_append)

    def md5sum(self, path_to_ressource):
        if md5_mappings.has_keys(path_to_ressource):
            return md5_mappings[path_to_ressource]
        
        md5 = toolkit.command_output( 'md5sum %s'
                                      % path_to_ressource)
        md5_mappings[path_to_ressource] = md5
        return md5

    def unlink_resources(self, resources, target_dir):
        for resource in resources:
            res = os.path.basename(resource)
            path = os.path.join(target_dir, res)
            if os.path.islink( path ):
                logging.debug("Removing link: %s."%path)
                os.remove( path )
            elif os.path.isfile( path ):
                logging.debug("Removing file: %s."%path)
                os.remove( path )
            elif os.path.isdir( path ):
                logging.debug("Removing directory: %s." % path)
                shutil.rmtree( path )

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
    
    @ivar program: The program to be run by the Test. The program's name
    PRGNAME is used to lookup for the program in the following manner::

      1) Look for a local program named PRGNAME
      2) Look for a plearn-like command (plearn, plearn_tests, ...) named PRGNAME
      3) Call 'which PRGNAME'
      4) Fail

    Compilable program should provide the keyword argument 'compiler'
    mapping to a string interpreted as the compiler name (e.g.
    "compiler = 'pymake'"). If no compiler is provided while the program is
    believed to be compilable, 'pymake' will be assigned by
    default. Arguments to be forwarded to the compiler can be provided as a
    string through the 'compile_options' keyword argument.  @type program:
    L{Program}

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
        - None: if you are sure no files are to be compared.

    @ivar ignored_files_re: Default behaviour of a test is to compare all
    files created by running the test. In some case, one may prefer some of
    these files to be ignored.
    @type ignored_files_re: list of regular expressions

    @ivar disabled: If true, the test will not be ran.
    @type disabled: bool
    """
    #######  Options  #############################################################

    name             = PLOption(None)
    description      = PLOption('')
    category         = PLOption('General')
    program          = PLOption(None)
    arguments        = PLOption('')
    resources        = PLOption([])
    precision        = PLOption(1e-6)
    pfileprg         = PLOption("__program__")
    ignored_files_re = PLOption([])
    disabled         = PLOption(False)
    

    #######  Class Variables and Methods  #########################################
    # NB: Class variables could now all be 'public', laziness is why it isn't so...
    
    _test_count    = 0
    _log_count     = 0
    _logged_header = False

    _instances_map  = Bindings()
    _families_map   = Bindings()
    _categories_map = Bindings()

    # Do not prompt user for results creation even if results already are
    # under version control
    force_results_creation = False

    _restrict_to = None
    _restrict_to_category = None

    def missingExpectedTests(cls):
        missing = []
        if cls._restrict_to is not None:
            for test_name in cls._restrict_to:
                if test_name not in cls._instances_map:
                    missing.append(test_name)
        return missing
    missingExpectedTests = classmethod(missingExpectedTests)

    def restrictTo(cls, test_name):        
        assert cls._restrict_to is None
        assert cls._restrict_to_category is None        
        logging.debug("\nRestriction to name %s"%test_name)
        cls._restrict_to = test_name.split(',')
        return cls._restrict_to[:]
    restrictTo = classmethod(restrictTo)
    
    def restrictToCategory(cls, category):
        assert cls._restrict_to is None
        assert cls._restrict_to_category is None
        logging.debug("\nRestriction to category %s"%category)
        cls._restrict_to_category = category.split(',')
        return cls._restrict_to_category[:]
    restrictToCategory = classmethod(restrictToCategory)

    #######  Instances' Methods  ##################################################

    def _optionFormat(self, option_pair, indent_level, inner_repr):
        optname, val = option_pair
        if val is None:
            return "%s = None"%optname
        
        elif optname == "description" and self.description.find('\n')!=-1:
            return 'description = \"\"\"%s\"\"\"'%self.description

        elif optname == "ignored_files_re" and len(self.ignored_files_re)==0:
            return ""

        else:
            return super(Test, self)._optionFormat(option_pair, indent_level, inner_repr)
        
    def __init__(self, **overrides):
        PyTestObject.__init__(self, **overrides)        

        self._directory = os.getcwd()
        self._results_directory = os.path.join(self._directory, ppath.pytest_dir, self.name)
        
        self._metaprotocol = ("PYTEST__%s__RESULTS"%self.name).replace(':', '_')
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
        
        check = ( self.name.find(' ') + self.name.find('/') +
                  self.name.find('<') + self.name.find('>') )

        if check != -4:
            raise PyTestUsageError(
                "%s\n Test.name should contain none of the following chars: "
                "' ', '/', '<', '>'."
                % self.getPath()
                )

        # Interprets pfileprg option's value
        if self.pfileprg == "__program__":
            if self.program.isCompilable():
                self.pfileprg = self.program
            else:
                self.pfileprg = Program(name='plearn_tests', compiler="pymake")
        elif self.pfileprg == "__plearn__":
            self.pfileprg = Program(name='plearn_tests', compiler="pymake")

        try:
            if self.pfileprg is not None:
                assert self.pfileprg.isCompilable()
        except AssertionError, err:
            raise core.PyTestUsageError("""In test %s:
        Option 'pfileprg': The program to be used for comparing files of psave &
        vmat formats. It can be either::
        - "__program__": maps to this test's program if its compilable;
                         maps to 'plearn_tests' otherwise (default);
        - "__plearn__": always maps to 'plearn_tests' (for when the program
                        under test is not a version of PLearn);
        - A Program (see 'program' option) instance
        - None: if you are sure no files are to be compared.

        Received %s
        """%(self.name, self.pfileprg)
            )
            
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
        if not os.path.exists(self.resultsDirectory()):
            os.makedirs( moresh.relative_path(self.resultsDirectory()) )
        self.program.compile(self.resultsDirectory())

    def ensureResultsDirectory(self, results_path):
        if os.path.exists( results_path ):
            if is_under_version_control( results_path ):
                answer = None
                if Test.force_results_creation:
                    # Do not ask user
                    answer = 'yes'
                    
                while not answer in ['yes', 'no']:
                    answer = raw_input( "Results %s already are under version control! Are you sure "
                                        "you want to generate new results (yes or no)? " % results_path )

                if answer == 'no':
                    raise PyTestUsageError("Results creation interrupted by user")

                # User answered yes
                svn_results = self.resultsDirectory(SVN_RESULTS)
                assert not os.path.exists(svn_results)
                logging.debug("os.rename(results_path, self.resultsDirectory(SVN_RESULTS))")
                os.rename(results_path, svn_results)

            ## Will have been removed under svn
            if ( os.path.exists(results_path) ):
                shutil.rmtree(results_path)

        ## Make a fresh directory
        os.makedirs( results_path )

    def formatted_description(self):
        fdesc = [ "In %s"%self.directory(), "" ]
        
        if self.description.lstrip(' \n') != "":
            fdesc.extend( toolkit.boxed_lines(self.description, 50, indent='    ')+[""] )

        return '\n'.join(["    "+line for line in fdesc])
        
    def getName(self):
        return self.name
    
    def getPath(self):
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

        # Hackish hardcoded display summing to 80...
        C = 6; S = len("** FAILED **")+2; H = len(statsHeader)+3
        N = 80 - (C+S+H); 
        def vpformat(c,n,s, h):
            if len(n) < N:
                logging.info( c.ljust(C)+n.ljust(N)+s.center(S)+h.center(H) )
            else:
                logging.info( c.ljust(C)+n.ljust(N) )
                logging.info( ((C+N)*" ")+s.center(S)+h.center(H) )
        
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
            neglect = (self.name not in self._restrict_to)
            if neglect:
                logging.debug(
                    "\nNeglecting %s due to name restriction: %s"
                    %(self.name,self._restrict_to)
                    )

        elif self._restrict_to_category is not None:
            neglect = (self.category not in self._restrict_to_category)
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
            # linkResources may be used from outside the test's standard structure
            test_results = results_path
            
        self.ensureResultsDirectory(test_results)

        ## Link 'physical' resources
        self._resources_obj = Resources()
        self._resources_obj.link_resources(self.directory(), self.resources, test_results)

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
        os.environ['PLEARN_CONFIGS'] = internal_plearn_configs

        ## Returns the test's results directory path
        logging.debug("Resources linked in %s."%test_results)
        logging.debug(self._resources_obj)
        return test_results

    def unlinkResources(self, test_results):
        """Removes links made by and environment variables set by I{linkResources}.

        @param test_results: B{Must} be the value returned by I{linkResources}.
        """
        self._resources_obj.unlink_resources( self.resources, test_results )
        del self._resources_obj
        
        ppath.remove_binding(self.metaprotocol())

        logging.debug("Reassinging PLEARN_CONFIGS to %s"%ppath.plearn_configs)
        os.environ['PLEARN_CONFIGS'] = ppath.plearn_configs
        del self._overriden_plearn_configs
        
        logging.debug("Resources unlinked.")
        
class Routine( PyTestObject ):
    test = PLOption(None)

    def __init__(self, **overrides):        
        PyTestObject.__init__(self, **overrides) 
        os.chdir( self.test.directory() )
        
    def compileProgram(self):
        if not self.test.program.isCompilable():
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
        if self.compileProgram():
            self.test.setStatus("PASSED")


class ResultsRelatedRoutine(Routine):
    """Base class for ResultsCreationRoutine && RunTestRoutine.

    Subclasses should only implement status hook and there routine
    should look like::

        def start(self):
            self.run_test( APPROPRIATE_DIRECTORY )
    """
    # Options
    run_log = PLOption('RUN.log')
    
    def clean_cwd( self ):
        dirlist = os.listdir( os.getcwd() )
        for fname in dirlist:
            if fname.endswith( '.pyc' ):
                os.remove( fname )

    def run_test(self, results):
        if not self.compileProgram():
            logging.debug("%s bails out." % self.classname())
            return 

        logging.debug("\nRunning program:")
        logging.debug("----------------")

        test_results  = self.test.linkResources(results)

        ## Run the test from inside the test_results directory and return
        ## to the cwd
        pushd(test_results)
        self.test.program.invoke(self.test.arguments, self.run_log)
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
                % self.test.getPath()
                )

    def start(self):
        self.run_test(RUN_RESULTS)
    
    def status_hook(self):
        # Comparing results with pldiff
        pushd(self.test.resultsDirectory())
        logging.debug("pldiff in %s"%os.getcwd())

        diffs = []
        plearn_exec = None
        if self.test.pfileprg is not None:
            if self.test.pfileprg.compile():
                plearn_exec = self.test.pfileprg.getInternalExecPath()
            else:
                diffs = [ "COMPILATION ERROR on test's pfileprg: %s\n"%self.test.pfileprg.name ]                
                        
        diffs.extend(
            pldiff.pldiff(self.expected_results, self.run_results,
                          self.test.precision, plearn_exec, self.test.ignored_files_re))
        popd()

        # Set status
        logging.debug("diffs: %s"%str(diffs))
        if diffs == []:
            self.test.setStatus("PASSED")
        else:
            toolkit.lines_to_file(diffs, self.report_path)
            self.test.setStatus("FAILED", log="Should contain diff lines!")
        
