
import os, shutil, string

from plearn.tasks.Task              import *
from plearn.utilities.verbosity     import *
from plearn.utilities.FrozenObject  import *

## In plearn.pytest
from threading                      import *
from programs                       import *
from BasicStats                     import BasicStats
from IntelligentDiff                import *          

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

def disable_file_name(directory, test_name=''):
    if test_name == '':
        test_name = 'pytest'

    test = os.path.join(directory, test_name)
    return ( test, test+'.disabled' )

    
def disable_test(directory, test_name):
    """Disables an existing test.

    The disabled test can be restored (L{enable test<enable_test>}) afterwards.
    """   
    test, dis = disable_file_name(directory, test_name)
        
    if os.path.exists( dis ):
        vprint('%s was already disabled.' % test)
    else:
        os.system("touch %s" % dis)
        vprint('%s is disabled.' % test, 2)

def enable_test(directory, test_name):
    "Enables a disabled (L{disable test<disable_test>}) test."    
    test, dis = disable_file_name(directory, test_name)

    if os.path.exists( dis ):
        os.remove(dis)
        vprint('%s is enabled.' % test, 2)
    else:
        vprint('%s was not disabled.' % test)

class PyTestUsageError(Exception): pass

class DuplicateName( PyTestUsageError ):
    def __init__(self, test1, test2):
        self.p1 = test1.get_path()
        self.p2 = test2.get_path()

    def __str__(self):
        return ( "Two tests have the same name: %s and %s."
                 % ( self.p1, self.p2 )
                 )
    
class TestDefaults:
    name            = None
    description     = ''
    program         = None
    arguments       = ''
    ressources      = []

    __declare_members__ = [ ('name',            types.StringType),
                            ('description',     types.StringType),
                            ('program',         Program),
                            ('arguments',       types.StringType),
                            ##('ressources',      Ressources),
                            ('ressources',      types.ListType) ]

class Test(FrozenObject):
    """Test is a class regrouping the elements that define a test for PyTest."""            
    instances_map    = {}
    statistics       = BasicStats("Test statistics")
    expected_results = os.path.join(plpath.pytest_dir, "expected_results")
    run_results      = os.path.join(plpath.pytest_dir, "run_results")


    def __init__(self, defaults=TestDefaults, **overrides):
        FrozenObject.__init__(self, defaults, overrides)

        if Test.instances_map.has_key( self.name ):
            raise DuplicateName( Test.instances_map[self.name], self )
        else:
            Test.statistics.new_test()
            Test.instances_map[self.name] = self

        self.set_attribute( 'test_directory', os.getcwd() )
        self.set_str_spacer('\n')

    def check_name_format(self):
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
        self.check_name_format()        
        self.program.compile()

    def ensure_results_directory(self, results):
        backup = None
        if os.path.exists( results ):
            backup = results + ".BAK"
            os.system( "cp -R %s %s" % (results, backup) )
            plpath.keep_only(results, plpath.cvs_directory)
        else:
            os.makedirs( results )
        return backup

    def get_path(self):
        return os.path.join(
            self.test_directory,
            self.name
            )

    def is_disabled(self):
        disabled_ext         = '.disabled'
        disabling_directory  = os.path.join(self.test_directory, 'pytest'+disabled_ext) 
        disabling_test       = os.path.join(self.test_directory, self.name+disabled_ext) 
        return ( os.path.exists(disabling_directory) or
                 os.path.exists(disabling_test)       )

    def link_ressources(self, test_results):
        ressources = []
        ressources.extend( self.ressources )
        ressources.append( self.program.path )
        
        for ressource in ressources:
            if not os.path.isabs( ressource ):
                ressource = os.path.join( self.test_directory, ressource ) 

            link_cmd = "ln -s %s %s" % ( ressource, test_results )
            vprint( "Linking ressource: %s." % link_cmd, 2 )
            os.system( link_cmd )
        
    def run(self, results):
        self.check_name_format()
        
        if results not in [Test.expected_results, Test.run_results]:
            raise ValueError(
                "%s.run expects its results argument to be either "
                "%s.expected_results or %s.run_results"
                % (self.classname(), self.classname(), self.classname())
                )

        test_results  = os.path.join( results, self.name )
        backup        = self.ensure_results_directory( test_results )
        self.link_ressources( test_results )

        run_command   = ( "%s %s >& %s"
                          % ( self.program.name, self.arguments, self.name+'.run_log' )
                          )
        
        vprint("Test name:   %s.\nDescription:\n    %s" % (self.name, self.description), 1)        
        vprint(run_command, 1)

        cwd = os.getcwd()
        os.chdir( test_results )
        os.system(run_command)
        os.chdir( cwd )

        ## Forwarding the removal of the old results: if any operation
        ## should cause the crash of PyTest, the old results would
        ## still be available in the backup directory.
        if backup is not None:
            shutil.rmtree( backup )
        self.unlink_ressources( test_results )

    def unlink_ressources(self, test_results):
        dirlist = os.listdir( test_results )
        for f in dirlist:
            path = os.path.join( test_results, f )
            if os.path.islink( path ):
                vprint( "Removing link: %s." % path, 2 ) 
                os.remove( path )

class RoutineDefaults(TaskDefaults):
    test                  = None
    
class Routine(Task):
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

    def forbidden_directories():
        return [ plpath.pymake_objs, "BACKUP", plpath.cvs_directory,
                 ".pymake", Test.expected_results, Test.run_results ]
    forbidden_directories = staticmethod(forbidden_directories)

    def path_resolve(self, results):
        os.path.walk( results,
                      path_resolve_dir,
                      { plpath.home                : "$HOME",
                        Test.expected_results      : "$RESULTS",
                        Test.run_results           : "$RESULTS",
                        forbidden_flag             : self.forbidden_directories() }
                      )

    ## Overrides run and succeeded
    def run(self):
        if self.test.is_disabled():
            Test.statistics.skip()
        else:
            os.chdir( self.test.test_directory )
            Task.run(self)
    
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
            
    A user familiar with config files can also pass directly through
    options in the command line the keyword arguments to fill the
    L{Test} declaration with.
    """
    def body_of_task(self):
        config_path  = config_file_path( self.test.program.processing_directory )
        config_file  = None
        config_text  = ''
    
        initial_creation = not os.path.exists( config_path )
        if initial_creation:
            config_file = open(config_path, 'w')
            config_text = ( '"""Pytest config file.\n\n%s\n"""'%Test.__doc__ )
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
        
        vprint("\nResults creation:", 1)
        vprint("-----------------", 1)
        
        self.test.run( Test.expected_results )
        self.path_resolve( Test.expected_results )
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
        if not os.path.exists( Test.expected_results ):
            raise PyTestUsageError(
                "%s\n Expected results must be generated by the 'results' mode "
                "prior to any use of the 'run' mode."
                % self.test.get_path()
                )
    
    def body_of_task(self):
        compilation_succeeded = self.compile_program()
        if not compilation_succeeded:
            vprint("Running bails out.", 1)
        
        vprint("\nRunning the test:", 1)
        vprint("-----------------", 1)
        
        self.test.run( Test.run_results )

        self.path_resolve( Test.run_results )
        verif = IntelligentDiff( Test.expected_results,
                                 Test.run_results,
                                 self.forbidden_directories() )
        
        diffs = verif.get_differences()
        if diffs == []:
            self.succeeded( True )
        else:            
            map(lambda err: vprint(err, 1), diffs)
            self.succeeded( False )
