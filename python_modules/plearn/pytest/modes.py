__cvs_id__ = "$Id: modes.py,v 1.20 2005/03/11 03:08:04 dorionc Exp $"

import copy, shutil
import plearn.utilities.version_control as version_control
import plearn.utilities.ppath           as ppath
import plearn.utilities.toolkit         as toolkit

from Test                       import *
from programs                   import *
from routines                   import *
from ModeAndOptionParser        import *
from plearn.utilities.toolkit   import *
from plearn.utilities.verbosity import *
## from   plearn.utilities.global_variables   import *

current_mode    = None

targets         = None
options         = None

__all__ = [
    ## Functions
    'add_supported_modes',

    ## Variables
    'current_mode', 'targets', 'options'        
    ]

## All modes that are completed must, in the class declaration,
## 
supported_modes = [ 'add',
                    ## 'commit',
                    'compile', 'disable', 'enable',
                    ## 'light_commit',
                    'list',   'ignore',  'prune',   'results',
                    'run',          'update',
                    'vc_add'
                    ]

def add_supported_modes( parser ):
    for mode in supported_modes:
        mode_instance = eval( "%s()" % mode )
        parser.add_mode( mode_instance )

class PyTestMode(Mode):

    ## Static method
    def build_tests(ignored_directories, directory, dirlist):
        if ignore.is_ignored(directory, dirlist):
            toolkit.exempt_list_of( dirlist, copy.copy(dirlist) )
            ignored_directories.append( directory )
            return

        toolkit.exempt_list_of( dirlist, ppath.special_directories )    
        os.chdir(directory)

        config = config_file_path()
        try:
            if config in dirlist:
                execfile(config)
        except Exception, e:
            vprint( "+++ In %s" % os.path.join(directory, config) )
            if options.traceback:
                raise
            else:
                vprint( "%s: %s." % (e.__class__.__name__,e) )
    build_tests = staticmethod(build_tests)

    ## Class methods
    def initialize(cls):
        global targets

        ## --all: Run all tests found in subdirectories of directories in
        ## globalvars.all_roots test_suite branches. If some targets are
        ## provided, these will be ignored.
        if options.all:
            targets           = plbranches  ## globalvars.all_roots
            options.recursive = True

        ## If no targets are provided, the cwd is the default target.
        if len(targets) == 0:
            targets.append( os.getcwd() )

        for i,target in enumerate(targets):
            targets[i] = os.path.abspath(target)

        if options.recursive:
            targets = ppath.exempt_of_subdirectories( targets )
    initialize = classmethod(initialize)

    def parse_config_files(cls):
        ignored_directories = []
        for target in targets:
            if hasattr(options, 'recursive') and options.recursive:
                os.path.walk(target, cls.build_tests, ignored_directories)
            else:
                cls.build_tests(ignored_directories, target, os.listdir(target))
        if len(ignored_directories) > 0:
            ignored = [ "The following directories (and their subdirectories) were ignored", "" ]
            ignored.extend( [ '    '+ign for ign in ignored_directories ] )
            vprint.highlight( ignored, highlighter='x' ) 

    parse_config_files = classmethod(parse_config_files)

           
    ## Instance methods
    def __init__(self):
        Mode.__init__(self)

    def option_groups(self, parser):
        return []
    
    def procedure(self):
        raise NotImplementedError

    def start(self):
        try:
            self.procedure()
        except PyTestUsageError, e: 
            if options.traceback:
                raise
            else:
                vprint( "%s: %s." % (e.__class__.__name__,e) )

    def target_options(self, parser):
        target_options = OptionGroup( parser, "Target Options", "" )

        target_options.add_option( "--all",
                                    action="store_true", default=False,
                                    help= "Run all tests found in subdirectories of directories in "
                                    "globalvars.all_roots test_suite branches. If some targets "
                                    "are provided, these will be ignored. " 
                                    )

        target_options.add_option( "-R", "--recursive",
                                    action="store_true", default=False,
                                    help = 'Process all targets recursively. If some target is '
                                    'the subdirectory to another target, it will be ignored, i.e. '
                                    'the whole hierarchy will be tested only once.'
                                    ) 

        target_options.add_option( '-n', '--test-name',
                                    help='Restricts the current mode to the named test.',
                                    default='' )

        return target_options


##    def global_options(self, parser):
##        global_options = OptionGroup( parser, "Global Options", "" )

