_cvs_id__ = "$Id: modes.py 3647 2005-06-23 15:49:51Z dorionc $"

import copy, shutil
import plearn.utilities.version_control as version_control
import plearn.utilities.ppath           as ppath
import plearn.utilities.toolkit         as toolkit

from tests                                import *
from programs                             import *
from plearn.utilities.toolkit             import *
from plearn.utilities.verbosity           import *
from plearn.utilities.ModeAndOptionParser import *

current_mode    = None

__all__ = [
    ## Variables
    'current_mode'
    ]

class PyTestMode(Mode):
    #
    #  Static method
    #
    def build_tests(args, directory, dirlist):
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
                vprint( "%s: %s" % (e.__class__.__name__,e) )
    build_tests = staticmethod(build_tests)

    #
    #  Class methods
    #
    def requires_config_parsing(cls):
        return True
    requires_config_parsing = classmethod(requires_config_parsing)

    def target_options(cls, parser):
        target_options = OptionGroup(parser, "Target Options", "")

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
    target_options = classmethod(target_options)

    def testing_options(cls, parser):
        testing_options = OptionGroup(parser, "Testing Options", "")

        testing_options.add_option( '-l', '--localhost',
                                    action='store_true',
                                    help='CURRENTLY ALWAYS TRUE!!!',
                                    default=True )

        testing_options.add_option( '--hosts', 
                                    help='The maximum nuber of hosts to use simultaneously.',
                                    default=10 )
        
        return testing_options
    testing_options = classmethod( testing_options )

    def option_groups(cls, parser):        
        return [ cls.target_options(parser) ]
    option_groups = classmethod(option_groups)

    #
    #  Instance methods
    #
    def __init__(self, targets, options):
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

        self.restrictTo(self.options.test_name)

    def restrictTo(self, test_name):
        """If a single test is to be ran, restrict the loaded test to that only one."""
        if test_name:
            Test.restrictTo(test_name)
            

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

    def option_groups(cls, parser):        
        return []
    option_groups = classmethod(option_groups)

    def requires_config_parsing(cls):
        return False
    requires_config_parsing = classmethod(requires_config_parsing)

    def __init__( self, targets, options ):
        super( ignore, self ).__init__( targets, options )

        for target in targets:
            os.system("touch %s" % os.path.join( target, ignore.ignore_file ) )
            
class list(PyTestMode):
    """Lists all tests within target directories."""
    def option_groups(cls, parser):
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
        
        return [ list_options ] + PyTestMode.option_groups(parser)
    option_groups = classmethod( option_groups )

    #
    #  Instance methods
    #
    def __init__(self, targets, options):
        super(list, self).__init__(targets, options)

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

class locate(list):
    """Locates the named test.

    Usage: pytest locate <test_name>
    (Equivalent to 'pytest list --all -n <test_name>')
    """
    def __init__(self, targets, options):
        if len(targets) != 1:
            vprint("Usage: pytest locate <test_name>")

        else:
            options.all = True
            options.test_name = targets[0]        
            try:
                super(locate, self).__init__(targets, options)
            except KeyError:
                vprint("No test named %s found."%options.test_name)

class prune( PyTestMode ):
    """Removes all pytest directories within given test directories."""    
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

#TO_BE_ADDED: class report(PyTestMode):
#TO_BE_ADDED:     """Test diagnosis."""
#TO_BE_ADDED:     def __init__(self, targets, options):
#TO_BE_ADDED:         super(report, self).__init__(targets, options)
#TO_BE_ADDED: 
#TO_BE_ADDED:         for (family, tests) in Test._families_map.iteritems():
#TO_BE_ADDED:             formatted_strings = []
#TO_BE_ADDED: 
#TO_BE_ADDED:             for test in tests:
#TO_BE_ADDED:                 if self.options.disabled and not test.disabled:
#TO_BE_ADDED:                     continue
#TO_BE_ADDED:                 if self.options.enabled  and test.disabled:
#TO_BE_ADDED:                     continue
#TO_BE_ADDED:                 formatted_strings.append(
#TO_BE_ADDED:                     formatted_string(test.name, test.disabled)
#TO_BE_ADDED:                     )
#TO_BE_ADDED: 
#TO_BE_ADDED:             if formatted_strings:
#TO_BE_ADDED:                 vprint( "In %s\n    %s\n"
#TO_BE_ADDED:                         % ( family, string.join(formatted_strings, '\n    ') )
#TO_BE_ADDED:                         )            

