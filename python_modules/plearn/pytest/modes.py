from   test_and_routines                   import *
from   ModeAndOptionParser                 import ModeAndOptionParser, OptionGroup
from   plearn.tasks.dispatch               import *
from   plearn.utilities.toolkit            import *
from   plearn.utilities.verbosity          import *
from   plearn.utilities.global_variables   import *

current_mode  = None

dispatch      = None
targets       = None
options       = None

routine_mappings = {
    'add'     : AddTestRoutine,
    'compile' : CompilationRoutine,
    'results' : ResultsCreationRoutine,
    'run'     : RunTestRoutine
    }

def build_tests(mode_name, directory, dirlist):
    if globalvars.ignore_hierarchy in dirlist:
        toolkit.exempt_list_of( dirlist, copy.copy(dirlist) )
        vprint( "Directory %s and subdirectories ignored (%s)."
                % ( directory, globalvars.ignore_hierarchy )
                )
        return
    
    toolkit.exempt_list_of( dirlist, Routine.forbidden_directories() )
    
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

    if mode_name == "add":
        Test( name    = options.test_name,
              program = GlobalCompilableProgram( name = "plearn")  )        

def disable( disabled = True ):
    """Disables targeted tests.

    The disabled tests can be restored (L{enable mode<enable>}) afterwards.
    """
    def set_disabled( test ):
        test.disabled = disabled

    family_generic_processing( test_hook = set_disabled )

def dispatch_tests( test_instances ):
    for (test_name, test) in test_instances:
        RoutineType = routine_mappings[current_mode.name]
        dispatch.add_task( RoutineType(test) )
    dispatch.run()

def enable():
    """Enables disabled (L{disable mode<mode_disable>}) tests."""
    disable( False )

## def family_generic_processing( family_hook = ( lambda f,t: None ),
##                                test_hook   = ( lambda t:   None ) ):
def family_generic_processing( test_hook   = ( lambda t:   None ) ):
    initialize()
    parse_config_files()

    for (family, tests) in Test.families_map.iteritems():
        config_path  = config_file_path( family )
        if os.path.exists( config_path ):
            os.system( "cp %s %s.%s"
                       % ( config_path, config_path,
                           toolkit.date_time_string())
                       )
            
        config_file  = open(config_path, 'w')

        config_text  = ( '"""Pytest config file.\n\n%s\n"""'% toolkit.doc(Test) )

        for test in tests:
            test_hook( test ) 
            config_text += str( test ) + '\n'

        config_file.write(config_text)
        config_file.close()    
    
def generic_processing():
    initialize()
    parse_config_files()
    
    test_instances = None
    if ( options.test_name != globalvars.test_name_default
         or current_mode.name == 'add' ):
        test_instances = [( options.test_name,
                            Test.instances_map[ options.test_name ]
                            )]
    else:
        test_instances = Test.instances_map.items()

    dispatch_tests( test_instances )
    print_stats()

def initialize():
    global dispatch, targets
    
    dispatch = Dispatch(
        localhost = options.localhost,
        nb_hosts  = options.hosts
        )
    
    ## --traceback: This flag triggers routines to report the traceback of
    ## PyTestUsageError. By default, only the class's name and meesage
    ## are reported.
    Routine.report_traceback = options.traceback
    
    ## --all: Run all tests found in subdirectories of directories in
    ## globalvars.all_roots test_suite branches. If some targets are
    ## provided, these will be ignored.
    if options.all:
        targets           = globalvars.all_roots
        options.recursive = True

    ## If no targets are provided, the cwd is the default target.
    if len(targets) == 0:
        targets.append( os.getcwd() )

    if options.recursive:
        targets = plpath.exempt_of_subdirectories( targets )


def list():
    """Lists all tests within target directories."""
    initialize()
    parse_config_files()

    formatted_string = lambda n,d: ( "%s Disabled: %s"
                                     % (string.ljust(n, 25), string.ljust(str(d), 15))
                                     )
    for (family, tests) in Test.families_map.iteritems():
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
            vprint( "In %s:\n    %s\n"
                    % ( family, string.join(formatted_strings, '\n    ') )
                    )    
        
def list_options( parser ):
    list_options = OptionGroup( parser, "Mode Specific Options",
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

    return list_options

def parse_config_files():
    for target in targets:
        if options.recursive:
            os.path.walk(target, build_tests, current_mode.name)
        else:
            build_tests(current_mode.name, target, os.listdir(target))

def print_stats():
    vprint("\n%s" % str(Test.statistics), 0)    
