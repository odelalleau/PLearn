
import os, shutil, string

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
    'config_file_path',

    ## Exceptions
    'PyTestUsageError',

    ## Classes
    'Test', 
    'Routine',
##    'AddTestRoutine',
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

## def disable_file_name(directory, test_name=''):
##     if test_name == '':
##         test_name = 'pytest'

##     test = os.path.join(directory, test_name)
##     return ( test, test+'.disabled' )

class DuplicateName( PyTestUsageError ):
    def __init__(self, test1, test2):
        PyTestUsageError.__init__(
            self, "Two tests have the same name: %s and %s."
            % ( test1.get_path(), test2.get_path() )
            )        

class Resources:
    md5_mappings = {}

    def link_resource(cls, path_to_resource, target):
        link_cmd = "ln -s %s %s" % ( path_to_resource, target )
        vprint( "Linking resource: %s." % link_cmd, 2 )
        os.system( link_cmd )
    link_resource = classmethod(link_resource)
    
    ## Class methods
    def link_resources(cls, path_to, resources, target): 
        for resource in resources:
            if not os.path.isabs( resource ):
                resource = os.path.join( path_to, resource ) 
            if not os.path.exists( resource ):
                raise PyTestUsageError(
                    "In %s: %s used as a resource but path doesn't exist."
                    % ( os.getcwd(), resource )
                    )

            cls.link_resource( resource )

            if toolkit.isvmat( resource ):
                meta = resource+'.metadata'
                if os.path.exists( meta ):
                    cls.link_resource( meta )       
    link_resources = classmethod(link_resources)

    def md5sum(cls, path_to_ressource):
        if md5_mappings.has_keys(path_to_ressource):
            return md5_mappings[path_to_ressource]
        
        md5 = toolkit.command_output( 'md5sum %s'
                                      % path_to_ressource)
        md5_mappings[path_to_ressource] = md5
        return md5
    md5sum = classmethod(md5sum)
        