class vc_add( PyTestMode ):
    """Add PyTest's config file and results to version control."""
    def __init__( self, targets, options ):
        super(vc_add, self).__init__(targets, options)
        
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
                raise PyTestError(
                    "The directory in which lies a config file must be added to version control by user.\n"
                    "%s was not." % family
                    )

class FamilyConfigMode(PyTestMode):
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

    def restrictTo(self, test_name):
        self.test_name = test_name
    
class add(FamilyConfigMode):
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

    def option_groups(cls, parser):
        add_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                   "Available under %s mode only." % cls.__name__ )

        add_options.add_option( "--program-type",
                                choices=["LocalProgram", "LocalCompilableProgram",
                                         "GlobalProgram", "GlobalCompilableProgram"], 
                                default="GlobalCompilableProgram",
                                help="The added test uses the program-type for its program attribute." 
                                )

        add_options.add_option( "--program-name",
                                 default="plearn_tests",
                                 help="The added test uses program-name as name for its program attribute." 
                                 )

        add_options.add_option( "--arguments",
                                 default="",
                                 help="Arguments' string to be used by the test." 
                                 )

        add_options.add_option( "--resources",
                                 default="",
                                 help="Comma separated list of resources to be used by the test." 
                                 )

        return [ add_options ] + PyTestMode.option_groups(parser)
    option_groups = classmethod( option_groups )


    def __init__(self, targets, options):
        test_name = options.test_name
        if test_name == '':
            test_name = 'MANDATORY_TEST_NAME' 

        ProgClass = globals()[options.program_type]
        program = ProgClass(name=options.program_name)
	
	resources = []
        if options.resources != "":
            resources = options.resources.split(',')

        # Caught by Test's instances management         
        Test( name=test_name, program=program,
              arguments=options.arguments,
              resources=resources )              
        super(add, self).__init__(targets, options)
                
    def test_hook(self, test):
        pass
    
class disable(FamilyConfigMode):
    """Disables targeted tests.

    The disabled tests can be restored (L{enable mode<enable>}) afterwards.
    """
    def test_hook(self, test):
        if self.test_name=="" or self.test_name==test.name:
            test.disabled = True

class enable(FamilyConfigMode):
    """Enables disabled (L{disable mode<disable>}) tests."""
    def test_hook(self, test):
        if self.test_name=="" or self.test_name==test.name:
            test.disabled = False

class update(FamilyConfigMode):
    """Updates the PyTest config file to the current format.

    If the PyTest developers change anything of the PyTest config
    file, from the docstring to the Test class behavior, the changes
    will be made taking in considerations that a simple call to the
    config mode should be enough to migrate an old PyTest config file
    the new format.

    This mode can also be used with options --test-name and --new-name to
    rename a test (works only under SVN).
    """
    def option_groups(cls, parser):
        update_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                      "Available under %s mode only." % cls.__name__ )

        update_options.add_option( '--test-name',
                                   default=None,
                                   help='The test to be renamed.',
                                   )

        update_options.add_option( "--new-name",                                   
                                   default=None,
                                   help="Must be used in conjunction with --test-name: Test which name is "
                                   "'test_name' will be renamed to 'new_name'." 
                                   )

        return [ update_options ]
    option_groups = classmethod(option_groups)

    def test_hook(self, test):
        """Does nothing.

        Tests are parsed by L{initialize}, called in the
        FamilyConfigMode template procedure. The PyTest config file
        docstring is always updated to the Test class' docstring by
        any FamilyConfigMode. For the Test class' internal behavior,
        changes will be made to support
        """
        name = self.options.test_name
        if name is not None and test.name == name:
            new_name = self.options.new_name
            assert new_name is not None,\
                "Options --test-name and --new-name must be used together."
            
            assert os.getcwd() == test.directory()
            expected_results = test.test_results(Test._expected_results)
            
            walker = os.walk(expected_results, topdown=False)
            for root, dirs, files in walker:
                if root.find(".svn") != -1:
                    continue
                
                for fname in dirs+files:
                    if fname.find(name) != -1:
                        old_path = os.path.join(root, fname)
                        new_path = os.path.join(root, fname.replace(name, new_name))
                        os.system('svn mv --force %s %s'%(old_path, new_path))

            os.system('svn mv --force %s %s'%(expected_results, expected_results.replace(name, new_name)))
            test.name = new_name
                        
    
