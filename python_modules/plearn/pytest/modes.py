import copy, logging, shutil

# PyTest Modules
import core, tests
from tests import *
from programs import *

from plearn.utilities import ppath
from plearn.utilities import moresh
from plearn.utilities import toolkit
from plearn.utilities import version_control

from plearn.utilities.ModeAndOptionParser import *


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
            logging.critical("+++ In %s" % os.path.join(directory, config))
            if options.traceback:
                raise
            else:
                logging.critical("%s: %s" % (e.__class__.__name__,e))
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
        return target_options
    target_options = classmethod(target_options)

    def testing_options(cls, parser):
        testing_options = OptionGroup(parser, "Testing Options", "")

        testing_options.add_option( '-l', '--localhost',
                                    action='store_true',
                                    help='CURRENTLY ALWAYS TRUE!!!',
                                    default=True )

        testing_options.add_option( '--hosts', 
                                    help='The maximum number of hosts to use simultaneously.',
                                    default=10 )

        testing_options.add_option( '--pymake-compile-options', default="",
            help="The default behavior of tests being compiled with Pymake is to "
                "forward their 'compile_options' to Pymake. This command line "
                "option can be used to add flags to the compilation process. DO "
                "NOT preceed the option(s) name by a dash. If you want to provide many "
                "flags, provide a CSV list of flags, e.g. \n"
                "    --pymake-compile-options safeopt,fastmath \n"
                "For now, it is the user's responsability to ensure that the options "
                "provided here do not clash with the ones potentially provided "
                "through the tests' option 'compile_options'.")
        
        return testing_options
    testing_options = classmethod( testing_options )

    def restriction_options(cls, parser):
        restriction_options = OptionGroup(parser, "Restriction Options", "")

        restriction_options.add_option( '-n', '--test-name',
                                   help='Restricts the current mode to the named test.',
                                   default='' )
        
        restriction_options.add_option( '--category',
                                   help='Restricts the current mode to the named category of tests. '
                                   'Note that the value of this option is neglected if --test-name '
                                   'is provided.',
                                   default='General' )

        return restriction_options
    restriction_options = classmethod(restriction_options)


    #
    #  Instance methods
    #
    def __init__(self, targets, options):
        Mode.__init__(self, targets, options)

        # Managing the test-name and category restrictions
        self.restrictions( )

        # --all: Run all tests found in subdirectories of directories in
        # plbranches. If some targets are provided, these will be ignored.
        if hasattr(options, 'all') and options.all:
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
                logging.warning('\n'.join(['---']+ignored+['---']))

            # Take the opportunity to update cached test map
            if hasattr(options, 'all') and options.all:
                core.updateCachedTestMap(Test._instances_map)

            # Were all tests provided through --test-name found?
            self.checkForExpectedTests( )

    def checkForExpectedTests(self):
        """Assert that all tests provided through --test-name were found."""
        missing_expected_tests = Test.missingExpectedTests()
        if not missing_expected_tests:
            return

        # Some tests were not found!
        msg_pair = (toolkit.plural(len(missing_expected_tests)),
                    ','.join(missing_expected_tests))
        logging.warning('\nSome test%s (%s) were not found!!!'%msg_pair)
        if hasattr(self.options, 'all') and self.options.all:
            raise PyTestUsageError(
                "Asked for test%s named %s which can't be found." % msg_pair)
            
        # Try to "fast-locate" missing tests
        logging.warning('Trying to "fast-locate" missing tests...')
        cached_test_map = core.getCachedTestMap()
        while missing_expected_tests:
            test_name = missing_expected_tests.pop()
            if test_name in cached_test_map:
                target = cached_test_map[test_name]
                self.build_tests( (self.options, []), target, os.listdir(target) )

            # If you can't fast locate, use brute force
            else:
                logging.warning('Must use brute force since %s was not in cache...'%test_name)
                self.options.all = True
                Test._instances_map = {} # To avoid DuplicateName errors
                for target in plbranches:
                    os.path.walk( target, self.build_tests, (self.options, []) )
                # Can't enter an infinite loop since we now have 'self.options.all == True'
                return self.checkForExpectedTests()

    def restrictions(self):
        """Default --test-name and --category management."""
        tnames = []
        if hasattr(self.options, 'test_name') and self.options.test_name:
            tnames = self.options.test_name.split(',')

        # If some target is not a directory, most likely it is a test name
        targets = self.targets[:]
        for target in targets:
            if not os.path.isdir(target):
                tnames.append(target)
                self.targets.remove(target)

        self.options.test_name = ",".join(tnames)                        
        if self.options.test_name:
            Test.restrictTo(self.options.test_name)

        elif hasattr(self.options, 'category'):
            Test.restrictToCategory(self.options.category)

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
        
        return [ list_options,
                 cls.target_options(parser),
                 cls.restriction_options(parser) ]
    option_groups = classmethod( option_groups )

    #
    #  Instance methods
    #
    def __init__(self, targets, options):
        super(list, self).__init__(targets, options)

        formatted_string = lambda n,d: \
            "%s Disabled: %s" % (n.ljust(25), str(d).ljust(15))

        self._listed = []
        for (family, test_list) in Test._families_map.iteritems():
            formatted_strings = []

            for test in test_list:
                if self.options.disabled and not test.disabled:
                    continue
                if self.options.enabled  and test.disabled:
                    continue
                self._listed.append(test)
                formatted_strings.append(
                    formatted_string(test.name, test.disabled)
                    )

            if formatted_strings:
                logging.warning(
                    "In %s\n    %s\n"%(family, '\n    '.join(formatted_strings)) )