class TestDefaults:
    name            = None
    description     = ''
    program         = None
    arguments       = ''
    resources      = []
    disabled        = False

    __declare_members__ = [ ('name',            types.StringType),
                            ('description',     types.StringType),
                            ('program',         Program),
                            ('arguments',       types.StringType),
                            ('resources',      types.ListType),
                            ('disabled',        types.BooleanType)
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
    
    def __init__(self, defaults=TestDefaults, **overrides):
        FrozenObject.__init__(self, defaults, overrides)        
        self.set_attribute( 'test_directory', os.getcwd() )

        if Test.instances_map.has_key( self.name ):
            raise DuplicateName( Test.instances_map[self.name], self )
        else:
            Test.statistics.new_test()
            Test.instances_map[self.name] = self
            if Test.families_map.has_key( self.test_directory ):
                Test.families_map[self.test_directory].append(self)
            else:
                Test.families_map[self.test_directory] = [self]
                
        self.set_str_spacer( '\n' )

        if overrides.has_key('ressources'):
            self.set_attribute('__ressources', None)
            self.resources = overrides['ressources']

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

        if hasattr(self, '__ressources'):
            raise PyTestUsageError(
                "The use of Test.ressources is deprecated! Test.resources must "
                "be used instead. Use the PyTest 'update' mode the correct your "
                "PyTest config file."
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
                    answer = raw_input( "Results %s already exists! Are you sure you want to "
                                        "generate new results (yes or no)? " % results )

                if answer == 'no':
                    raise PyTestUsageError("Results creation interrupted by user")

                ## YES
                os.system( "cvs remove -Rf %s" % results )
                cvs.commit( '.',
                            'Removal of %s for new results creation.'
                            % results )
                
            backup = "%s.%s" % (results, toolkit.date_time_string())
            os.system( "mv %s %s" % (results, backup) )
        os.makedirs( results )
        return backup

    def get_path(self):
        return os.path.join(
            self.test_directory,
            self.name
            )

    def is_disabled(self):
##         disabled_ext         = '.disabled'
##         disabling_directory  = os.path.join(self.test_directory, 'pytest'+disabled_ext) 
##         disabling_test       = os.path.join(self.test_directory, self.name+disabled_ext) 
##         return ( os.path.exists(disabling_directory) or
##                  os.path.exists(disabling_test)       )
        return self.disabled
    
    def link_resources(self, test_results):
        resources = []
        resources.extend( self.resources )
        resources.append( self.program.path )

        def single_link(resource):
            link_cmd = "ln -s %s %s" % ( resource, test_results )
            vprint( "Linking resource: %s." % link_cmd, 2 )
            os.system( link_cmd )
            
        for resource in resources:
            if not os.path.isabs( resource ):
                resource = os.path.join( self.test_directory, resource ) 
            if not os.path.exists( resource ):
                raise PyTestUsageError(
                    "The %s test uses %s as a resource but path doesn't exist."
                    % ( self.name, resource )
                    )

            single_link( resource )

            if toolkit.isvmat( resource ):
                meta = resource+'.metadata'
                if os.path.exists( meta ):
                    single_link( meta )

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
        if backup is not None:
            vprint("Previous results saved in %s" % backup )

        self.link_resources( test_results )
        run_command   = ( "%s %s >& %s"
                          % ( self.program.get_name(), self.arguments, self.name+'.run_log' )
                          )
        
        vprint("Test name:   %s.\nDescription:\n    %s" % (self.name, self.description), 1)        
        vprint(run_command, 1)

        cwd = os.getcwd()
        os.chdir( test_results )
        os.system(run_command)
        os.chdir( cwd )
        
##         ## Forwarding the removal of the old results: if any operation
##         ## should cause the crash of PyTest, the old results would
##         ## still be available in the backup directory.
##         if backup is not None:
##             shutil.rmtree( backup )
        self.unlink_resources( test_results )
        os.putenv("PLEARN_DATE_TIME", "YES")

    def unlink_resources(self, test_results):
        dirlist = os.listdir( test_results )
        for f in dirlist:
            path = os.path.join( test_results, f )
            if os.path.islink( path ):
                vprint( "Removing link: %s." % path, 2 ) 
                os.remove( path )

class RoutineDefaults(TaskDefaults):
    test                  = None
    
class Routine(Task):

    report_traceback = False
    
    def __init__( self, test,
                  defaults=RoutineDefaults, **overrides ):
        overrides['task_name'] = test.name
        Task.__init__( self, defaults, **overrides ) 

        ## os.chdir( test_suite_dir(directory) )
        ## os.chdir( directory )
        self.test = test

    def compile_program(self):
        if not isinstance(self.test.program, Compilable):
            return True

        vprint("\nCompilation:", 1)
        vprint("------------", 1)
        
        self.test.compile()
        if not self.test.compilation_succeeded():
            vprint("Compilation failed.", 1)
            self.succeeded(False)
            return False
        vprint("Compilation succeedded.", 1)
        return True

    ## Overrides run and succeeded
    def run(self):
        try:
            if self.test.is_disabled():
                vprint("Test %s is disabled." % self.test.name, 1)
                Test.statistics.skip()
                self.signal_completion(self)
            else:
                os.chdir( self.test.test_directory )
                Task.run(self)
        except PyTestUsageError, e: 
            Test.statistics.skip()
            if Routine.report_traceback:
                raise
            else:
                vprint( "%s: %s." % (e.__class__.__name__,e) )
            
    def succeeded(self, success):
        Task.succeeded(self, success)
        if success:
            Test.statistics.success()
        else:
            Test.statistics.failure( self.test.get_path() )

class AddTestRoutine(Routine):
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
##     A user familiar with config files can also pass directly through
##     options in the command line the keyword arguments to fill the
##     L{Test} declaration with.

    def body_of_task(self):
        config_path  = config_file_path( self.test.program.processing_directory )
        config_file  = None
        config_text  = ''
    
        initial_creation = not os.path.exists( config_path )
        if initial_creation:
            config_file = open(config_path, 'w')
            config_text = ( '"""Pytest config file.\n\n%s\n"""'% toolkit.doc(Test) )
        else:
            config_file = open(config_path, 'a+')
            
        config_text += str( self.test ) + '\n'
        config_file.write(config_text)
        config_file.close()
        self.succeeded(True)
    
class CompilationRoutine(Routine):
    """Launches the compilation of target tests' compilable files."""    
    def body_of_task(self):
        if self.compile_program():
            self.succeeded( True )
            
class DisableRoutine(Routine):
    """Disables an existing test.

    The disabled test can be restored (L{enable mode<EnableRoutine>}) afterwards.
    """
    def body_of_task(self):
        test, dis = disable_file_name( self.test.test_directory,
                                       self.test.name            )
        if os.path.exists( dis ):
            vprint('%s was already disabled.' % test)
        else:
            os.system("touch %s" % dis)
            vprint('%s is disabled.' % test, 2)
        self.succeeded( True )

class EnableRoutine(Routine):
    """Enables a disabled (L{disable test<disable_test>}) test."""
    def body_of_task(self):
        test, dis = disable_file_name( self.test.test_directory,
                                       self.test.name            )

        if os.path.exists( dis ):
            os.remove(dis)
            vprint('%s is enabled.' % test, 2)
        else:
            vprint('%s was not disabled.' % test)
        self.succeeded( True )
    
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
    def body_of_task(self):
        compilation_succeeded = self.compile_program()
        if not compilation_succeeded:
            vprint("Results creation bails out.", 1)
            return
            
        vprint("\nResults creation:", 1)
        vprint("-----------------", 1)
        
        self.test.run( Test.expected_results )

        mappings = { os.path.abspath(Test.expected_results): "$RESULTS" }
        mappings.update( plpath.env_mappings )
        plpath.process_with_mappings( Test.expected_results, mappings )
        
        vprint("", 1)
        self.succeeded( True )

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
        compilation_succeeded = self.compile_program()
        if not compilation_succeeded:
            vprint("Running bails out.", 1)
            return
        
        vprint("\nRunning the test:", 1)
        vprint("-----------------", 1)
        
        self.test.run( Test.run_results )

        mappings = { os.path.abspath(Test.run_results): "$RESULTS" }
        mappings.update( plpath.env_mappings )
        plpath.process_with_mappings( Test.run_results, mappings )

        idiff  =  IntelligentDiff()
        diffs  =  idiff.diff( self.expected_results, self.run_results )
        if diffs == []:
            self.succeeded( True )
        else:            
            map(lambda err: vprint(err, 1), diffs)
            self.succeeded( False )