class RoutineBasedMode(PyTestMode):
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
        super(RoutineBasedMode, self).__init__(targets, options)

        self.report_traceback = options.traceback

        # test_instances = None
        # if self.options.test_name:
        #     test_instances = [( self.options.test_name,
        #                         Test._instances_map[ self.options.test_name ]
        #                         )]
        #     Test._test_count = 1
        #     
        # else:
        #     test_instances = Test._instances_map.items()
        test_instances = Test._instances_map.items()
        
        self.dispatch_tests(test_instances)

    def dispatch_tests(self, test_instances):
        for (test_name, test) in test_instances:            
            if test.is_disabled():
                vprint("Test %s is disabled." % test.name, 2)
                test.setStatus("DISABLED")
            else:
                try:
                    RoutineType = self.routine_type( self.options )            
                    routine     = RoutineType(test=test)
                    routine.start()
                except PyTestError, e:
                    # --traceback: This flag triggers routines to report
                    # the traceback of PyTestError. By default, only the
                    # class's name and meesage are reported.
                    if self.options.traceback:
                        raise
                    else:
                        test.setStatus("SKIPPED", e.pretty_str())


class compile(RoutineBasedMode):
    def routine_type(cls, options=None): return CompilationRoutine
    routine_type = classmethod(routine_type)

class ResultsBasedMode(RoutineBasedMode):
    def option_groups(cls, parser):
        ogroups = RoutineBasedMode.option_groups(parser)

        ogroups[1].add_option( '--no-compile', default=False,
                               action="store_true",
                               help='Any program compilation is bypassed.' )

        return ogroups        
    option_groups = classmethod(option_groups)
    
class results(ResultsBasedMode):
    def routine_type(cls, options=None):
        if options:
            ResultsCreationRoutine.no_compile_option = options.no_compile
        return ResultsCreationRoutine
    routine_type = classmethod(routine_type)

class run(ResultsBasedMode):    
    def routine_type(cls, options=None):
        if options:
            RunTestRoutine.no_compile_option = options.no_compile
        return RunTestRoutine
    routine_type = classmethod(routine_type)

    
if __name__ == '__main__':
    import os, sys
    def vsystem(cmd):        
        print >>sys.stderr, '#  %s\n' % cmd
        os.system( '%s > /dev/null'%cmd )
        print >>sys.stderr, ''

    ## Since run results are not under version control...
    os.chdir(os.path.join(ppath.ppath('PLEARNDIR'), 'test_suite/svn_tests'))

    ## Creates a new test script
    f = open('INTERNAL_SCRIPT.py', 'w')
    f.write("print 'INTERNAL_SCRIPT'\n")
    f.close()
    
    ## Add it
    vsystem("pytest add -n PYTEST_INTERNAL --program-type GlobalProgram --program-name python "
              "--arguments INTERNAL_SCRIPT.py --resources INTERNAL_SCRIPT.py")
    
    ## Generate results & run the test
    vsystem("pytest results")
    vsystem("pytest run")
    
    ## vc_add && commit
    vsystem('pytest vc_add')
    vsystem('svn commit -m "PYTEST INTERNAL TEST"')
        
    ## shutil.rmtree
    print >>sys.stderr, '#\n#  shutil.rmtree\n#'
    shutil.rmtree("pytest")
    
    ## svn up && run
    vsystem('svn up')
    vsystem('pytest run')
    
    ## svn remove -f
    vsystem('svn remove --force pytest pytest.config')
    vsystem('svn commit -m "PYTEST INTERNAL TEST"')
    os.remove('INTERNAL_SCRIPT.py')

    ## Verify that directory is empty
    print os.listdir(os.getcwd())