class locate(list):
    """Locates the named test.

    Usage: pytest locate <test_name> ...
    (Equivalent to 'pytest list --all -n <test_name>...')
    """
    def __init__(self, targets, options):
        cached_test_map = core.getCachedTestMap()
	for target in targets:
	    if target in cached_test_map:
                test_dir = cached_test_map[target]
                self._listed = test_dir, target
                logging.warning("In %s\n    %s"%self._listed)
            else:
                options.all = True
                options.test_name = target        
                try:
                    super(locate, self).__init__([target], options)
                except KeyError:
                    logging.critical("No test named %s found."%options.test_name)

class diff(locate):
    """Starts kdiff3 to compare the expected/run results directory trees.

    Usage: pytest diff <test_name>
    """
    def __init__(self, targets, options):
        super(diff, self).__init__(targets, options)
        if isinstance(self._listed, tuple):
            moresh.pushd()
            dirc = self._listed[0]
            test_name = self._listed[1]
            assert( test_name.find(',') == -1 )

            Test.restrictTo(test_name)
            self.build_tests((options, []), dirc, os.listdir(dirc))

            assert( len(Test._instances_map)==1 )
            test = Test._instances_map[test_name]
            
        elif isinstance(self._listed, type([])):
            assert( len(self._listed)==1 )            
            test = self._listed[0]

        else:
            raise TypeError, type(self._listed)

        expected = test.resultsDirectory(EXPECTED_RESULTS)
        run_results = test.resultsDirectory(RUN_RESULTS)
        os.system("kdiff3 %s %s >& /dev/null"%(expected, run_results))
        #os.system("meld %s %s"%(expected, run_results))
        
class prune( PyTestMode ):
    """Removes all pytest directories within given test directories."""    
    def option_groups(cls, parser):
        return [ cls.target_options(parser) ]
    option_groups = classmethod(option_groups)

    def __init__( self, targets, options ):
        super( prune, self ).__init__( targets, options )
        
        answer = ""
        while not answer in ['yes', 'no']:
            answer = raw_input( "This mode removes all .pytest directories within "
                                "given test directories. Are you sure you want to "
                                "continue (yes or no): " )

        if answer == "no":
            print "\nMode prune interrupter by user."
            return

        
        for ( family, test_list ) in Test._families_map.iteritems():
            fam_pytest_dir = os.path.join( family, ".pytest" )
            
            if os.path.exists( fam_pytest_dir ):
                os.chdir( family )
                shutil.rmtree( ".pytest" )