##        global_options.add_option( '--traceback',
##                                    action="store_true",
##                                    help="This flag triggers routines to report the traceback of "
##                                    "PyTestUsageError. By default, only the class's name and meesage "
##                                    "are reported.",
##                                    default = False )
        
##         return global_options
        
    def testing_options(self, parser):
        testing_options = OptionGroup( parser, "Testing Options", "" )

        testing_options.add_option( '-l', '--localhost',
                                    action='store_true',
                                    help='CURRENTLY ALWAYS TRUE!!!',
                                    default=True )

        testing_options.add_option( '--hosts', 
                                    help='The maximum nuber of hosts to use simultaneously.',
                                    default=10 )
        
        return testing_options


class ignore(PyTestMode):
    """Causes the target hierarchy to be ignored by PyTest.

    Simply drops a I{pytest.ignore} file that is recognized by PyTest
    afterwards.
    """
    ignore_file               = 'pytest.ignore'

    def is_ignored(directory, dirlist=None):
        if dirlist == None:
            dirlist = os.listdir(directory)
            
        if ignore.ignore_file in dirlist:
            return True
        return False
    is_ignored = staticmethod(is_ignored)

    def procedure(self):
        global targets
        if len(targets) == 0:
            targets = [ os.getcwd() ]
        else:
            targets = ppath.exempt_of_subdirectories( targets )

        for target in targets:
            os.system("touch %s" % os.path.join( target, ignore.ignore_file ) )

            
class list(PyTestMode):
    """Lists all tests within target directories."""
    def procedure(self):
        self.initialize()
        self.parse_config_files()

        formatted_string = lambda n,d: ( "%s Disabled: %s"
                                         % (string.ljust(n, 25), string.ljust(str(d), 15))
                                         )
        for (family, tests) in Test._families_map.iteritems():
            formatted_strings = []

            for test in tests:
                if options.disabled and not test.disabled:
                    continue
                if options.enabled  and test.disabled:
                    continue
                formatted_strings.append(
                    formatted_string(test.name, test.disabled)
                    )

            if formatted_strings:
                vprint( "In %s\n    %s\n"
                        % ( family, string.join(formatted_strings, '\n    ') )
                        )    
        
    def option_groups( self, parser ):
        list_options = OptionGroup( parser, "Mode Specific Options --- %s" % self.classname(),
                                    "Available under list mode only." )

        list_options.add_option( "-d", "--disabled",
                                 action="store_true",
                                 default=False,
                                 help= "The list provided will contain only disabled tests." 
                                 )

        list_options.add_option( "-e", "--enabled",
                                 action="store_true",
                                 default=False,
                                 help= "The list provided will contain only enabled tests." 
                                 )

        return [ self.target_options(parser), list_options ]


class prune( PyTestMode ):
    """Removes all pytest directories within given test directories."""
    def procedure(self):
        self.initialize()
        self.parse_config_files()

        answer = ""
        while not answer in ['yes', 'no']:
            answer = raw_input( "This mode removes all pytest directories within "
                                "given test directories. Are you sure you want to "
                                "continue (yes or no): " )

        if answer == "no":
            print "\nMode prune interrupter by user."
            return

        
        for ( family, tests ) in Test._families_map.iteritems():
            fam_pytest_dir = os.path.join( family, "pytest" )
            
            if os.path.exists( fam_pytest_dir ):
                os.chdir( family )
                shutil.rmtree( "pytest" )

    def option_groups(self, parser):
        return [ self.target_options(parser) ]

class vc_add( PyTestMode ):
    """Add PyTest's config file and results to version control."""
    def procedure( self ):
        global targets
        if len(targets) == 0:
            targets = [ os.getcwd() ]
        else:
            targets = ppath.exempt_of_subdirectories( targets )            
        self.parse_config_files()

        for (family, tests) in Test._families_map.iteritems():
            os.chdir( family )


            config_path = config_file_path()
            try:
                version_control.add( config_path )
                version_control.add( ppath.pytest_dir )
                version_control.add( Test._expected_results )
                for test in tests:
                    version_control.recursive_add( test.test_results( Test._expected_results ) )
            except version_control.VersionControlError:
                raise PyTestUsageError(
                    "The directory in which lies a config file must be added to version control by user.\n"
                    "%s was not." % family
                    )

