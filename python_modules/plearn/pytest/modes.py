_cvs_id__ = "$Id$"

import copy, shutil
import plearn.utilities.version_control as version_control
import plearn.utilities.ppath           as ppath
import plearn.utilities.toolkit         as toolkit

from Test                                 import *
from programs                             import *
from routines                             import *
from plearn.utilities.toolkit             import *
from plearn.utilities.verbosity           import *
from plearn.utilities.ModeAndOptionParser import *

current_mode    = None

__all__ = [
    ## Variables
    'current_mode'
    ]

class PyTestMode( Mode ):

    def target_options( cls, parser ):
        target_options = OptionGroup( parser, "Target Options", "" )

        target_options.add_option( "--all",
                                    action="store_true", default=False,
                                    help= "Run all tests found in subdirectories of directories in"
                                    "plbranches. If some targets are provided, these will be ignored."
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
    target_options = classmethod( target_options )

    def testing_options( cls, parser ):
        testing_options = OptionGroup( parser, "Testing Options", "" )

        testing_options.add_option( '-l', '--localhost',
                                    action='store_true',
                                    help='CURRENTLY ALWAYS TRUE!!!',
                                    default=True )

        testing_options.add_option( '--hosts', 
                                    help='The maximum nuber of hosts to use simultaneously.',
                                    default=10 )
        
        return testing_options
    testing_options = classmethod( testing_options )

    ## Static method
    def build_tests( args, directory, dirlist):
        options, ignored_directories = args
        
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

    #
    #  Class methods
    #
    def requires_config_parsing(cls):
        return True
    requires_config_parsing = classmethod(requires_config_parsing)

    def option_groups( cls, parser ):
        return []
    option_groups = classmethod( option_groups )

    #
    #  Instance methods
    #
    def __init__( self, targets, options ):
        Mode.__init__(self, targets, options)

        # --all: Run all tests found in subdirectories of directories in
        # plbranches. If some targets are provided, these will be ignored.
        if hasattr( options, 'all' ) and options.all:
            targets           = plbranches
            options.recursive = True

        # If no targets are provided, the cwd is the default target.
        if len(targets) == 0:
            targets.append( os.getcwd() )

        # Ensure paths to be absolute
        for i,target in enumerate(targets):
            targets[i] = os.path.abspath(target)

        # Sanity check
        if hasattr( options, 'recursive' ) and options.recursive:
            targets = ppath.exempt_of_subdirectories( targets )

        # Parses PyTest's config files if the mode requires it.
        if self.requires_config_parsing():
            ignored_directories = []
            for target in targets:
                if hasattr( options, 'recursive' ) and options.recursive:
                    os.path.walk( target, self.build_tests, (options, ignored_directories) )
                else:
                    self.build_tests( (options, ignored_directories), target, os.listdir(target) )

            if len(ignored_directories) > 0:
                ignored = [ "The following directories (and their subdirectories) were ignored", "" ]
                ignored.extend( [ '    '+ign for ign in ignored_directories ] )
                vprint.highlight( ignored, highlighter='x' ) 

            

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

    def requires_config_parsing(cls):
        return False
    requires_config_parsing = classmethod(requires_config_parsing)

    def __init__( self, targets, options ):
        super( ignore, self ).__init__( targets, options )

        for target in targets:
            os.system("touch %s" % os.path.join( target, ignore.ignore_file ) )
            
class list(PyTestMode):
    """Lists all tests within target directories."""
    def option_groups( cls, parser ):
        list_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
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

        return [ cls.target_options(parser), list_options ]
    option_groups = classmethod( option_groups )

    #
    #  Instance methods
    #
    def __init__( self, targets, options ):
        super( list, self ).__init__( targets, options )

        formatted_string = lambda n,d: ( "%s Disabled: %s"
                                         % (string.ljust(n, 25), string.ljust(str(d), 15))
                                         )
        for (family, tests) in Test._families_map.iteritems():
            formatted_strings = []

            for test in tests:
                if self.options.disabled and not test.disabled:
                    continue
                if self.options.enabled  and test.disabled:
                    continue
                formatted_strings.append(
                    formatted_string(test.name, test.disabled)
                    )

            if formatted_strings:
                vprint( "In %s\n    %s\n"
                        % ( family, string.join(formatted_strings, '\n    ') )
                        )    
        


class prune( PyTestMode ):
    """Removes all pytest directories within given test directories."""
    def option_groups( cls, parser ):
        return [ cls.target_options(parser) ]
    option_groups = classmethod( option_groups )
    
    def __init__( self, targets, options ):
        super( prune, self ).__init__( targets, options )
        
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

class vc_add( PyTestMode ):
    """Add PyTest's config file and results to version control."""
    def __init__( self, targets, options ):
        super( vc_add, self ).__init__( targets, options )
        
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
    def option_groups( cls, parser ):
        return [ cls.target_options(parser) ]
    option_groups = classmethod( option_groups )

    def __init__( self, targets, options ):
        super( FamilyConfigMode, self ).__init__( targets, options )
        
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

    def __init__( self, targets, options ):
        test_name = options.test_name
        if test_name == '':
            test_name = 'MANDATORY_TEST_NAME' 

        #
        #  Caught by Test's instances management 
        #
        Test( name    = test_name,
              program = GlobalCompilableProgram( name = "plearn")  )

        super( add, self ).__init__( targets, options )
                
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
    #
    #  Class methods
    #
    def description( cls ):
        return toolkit.doc( cls.routine_type() )
    description = classmethod( description )
    
    def help( cls ):
        return toolkit.short_doc( cls.routine_type() )
    help = classmethod( help )
    
    def routine_type( cls, options=None ):
        raise NotImplementedError
    routine_type = classmethod( routine_type )

    def option_groups( cls, parser ):
        return [ cls.target_options(parser),
                 cls.testing_options(parser) ]
    option_groups = classmethod( option_groups )
    
    #
    #  Instance methods
    #    
    def __init__( self, targets, options ):
        super( RoutineBasedMode, self ).__init__( targets, options )

        ## --traceback: This flag triggers routines to report the traceback of
        ## PyTestUsageError. By default, only the class's name and meesage
        ## are reported.
        Routine._report_traceback = self.options.traceback

        test_instances = None
        if self.options.test_name:
            test_instances = [( self.options.test_name,
                                Test._instances_map[ self.options.test_name ]
                                )]

            Test._statistics.restrict( self.options.test_name )
            
        else:
            test_instances = Test._instances_map.items()

        self.dispatch_tests( test_instances )
        print_stats()

    def dispatch_tests(self, test_instances):
        for (test_name, test) in test_instances:
            RoutineType = self.routine_type( self.options )
            routine     = RoutineType( test = test )
            routine.start()


class compile( RoutineBasedMode ):
    def routine_type( cls, options=None ): return CompilationRoutine
    routine_type = classmethod( routine_type )

class results( RoutineBasedMode ):
    def routine_type( cls, options=None ): return ResultsCreationRoutine
    routine_type = classmethod( routine_type )

class run( RoutineBasedMode ):    
    def routine_type(cls, options=None):
        if options:
            RunTestRoutine.no_compile_option = options.no_compile
        return RunTestRoutine
    routine_type = classmethod( routine_type )

    def option_groups( cls, parser ):
        ogroups = RoutineBasedMode.option_groups( parser )

        ogroups[1].add_option( '--no-compile', default=False,
                               action="store_true",
                               help='Any program compilation is bypassed.' )

        return ogroups        
    option_groups = classmethod( option_groups )
    