#TO_BE_ADDED: class report(PyTestMode):
#TO_BE_ADDED:     """Test diagnosis."""
#TO_BE_ADDED:     def option_groups(cls, parser):
#TO_BE_ADDED:         return [ cls.target_options(parser),
#TO_BE_ADDED:                  cls.restriction_options(parser) ]
#TO_BE_ADDED:     option_groups = classmethod(option_groups)
#TO_BE_ADDED:     
#TO_BE_ADDED:     def __init__(self, targets, options):
#TO_BE_ADDED:         super(report, self).__init__(targets, options)
#TO_BE_ADDED: 
#TO_BE_ADDED:         for (family, test_list) in Test._families_map.iteritems():
#TO_BE_ADDED:             formatted_strings = []
#TO_BE_ADDED: 
#TO_BE_ADDED:             for test in test_list:
#TO_BE_ADDED:                 if self.options.disabled and not test.disabled:
#TO_BE_ADDED:                     continue
#TO_BE_ADDED:                 if self.options.enabled  and test.disabled:
#TO_BE_ADDED:                     continue

class confirm( PyTestMode ):
    """Confirm the results obtained using PyTest's 'results' mode."""
    def option_groups(cls, parser):
        return [ cls.target_options(parser),
                 cls.restriction_options(parser) ]
    option_groups = classmethod(option_groups)

    def __init__( self, targets, options ):
        super(confirm, self).__init__(targets, options)

        # Trying to isolate a weard bug in confirm...        
        true_level = logging.root.level
        logging.root.removeHandler(core.hdlr)

        import StringIO
        strstream = StringIO.StringIO()
        hdlr      = logging.StreamHandler(strstream)        
        logging.root.addHandler(hdlr)
        logging.root.setLevel(logging._levelNames["DEBUG"])
        ###
        
        for (family, test_list) in Test._families_map.iteritems():
            os.chdir( family )

            config_path = config_file_path()
            try:
                version_control.add( config_path )
                version_control.add( ppath.pytest_dir )
                version_control.ignore( ppath.pytest_dir, [ '*.compilation_log' ] )
                for test in test_list:
                    if test.disabled:
                        continue
                    svn_results = test.resultsDirectory(tests.SVN_RESULTS)

                    if os.path.exists(svn_results):
                        expected_results = test.resultsDirectory(tests.EXPECTED_RESULTS)
                        confirmed_results = expected_results+'.confirmed'
                        if os.path.exists(confirmed_results):
                            shutil.rmtree(confirmed_results)#in case we crashed
                        os.rename(expected_results, confirmed_results)
                        os.rename(svn_results, expected_results)
                        self.migrate_results_trees(expected_results, confirmed_results)
                        shutil.rmtree(confirmed_results)
                    else:
                        version_control.add( test.resultsDirectory() )
                        version_control.recursive_add( test.resultsDirectory(tests.EXPECTED_RESULTS) )
                        version_control.ignore( test.resultsDirectory(), [ '.plearn', tests.RUN_RESULTS, 'PSAVEDIFF' ] )

            except version_control.VersionControlError:
                raise core.PyTestUsageError(
                    "The directory in which lies a config file must be added to version control by user.\n"
                    "%s was not." % family
                    )

        # ... trying to isolate a weard bug in confirm
        logging.root.removeHandler(hdlr)
        #core.mail("PyTest Confirm Log", strstream.getvalue())

        logging.root.addHandler(core.hdlr)
        ###

    def migrate_results_trees(self, expected_results, confirmed_results):
        cwd = os.getcwd()
        expected_results = moresh.relative_path(expected_results, cwd)
        confirmed_results = moresh.relative_path(confirmed_results, cwd)
        
        # Files are listed in 'topdown' order.
        logging.debug("Migrating %s to %s"%(confirmed_results,expected_results))
        common_files, unique_to_expected, unique_to_confirmed =\
            moresh.compare_trees(expected_results, confirmed_results, ["\.svn"])

        # Overwritten results
        for expected_filepath, confirmed_filepath in common_files:
            mv_cmd = 'mv -f %s %s'%(confirmed_filepath, expected_filepath)
            logging.debug(mv_cmd)
            os.system(mv_cmd)
        
        # Old expected results to be removed
        for expected_filepath in unique_to_expected:
            logging.debug("recursive_remove %s --force"%expected_filepath)
            version_control.recursive_remove(expected_filepath, '--force')

        # New expected results to be added
        for confirmed_filepath in unique_to_confirmed:
            expected_filepath = confirmed_filepath.replace('.confirmed', '')
            assert not os.path.exists(expected_filepath)

            if os.path.isdir(confirmed_filepath) \
               and moresh.is_recursively_empty(confirmed_filepath):
                continue # Do not add empty directory to version control.
            
            rename_cmd = 'os.rename("%s", "%s")'%(confirmed_filepath, expected_filepath)
            logging.debug(rename_cmd)
            eval(rename_cmd)

            version_control.recursive_add(expected_filepath)
            

