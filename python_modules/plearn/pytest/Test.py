import os, shutil, string
import plearn.utilities.ppath           as ppath
import plearn.utilities.version_control as version_control

from BasicStats                       import *
from IntelligentDiff                  import *
from PyTestUsageError                 import *
from plearn.pyplearn.PyPLearnObject   import *
from plearn.utilities.version_control import is_under_version_control

__all__ = [ "Test" ]

class DuplicateName( PyTestUsageError ):
    def __init__(self, test1, test2):
        PyTestUsageError.__init__(
            self, "Two tests have the same name: %s and %s."
            % ( test1.get_path(), test2.get_path() )
            )        

class Test( PyPLearnObject ):
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

    class Defaults:
        name              = None
        description       = ''
        program           = None
        arguments         = ''
        resources         = []
        disabled          = False

        __ordered_attr__  = [ 'name',
                              'description',
                              'program',
                              'arguments',
                              'resources',
                              'disabled'
                              ]

        ## Internal attributes
        _directory        = None
        _metaprotocol     = None
        
    _instances_map    = {}
    _families_map     = {}
    
    _statistics       = BasicStats("Test statistics")
    _expected_results = os.path.join(ppath.pytest_dir, "expected_results")
    _run_results      = os.path.join(ppath.pytest_dir, "run_results")

    def current_stats(cls):
        return cls._statistics.current_stats()
    current_stats = classmethod(current_stats)
    
    def __init__(self, **overrides):
        PyPLearnObject.__init__(self, **overrides)        

        self._directory = os.getcwd()
        self._metaprotocol   = string.replace( "PYTEST__%s__RESULTS" %
                                               self.name, ':', '_'
                                               )

        if Test._instances_map.has_key( self.name ):
            raise DuplicateName( Test._instances_map[self.name], self )
        else:
            Test._statistics.new_test( self.directory(), self.name )
            Test._instances_map[self.name] = self
            if Test._families_map.has_key( self.directory() ):
                Test._families_map[self.directory()].append(self)
            else:
                Test._families_map[self.directory()] = [self]

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