class FamilyConfigMode( PyTestMode ):
    def procedure(self):
        self.initialize()
        self.parse_config_files()

        for (family, tests) in Test._families_map.iteritems():
            config_path  = config_file_path( family )
            if os.path.exists( config_path ):
                toolkit.keep_a_timed_version( config_path )

            config_file  = open(config_path, 'w')

            config_text  = ( '"""Pytest config file.\n\n%s\n"""'% toolkit.doc(Test) )

            for test in tests:
                self.test_hook( test ) 
                config_text += "\n%s\n" % str( test )

            config_file.write(config_text)
            config_file.close()    

    def test_hook(self, test):
        raise NotImplementedError
    
    def option_groups(self, parser):
        return [ self.target_options(parser) ]

class add( FamilyConfigMode ):
    """Adds a test in a given directory.
    
    A directory is considered to be a test directory as soon as it
    contains a I{pytest.config} file. Therefore, this mode simply
    drops a config file with some explanation how to instanciate the
    L{Test} objects within the file. If a I{pytest.config} file
    already exists, it is appended a new L{Test} template.
    
    The config file is executed within the pytest script. So, it B{is}
    in python, which means that one can add any comments he wishes and
    may also define own functions if complicated processing is
    requested before defining the test.
    """
    
    ## Static method
    def build_tests(ignored_directories, directory, dirlist):
        PyTestMode.build_tests(ignored_directories, directory, dirlist)

        test_name = options.test_name
        if test_name == '':
            test_name = 'MANDATORY_TEST_NAME' 

        Test( name    = test_name,
              program = GlobalCompilableProgram( name = "plearn")  )        
    build_tests = staticmethod(build_tests)

    def test_hook(self, test):
        pass
    
class disable( FamilyConfigMode ):
    """Disables targeted tests.

    The disabled tests can be restored (L{enable mode<enable>}) afterwards.
    """
    def test_hook( self, test ):
        test.disabled = True

class enable( FamilyConfigMode ):
    """Enables disabled (L{disable mode<disable>}) tests."""
    def test_hook( self, test ):
        test.disabled = False

class update( FamilyConfigMode ):
    """Updates the PyTest config file to the current format.

    If the PyTest developers change anything of the PyTest config
    file, from the docstring to the Test class behavior, the changes
    will be made taking in considerations that a simple call to the
    config mode should be enough to migrate an old PyTest config file
    the new format.
    """
    def test_hook( self, test ):
        """Does nothing.

        Tests are parsed by L{initialize}, called in the
        FamilyConfigMode template procedure. The PyTest config file
        docstring is always updated to the Test class' docstring by
        any FamilyConfigMode. For the Test class' internal behavior,
        changes will be made to support
        """
        pass
    
class RoutineBasedMode( PyTestMode ):
    def initialize(cls):
        PyTestMode.initialize()
    initialize = classmethod(initialize)
    
    def description(self):
        return toolkit.doc( self.routine_type() )

    def dispatch_tests(self, test_instances):
        for (test_name, test) in test_instances:
            RoutineType = self.routine_type()
            routine     = RoutineType( test = test )
            routine.start()

    def help(self):
        return toolkit.short_doc( self.routine_type() )

    def procedure(self):
        self.initialize()
        
        ## --traceback: This flag triggers routines to report the traceback of
        ## PyTestUsageError. By default, only the class's name and meesage
        ## are reported.
        Routine._report_traceback = options.traceback

        self.parse_config_files()

        test_instances = None
        if options.test_name:
            test_instances = [( options.test_name,
                                Test._instances_map[ options.test_name ]
                                )]

            Test.statistics.restrict( options.test_name )
            
        else:
            test_instances = Test._instances_map.items()

        self.dispatch_tests( test_instances )
        print_stats()

    def routine_type(self):
        raise NotImplementedError
    
    def option_groups(self, parser):
        return [ self.target_options(parser),
                 self.testing_options(parser) ]

class compile( RoutineBasedMode ):
    def routine_type(self): return CompilationRoutine

class results( RoutineBasedMode ):
    def routine_type(self): return ResultsCreationRoutine

class run( RoutineBasedMode ):    
    def routine_type(self):
        if options:
            RunTestRoutine.no_compile_option = options.no_compile
        return RunTestRoutine

    def option_groups(self, parser):
        ogroups = RoutineBasedMode.option_groups(self, parser)

        ogroups[1].add_option( '--no-compile', default=False,
                               action="store_true",
                               help='Any program compilation is bypassed.' )

        return ogroups