class FamilyConfigMode(PyTestMode):
    def option_groups(cls, parser):
        return [ cls.target_options(parser) ]
    option_groups = classmethod(option_groups)

    def __init__( self, targets, options ):
        super( FamilyConfigMode, self ).__init__( targets, options )

        # Provide subclasses the occasion to process test after loading but
        # before writing back the config file. Intended for more 'general'
        # use than test_hook() which applies to a sole given test.
        self.setup()
        
        for (family, test_list) in Test._families_map.iteritems():
            config_path  = config_file_path( family )
            if os.path.exists( config_path ):
                toolkit.keep_a_timed_version( config_path )

            config_file  = open(config_path, 'w')

            config_text  = ( '"""Pytest config file.\n\n%s\n"""'% toolkit.doc(Test) )

            for test in test_list:
                self.test_hook( test )
                config_text += "\n%s\n" % str( test )

            config_file.write(config_text)
            config_file.close()    

    def setup(self):
        """For subclass use.

        Provide subclasses the occasion to process test after loading but
        before writing back the config file. Intended for more 'general'
        use than test_hook() which applies to a sole given test.
        """
        pass

    def test_hook(self, test):
        raise NotImplementedError

    def restrictions(self):
        pass
    
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

        add_options.add_option( '-n', '--test-name',
                                help='The name of the test to add.',
                                default='' )

        add_options.add_option( '--category',
                                help='The category to assign to the new test.',
                                default='General' )        
        
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

        add_options.add_option( "--no-pfileprg",
                                action = "store_true",
                                default=False,
                                help="Test's pfileprg will be None." 
                                )

        return [ add_options ]+super(add, cls).option_groups(parser)
    option_groups = classmethod( option_groups )


    def __init__(self, targets, options):
        test_name = options.test_name
        if test_name == '':
            test_name = 'MANDATORY_TEST_NAME' 

        # Avoiding [""] in place of the empty list
        resources = []                               
        if options.resources != "":                 
            resources = options.resources.split(',')

        # Option --no-pfileprg
        overrides = {}
        if options.no_pfileprg:
            overrides['pfileprg'] = None

        # Caught by Test's instances management
        self._added_test = \
            Test( name=test_name, category=options.category,
                  program=Program(name=options.program_name),
                  arguments=options.arguments, resources=resources, **overrides )
        super(add, self).__init__(targets, options)
        print 'Successfully added skeleton for test. Please edit pytest.config file.'

    def setup(self):
        pass
    
    def test_hook(self, test):
        pass
    
class disable(FamilyConfigMode):
    """Disables targeted tests.

    The disabled tests can be restored (L{enable mode<enable>}) afterwards.
    """
    def option_groups(cls, parser):
        disable_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                   "Available under %s mode only." % cls.__name__ )

        disable_options.add_option( '-n', '--test-name',
                                help='The name of the test to disable.',
                                default='' )
        
        return [ disable_options ]+super(disable, cls).option_groups(parser)
    option_groups = classmethod( option_groups )

    def test_hook(self, test):
        test_name = self.options.test_name
        if test_name=="" or test_name==test.name:
            test.disabled = True

class enable(FamilyConfigMode):
    """Enables disabled (L{disable mode<disable>}) tests."""
    def option_groups(cls, parser):
        enable_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                   "Available under %s mode only." % cls.__name__ )

        enable_options.add_option( '-n', '--test-name',
                                help='The name of the test to enable.',
                                default='' )
        
        return [ enable_options ]+super(enable, cls).option_groups(parser)
    option_groups = classmethod( option_groups )

    def test_hook(self, test):
        test_name = self.options.test_name
        if test_name=="" or test_name==test.name:
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
                                                                                                              
        return [ update_options ]+super(update, cls).option_groups(parser)
    option_groups = classmethod(option_groups)                                                                

    def test_hook(self, test):
        """If needed, update the informations related to this test.

        Tests are parsed by L{initialize}, called in the FamilyConfigMode
        template procedure. The PyTest config file docstring is always
        updated to the Test class' docstring by any FamilyConfigMode.
        
        The test_name & new_name options can be used together to rename a test.
        """
        name = self.options.test_name                                                                     
        if name is not None and test.name == name:                                                        
            new_name = self.options.new_name                                                              
            assert new_name is not None,\
                "Options --test-name and --new-name must be used together."                               
                                                                                                          
            assert os.getcwd() == test.directory()                                                        
            results = test.resultsDirectory()                                  
            os.system('svn mv --force %s %s'%(results, results.replace(name, new_name)))                                                                                           
            test.name = new_name
    
class RoutineBasedMode(PyTestMode):
    #
    #  Class methods
    #
    def description( cls ):
        return toolkit.doc( cls.RoutineType )
    description = classmethod( description )
    
    def help( cls ):
        return toolkit.short_doc( cls.RoutineType )
    help = classmethod( help )
    
    def option_groups( cls, parser ):
        return [ cls.target_options(parser),
                 cls.testing_options(parser),
                 cls.restriction_options(parser) ]
    option_groups = classmethod( option_groups )
    
    #
    #  Instance methods
    #    
    def __init__( self, targets, options ):
        logging.debug("--pymake-compile-options (=%s) option forwarded to Program."
                      % options.pymake_compile_options)
        Program.cmdline_compile_options = options.pymake_compile_options

        # The option had to be forwarded to Program *before* calling the
        # inherited ctor since Program instances are instanciated there...
        super(RoutineBasedMode, self).__init__(targets, options)

        test_instances = Test._instances_map.items()        
        self.dispatch_tests(test_instances)

    def dispatch_tests(self, test_instances):
        for (test_name, test) in test_instances:            
            if test.is_disabled():
                logging.debug("Test %s is disabled." % test.name)
                test.setStatus("DISABLED")
            else:
                try:
                    routine = self.RoutineType(test=test)
                    routine.start()
                except core.PyTestUsageError, e:
                    # --traceback: This flag triggers routines to report
                    # the traceback of PyTestUsageError. By default, only the
                    # class's name and meesage are reported.
                    if self.options.traceback:
                        logging.critical( core.traceback(e) )
                    else:
                        logging.debug(e)
                        test.setStatus("SKIPPED", core.traceback(e))
        l = set()
        for (test_name, test) in test_instances:
            if not test.compilationSucceeded() and not test.is_disabled():
                f = test.program.getCompilationLogPath()
                if f not in l:
                    l.add(f)
                    logging.info("Failed compilation log: %s" % f)

class compile(RoutineBasedMode):
    RoutineType = CompilationRoutine

class ResultsBasedMode(RoutineBasedMode):
    def option_groups(cls, parser):
        ogroups = RoutineBasedMode.option_groups(parser)

        ogroups[1].add_option( '--no-compile', default=False,
                               action="store_true",
                               help='Any program compilation is bypassed.' )
        ogroups[1].add_option( '--showtime', default=False,
                               action = "store_true",
                               help="If true, print the run time and diff time" )

        return ogroups        
    option_groups = classmethod(option_groups)

    def __init__(self, targets, options):
        logging.debug("--no-compile (=%s) option forwarded to Program."%options.no_compile)
        Program.compilation_disabled = options.no_compile        
        logging.debug("--showtime (=%s) option forwarded to Program."%options.showtime)
        Program.showtime = options.showtime
        super(ResultsBasedMode, self).__init__(targets, options)
    
class results(ResultsBasedMode):
    RoutineType = ResultsCreationRoutine

    def option_groups(cls, parser):
        ogroups = super(results, cls).option_groups(parser)

        ogroups[1].add_option(
            '-f', '--force', default=False,
            action="store_true",
            help="If True, new results will be generated without prompting, even "
            "if results already are under version control." )

        return ogroups
    option_groups = classmethod(option_groups)

    def __init__(self, targets, options):
        logging.debug("--force (=%s) option forwarded to Test."%options.force)
        Test.force_results_creation = options.force        
        super(results, self).__init__(targets, options)
        if TestStatus.nPassed() > 0:
            logging.warning("\nNew results were generated. Verify their validity, and "
                            "if they are correct, run 'pytest confirm' to confirm the "
                            "new results before your commit.")

class run(ResultsBasedMode):    
    RoutineType = RunTestRoutine

class rundiff(ResultsBasedMode):
    """Redo the diff of last execution.
    
    Usage: pytest rundiff <test_name>
    """
    RoutineType = DiffTestRoutine

    def empty(self):
        pass

    def __init__(self, targets, options):
        super(rundiff, self).__init__(targets, options)

#######  Builtin Unit Tests  ##################################################

def testAllModes():
    import os, sys
    def vsystem(cmd):        
        print >>sys.stderr, '#  %s\n' % cmd
        os.system( '%s >& /dev/null'%cmd )
        # os.system( cmd )
        print >>sys.stderr, ''

    ## Since run results are not under version control...
    os.chdir(os.path.join(os.path.dirname(__file__), '.pytest/svn_tests'))

    ## Creates a new test script
    f = open('INTERNAL_SCRIPT.py', 'w')
    f.write("print 'INTERNAL_SCRIPT'\n")
    f.close()
    
    ## Add it
    vsystem("pytest add -n PYTEST_INTERNAL --program-name python "
              "--arguments INTERNAL_SCRIPT.py --resources INTERNAL_SCRIPT.py --no-pfileprg")
    
    ## Generate results & run the test
    vsystem("pytest results")
    vsystem("pytest run")
    
    ## confirm && commit
    vsystem('pytest confirm')
    vsystem('svn commit -m "PYTEST INTERNAL TEST"')
        
    ## shutil.rmtree
    print >>sys.stderr, '#\n#  shutil.rmtree\n#'
    shutil.rmtree(ppath.pytest_dir)
    
    ## svn up && run
    vsystem('svn up')
    vsystem('pytest run')
    
    ## svn remove -f
    vsystem('svn remove --force .pytest pytest.config')
    vsystem('svn commit -m "PYTEST INTERNAL TEST"')
    os.remove('INTERNAL_SCRIPT.py')

    ## Verify that directory is empty
    print os.listdir(os.getcwd())


