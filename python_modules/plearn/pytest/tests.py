import shutil, types
import plearn.utilities.plpath as plpath           

from plearn.utilities.global_variables  import *
from plearn.utilities.verbosity         import vprint

from __init__                           import test_suite_dir
from plearn.tasks.Task                  import Task

from BasicStats                         import BasicStats
from CompilableProgram                  import *
from IntelligentDiff                    import *          

__all__ = [ "add_global_program", "complete_test_name", "define_test",
            "get_add_options", "relative_path",
            "DefineTest",
            "Test",        "AddTest",     "CompileTest", "DisableTest",
            "ResultsTest", "RestoreTest", "RunTest",     "SyncTest"    ]

global_programs = {}

## vprint          = None
## def set_tests_verbosity(vpr):
##     global vprint 
##     vprint = vpr

def add_global_program(test_directory, prog_name):
    """Properly adds a CompilableProgram object to global_programs.

    The only global compilable programs currently supported are the branches
    plearn mains.
    """
    ts_dir = test_suite_dir(test_directory)
    plearn = globalvars.branches[ts_dir]
    if string.find(plearn, prog_name) == -1:
        raise ValueError('The %s global_pymake_prog is not supported yet.'%prog_name)
    
    global_programs[prog_name] = CompilableProgram(plearn)

def complete_test_name(directory):
    return os.path.join( globalvars.pure_branches[test_suite_dir(directory)],
                         relative_path(directory) )

def define_test(directory, prog_name, copt, args, cvs_files, compiler):    
    mode_test = globalvars.modes_with_config[globalvars.current_mode.name]

    globalvars.test_id += 1
    test = mode_test( globalvars.dispatch, globalvars.test_id, directory, None,
                      prog_name, copt, args, cvs_files, compiler)

    if not globalvars.stats.has_key(test.classname()):
        globalvars.stats[test.classname()] = BasicStats(test.classname())
    globalvars.stats[test.classname()].new_test()

    return test

def get_add_options(parser):
    return DefineTest().command_line_options(parser)
    
def relative_path(path):
    tsdir = test_suite_dir(path)
    if string.find(path, tsdir) == -1:
        raise FormatError(path + " must begin with %s" % tsdir)

    rel_path = path[len(tsdir):]
    if len(rel_path) != 0 and rel_path[0] == '/':
        rel_path = rel_path[1:]
    return  rel_path

class DefineTest(WithOptions):
    """Small class managing the configuration provided in the config file."""

    # The following dictionaries are lists of options that may be provided
    # to the DefineTest constructor. NOTE THAT the DefineTest constructor ONLY accepts
    # keyword arguments, that is that you must provide the options the following manner, e.g.
    #            DefineTest( local_pymake_name='bid',
    #                        pymake_options='-g++ -dbg' )
    # where the order DOES NOT matter.

    # Defining the program name. These are MUTUALLY EXCLUSIVE OPTIONS.
    #  - 'local' means that the program is located in the current directory
    #  - 'global' means that the program is accessible in the user's path (shell variable).
    #  - A 'pymake_name' means that the program must be compiled with pymake prior to be ran
    #    while a simple 'prog_name' may be any shell or python script/command you like.
    # The argument must be a string.
    PROG_NAME = { 'local_prog_name':None,
                  'global_prog_name':None,
                  'local_pymake_name':None,
                  'global_pymake_name':None }

    # The 'X_options' can be provided only if the PROG_NAME is a 'X_name' and will be used to invoke
    # the compilation process. Must be a string or a list of strings. If it's a list, the test will be
    # repeated for every options configuration in the list. These are MUTUALLY EXCLUSIVE OPTIONS.
    COMPILE_OPTIONS = { 'pymake_options': None }

    # The argument string (or list of strings) to provide the PROG_NAME. Again,
    # if the it is a list, the test will be repeated (for every options configuration in
    # the COMPILE_OPTIONS list) for every arguments configuration in the 'test_args'
    # list.
    # 
    # The 'local_files' (array of strings) in the 'test_arguments' MUST be highlighted.
    # For example
    #            DefineTest( local_pymake_name='bid',
    #                        pymake_options='-g++ -dbg' 
    #                        test_args='--db data.amat --log log.txt',
    #                        local_files=['data.amat'] )
    # means that the data.amat file is located in the test's directory. Note that
    # local_files='data.amat' would also be understood by pytest.
    ARGUMENTS = { 'test_args':'',
                  'local_files':[]}

    # An array (or a single file name [string]) 'related_files' are to be managed (added or commited)
    # under pytest's cvs mode.  NOTE that if you do not include local_files, these will still be
    # considered.
    CVS_FILES = { 'related_files':[] }

    def __init__(self, **kwargs):
        WithOptions.__init__(self)
        
        self.define_options(self.PROG_NAME)
        self.define_options(self.COMPILE_OPTIONS)
        self.define_options(self.ARGUMENTS)
        self.define_options(self.CVS_FILES)

        # This class and the Test class may be merged in the future.
        # For consistency:
        self.define_options(Task.OPTIONS)

        if len(kwargs) > 0:
            self.set_options(kwargs)
            
            self.set_prog_name()
            self.set_compile_options()
            self.set_arguments()
            self.set_cvs()
            self.add_tests()

    def __str__(self):
        return ("DefineTest( " +
                "PROG_NAME= '" + self.prog_name + "', " +
                "COMPILE_OPTIONS= " + str(self.compile_options) + ", " +
                "ARGUMENTS= " + str(self.arguments) + ", " +
                "CVS_FILES= " + str(self.cvs_files) + ") ")

    def __repr__(self):
        return str(self)

    def add_tests(self):
        arg_pairs = []
        if globalvars.current_mode.name in ['commit']:
            arg_pairs.append((None, None))
        else:
            for copt in self.compile_options:
                for args in self.arguments:
                    arg_pairs.append((copt, args))

        for (copt, args) in arg_pairs:
            globalvars.dispatch.add_task( define_test(os.getcwd(), self.prog_name,
                                          copt, args,
                                          self.cvs_files, self.compiler) )

    def check_string_types(self, opt_name, error_msg):
        option = self.get_option(opt_name)
        if isinstance(option, types.StringType):
            self.set_option(opt_name, [option])
        elif not isinstance(option, type([])):
            raise TypeError("The type of '%s' (%s) is not valid.\n%s"
                            % (opt_name, type(self._test_args), error_msg)           )        

    def set_arguments(self):
        """                    
        The argument string (or list of strings) to provide the PROG_NAME. Again,
        if the it is a list, the test will be repeated (for every options configuration in
        the COMPILE_OPTIONS list) for every arguments configuration in the 'test_args'
        list.
        
        The 'local_files' (array of strings) in the 'test_arguments' MUST be highlighted.
        For example::
            DefineTest( local_pymake_name='bid',
                        pymake_options='-g++ -dbg' 
                        test_args='--db data.amat --log log.txt',
                        local_files=['data.amat'] )
        means that the data.amat file is located in the test's directory. Note that
        local_files='data.amat' would also be understood by pytest.

        Supported arguments::
            ARGUMENTS = { 'test_args':'',
                          'local_files':[]}
        """
        self.check_string_types('test_args', self.set_arguments.__doc__)
        self.check_string_types('local_files', self.set_arguments.__doc__)

        self.arguments = []
        for args in self._test_args:
            for f in self._local_files:
                if f not in self._related_files:
                    self._related_files.append(f)
                args = string.replace(args, f, os.path.abspath(f))
            self.arguments.append(args)
            
        if self.arguments == []:
            self.arguments = ['']
        
    def set_compile_options(self):
        """
        The 'X_options' can be provided only if the PROG_NAME is a 'X_name' and will be used to invoke
        the compilation process. Must be a string or a list of strings. If it's a list, the test will be
        repeated for every options configuration in the list. These are MUTUALLY EXCLUSIVE OPTIONS.

        COMPILE_OPTIONS = { 'pymake_options': None }
        """
        if self.compiler != 'pymake' and self._pymake_options is not None:
            raise ValueError("Provided pymake options while the compiler is %s\n%s"
                             % (self.compiler, self.set_compile_options.__doc__)  )

        if self._pymake_options is not None:
            self.check_string_types('pymake_options', self.set_compile_options.__doc__)
        else:
            self._pymake_options = ['']
            
        self.compile_options = self._pymake_options

    def set_compiler(self, prog_name_type):
        self.compiler = None
        if string.find(prog_name_type, 'pymake'):
            self.compiler = 'pymake'
        
    def set_cvs(self):
        """
        An array (or a single file name [string]) 'related_files' are to be managed (added or commited)
        under pytest's cvs mode. NOTE that if you do not include local_files, these will still be
        considered.
        CVS_FILES = { 'related_files':[] }
        """
        self.check_string_types('related_files', self.set_cvs.__doc__)
        self.cvs_files = self.get_option('related_files')
        
    def set_prog_name(self):
        """Defines the program name.

        These are MUTUALLY EXCLUSIVE OPTIONS:
            - 'local' means that the program is located in the current directory
            - 'global' means that the program is accessible in the user's path (shell variable).
            - A 'pymake_name' means that the program must be compiled with pymake prior to be ran
            while a simple 'prog_name' may be any shell or python script/command you like.
            The argument must be a string.
            
        Supported::
            PROG_NAME = { 'local_prog_name':None,
                          'global_prog_name':None,
                          'local_pymake_name':None,
                          'global_pymake_name':None }
        """
        count = 0
        possible_names = self.PROG_NAME.keys()
        for possible in possible_names:
            provided = self.get_option(possible)
            if provided is not None:          
                if not isinstance(provided, types.StringType):
                    raise TypeError("The provided '%s' should be a string.\n%s"
                                    %(possible, self.set_prog_name.__doc__)   )
                count += 1
                self.prog_name = provided
                self.local_prog = (string.find(possible, 'local') != -1)
                self.set_compiler(provided)
                
        if count != 1:
            raise ValueError("Exactly one of %s must be provided.\n%s"
                             % (str(possible_names), self.set_prog_name.__doc__)      )

        if not self.local_prog:
            add_global_program(os.getcwd(), self.prog_name)

        
class Test(Task):    
    def __init__(self, dispatch, test_id, directory, parent,
                 prog_name=None, compile_options=None, arguments=None, cvs_files=None, compiler=None):
        os.chdir( test_suite_dir(directory) )

        self.prog_name = prog_name
        self.compile_options = compile_options
        self.arguments = arguments
        self.cvs_files = cvs_files
        self.compiler = compiler

        Task.__init__(self, dispatch, test_id,
                      self.build_cmd_line(directory, prog_name, compile_options,
                                          arguments, cvs_files, compiler))

        self.directory = directory
        self.set_option('parent_name',parent)
        self.set_option('quote_cmd_line', True)

        # The results directory will be built for the given options
        if self.prog_name is not None:
            self.set_results_directory()

    def _failed(self):
        if globalvars.management_modes.has_key(globalvars.current_mode.name):
            return
        globalvars.stats[self.classname()].failure( complete_test_name(self.directory) )

    def _succeeded(self):
        if globalvars.management_modes.has_key(globalvars.current_mode.name):
            return
        globalvars.stats[self.classname()].success()
        
    def build_cmd_line( self, directory, prog_name,
                        compile_options, arguments, cvs_files, compiler ):
        raise NotImplementedError("%s.build_cmd_line() was not implemented.\n"
                                  "(Test.build_cmd_line() is abstract)" % self.classname())

    def set_results_directory(self):
        self._results_base = os.path.join(self.directory, globalvars.results_path)

        args = string.replace(self.arguments, self.directory, '')
        if len(args) != 0 and args[0] == '/':
            args = args[1:]
        
        res_dir = self._results_base
        for s in [self.prog_name,self.compile_options,args]:
            s = string.replace(s, ' ', '_')
            s = string.replace(s, '-', '_')
            s = string.replace(s, '/', '_')
            res_dir = os.path.join(res_dir, s)
        self.set_option('results_directory', res_dir)

class AddTest(Test):
    """Enables pytest to run the tests in a given directory.
    
    A directory is considered to be a test directory as soon as it
    contains a pytest.config file. Therefore, this mode simply drops
    a config file with some explanation how to instanciate the
    DefineTest objects within the file.

    The config file is executed within the pytest script. So, it IS
    in python, which means that one can add any comments he wishes
    (each line preceded by at least one #) and may also define is one
    functions if complicated processing is requested before defining
    the test. NOTE however that, if you do so, it is strongly suggested
    to preceed the name of anything you define with two underscores at
    to end the name with a trailing underscore, e.g::
    
        def __user_defined_bla_(...)

    A user familiar with config files can also pass directly through
    options in the command line the keyword arguments to fill the
    DefineTest declaration with.
    """
    
    def __init__(self, dispatch, test_id, directory, parent):
        Test.__init__(self, dispatch, test_id, directory, parent)
        
    def _do_not_run(self):
        if os.path.exists(self.config):
            print("%s exists: no tests to add"
                  % relative_path(self.config) )
            return True

        current_test_name = complete_test_name(self.directory)
        if current_test_name in globalvars.from_file.disabled:
            vprint("Can not add %s: it is disabled... Restore it first!")
            return True

        text = ( DefineTest.set_prog_name.__doc__       +
                 DefineTest.set_compile_options.__doc__ +
                 DefineTest.set_arguments.__doc__       +
                 DefineTest.set_cvs.__doc__             )
        text_lines = string.split(text, '\n')

        config_lines = []
        for line in text_lines:
            config_lines.append('## ' + string.lstrip(line))

        option_lines = []
        option_list  = globalvars.add_options.option_list
        for option in option_list:
            name = option.dest
            val = getattr(globalvars.options, name)
            if isinstance(val, type('')):
                val = quote(val)
            if val is not None:
                option_lines.append( '%s=%s,' % (name, str(val)) )

        opt_text = ''
        if option_lines:
            option_lines[-1] = string.rstrip(option_lines[-1], ',')
            opt_text = string.join(option_lines, '\n')
        
        config_text = string.join(config_lines, '\n')
        config_text += """\nDefineTest(
        %s
        )        
        """%opt_text
        
        conf = open(self.config, 'w')
        conf.write(config_text)
        conf.close()
        return True


    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        self.config = os.path.join(directory, globalvars.pytest_config)
        return '' 

class CompileTest(Test):
    """Launches the compilation of all target tests' compilable files.
    """
    def __init__(self, dispatch, test_id, directory, parent,
                 prog_name, compile_options, arguments, cvs_files, compiler):
        Test.__init__(self, dispatch, test_id, directory, parent,
                      prog_name, compile_options, arguments, cvs_files, compiler)
        self.set_option('results_directory', None)
        self.set_option('log_file', prog_name + ".compile_log")

    ###############################################
    ## Protected Methods
    ###############################################

    def _do_not_run(self):
        if self.global_prog:
            to_compile = self.global_prog.launch_compilation()
            if not to_compile:
                if ( self.global_prog.status() ==
                     CompilableProgram.FAILED   ):
                    self.failed = True
                globalvars.stats[self.classname()].skip()
                self.skipped = True
                return True
        return False

    def _link_n_dir(self):
        if self.global_prog:
            return (self.prog_name, os.path.dirname( self.prog_name ))
        return ( os.path.join(self.directory, self.prog_name),
                 self.directory )

    def _local_postprocessing(self):
        link = False
        directory = None

        link, directory = self._link_n_dir()
        if ( os.path.islink(link)
             and os.path.exists(os.path.join(directory,os.readlink(link))) ):
            vprint("*** Compilation Test PASSED ***", 2)
            return

        #raw_input('will fail')
        self.failed = True
    
    def _failed(self):
        fail_msg = ("*** " + os.path.join( relative_path(self.directory), self.prog_name) +
                    " Compilation Test FAILED")
        fail_msg += (": Compilation log wrote to file " + self._log_file)
        fail_msg += (" ***")
        vprint(fail_msg, 0)

        if self.global_prog and not self.skipped:
            self.global_prog.compilation_failed()        
        Test._failed(self)

    def _succeeded(self):
        if self.global_prog and not self.skipped:
            self.global_prog.compilation_succeeded()
        Test._succeeded(self)

    ###############################################
    ## Public Methods
    ###############################################
    
    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        if global_programs.has_key( prog_name ):
            self.prog_name = global_programs[prog_name].global_prog_name
            self.global_prog = global_programs[prog_name]
        else:
            self.global_prog = False
        self.skipped = False

        if compiler == 'pymake':
            return ("cd %s; pymake %s %s" % (directory, self.prog_name, compile_options))
        raise NotImplementedError("Unknown compiler %s" % compiler)

class DisableTest(Test):
    """Makes a directory unrunable. 

    The disabled test can be restored (restore mode) afterwards.
    """
    def __init__(self, dispatch, test_id, directory, parent):
        Test.__init__(self, dispatch, test_id, directory, parent)
        
        dis_name = complete_test_name(directory)
        if dis_name in globalvars.from_file.disabled:
            vprint('%s already disabled.' % dis_name)
        else:
            globalvars.from_file.disabled.append(dis_name)
            globalvars.from_file.modified = True
            
    def _do_not_run(self):
        return True
    
    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        return ''

## class RemoveTest(CommitTest):
##     """Definitively removes a test (also does cvs management).

##     The removed test will have its files moved to 'REMOVED_test' directory
##     which you shall remove manually.
##     """

##     def _action(self, path):
##         print path
##         move_to  = self._removed_path(path)
##         if os.path.isdir(path):
##             if not os.path.exists(move_to):
##                 os.mkdir(move_to)
##         else:
##             os.system( string.join(['mv', path, move_to]) )
##             cvs_remove( path )

##     def _removed_path(self, path):
##         path_end = path[len(relative_path(self.directory))+1:]
##         rel_rem  = relative_path(self.removed_test)
##         rem_path = os.path.join( rel_rem, path_end )        
##         return rem_path
        
##     def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
##         CommitTest.build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler)

##         (self.previous, test) = plpath.splitprev(directory)
##         print (self.previous, test)
##         self.removed_test = os.path.join(self.previous, 'REMOVED_'+test)

class ResultsTest(Test):
    """Creates a results directory in every target test directories.

    Before a test can be ran, it must defines its expected results. In the
    current context, all outputs (standard output and files) of a test are
    stored by the results mode in a 'Results' directory.

    The run mode will thereafter be able to compare results obtained on a
    new run to the ones kept in the results directory.

    Note that the add mode (config file) must have been invoked on a given
    test before the results can be created.

    DO NOT MODIFY the results directory manually.
    """
    def __init__(self, dispatch, test_id, directory, parent,
                 prog_name, compile_options, arguments, cvs_files, compiler):
        Test.__init__(self, dispatch, test_id, directory, parent,
                      prog_name, compile_options, arguments, cvs_files, compiler)

        self.set_option('log_file', self.prog_name + ".run_log")
        self.new_results = self._results_directory

        self.success_msg = "*** Results were created successfully ***\n"
        self.failure_msg = ( "*** %s test didn't create new results (prior results kept, if any) ***\n"
                             % self.prog_name )

    ###############################################
    ## Protected Methods
    ###############################################

    def _previously_compile(self):
        """_previously_compile()
        -> compilation_passed : [boolean]
        """
        if not globalvars.stats.has_key('CompileTest'):
            globalvars.stats['CompileTest'] = BasicStats('CompileTest')
        globalvars.stats['CompileTest'].new_test()

        compilation = CompileTest( self.dispatch,
                                   self._id+1000, self.directory, None,
                                   self.prog_name, self.compile_options,
                                   self.arguments, self.cvs_files, self.compiler )
        compilation.set_option('signal_completion', False)
        compilation.set_option('quote_cmd_line', False)

        compilation.run()
        self.dispatch.release_shared_files_access(self)

        if compilation.failed:
            self.failed = True
            return False
        return True

    def _do_not_run(self):
        # Prior to the results generation, the program
        if self.compiler is not None:
            compilation_passed = self._previously_compile()
            if not compilation_passed:
                vprint("ResultsTest will be SKIPPED due to compilation errors.", 0)
                return True

        self.bak = ''

        if os.path.exists(self.new_results):
            self.bak = self.new_results + ".BAK"
            os.system("cp -R " + self.new_results + " " + self.bak)
            plpath.keep_only(self.new_results, globalvars.cvs_directory)
        else:
            os.makedirs(self.new_results)

        # Does linking to allow the test to be executed in results path 
        source = os.path.join(self.directory, self.prog_name)                               
        dest = os.path.join(self.new_results, self.prog_name)

        os.symlink(source, dest)

        ## The following is a hack since self.new_results is not known when
        ## build_cmd_line() is called. This must be fixed someday
        self._cmd_line = 'cd %s; %s' % (self.new_results, self._cmd_line)
        ##raw_input(self._cmd_line)

        return False

    def _local_postprocessing(self):
        if self.bak and os.path.exists(self.bak):
            shutil.rmtree( self.bak )
        vprint(self.success_msg, 1)

        ## This must be done here: It is tempting to let the results as they are and
        ## wait the comparison to modify them at the same time than the 'run' results,
        ## but this would not work if the results were generated by one user and
        ## the test ran by another. Indeed, the home path, that could be saved in the
        ## results, would be highly difficult to trace once in another user's account.
        os.path.walk(self.new_results, path_resolve_dir, globalvars.resolving_paths)

    def _failed(self):
        vprint(self.failure_msg, 1)
        Test._failed(self)
        
    ###############################################
    ## Public Methods
    ###############################################
    
    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        return ("%s %s" % (prog_name, arguments))

class RestoreTest(Test):
    """Restores a disabled test.
    """
    def __init__(self, dispatch, test_id, directory, parent):
        Test.__init__(self, dispatch, test_id, directory, parent)
        
        dis_name = complete_test_name(directory)
        if dis_name in globalvars.from_file.disabled:
            globalvars.from_file.disabled.remove(dis_name)
            globalvars.from_file.modified = True            
        else:
            vprint('%s was not disabled.' % dis_name)

    def _do_not_run(self):
        return True
    
    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        return ''
    
class RunTest(ResultsTest):
    """Compares new results to expected ones.
    
    NOTE THAT, before a test can be ran, it must defines its expected results. In the
    current context, all outputs (standard output and files) of a test are
    stored by the results mode in a 'Results' directory.

    The run mode will thereafter be able to compare results obtained on a
    new run to the ones kept in the results directory.

    Note that the add mode (config file) must have been invoked on a given
    test before the results can be created.

    DO NOT MODIFY the results directory manually.
    """
    def __init__(self, dispatch, test_id, directory, parent,
                 prog_name, compile_options, arguments, cvs_files, compiler):
        ResultsTest.__init__(self, dispatch, test_id, directory, parent,
                      prog_name, compile_options, arguments, cvs_files, compiler)

##         self.new_results = os.path.join(self.directory, RESULTS_ON_RUN, self.prog_name)
        self.new_results = string.replace( self._results_directory,
                                           globalvars.results_path,
                                           globalvars.results_on_run )
##         raw_input(self._results_directory)
##         raw_input(self.new_results)
        

        self.success_msg = "*** RunTest PASSED ***\n"
        self.failure_msg = ( "*** RunTest FAILED: logged in %s ***"
                             % self._log_file )
        
    ###############################################
    ## Protected Methods
    ###############################################

    def _do_not_run(self):
        if not os.path.exists( self._results_directory ):
            vprint("Must call pytest results mode for %s before running the test."
                   % os.path.join(self.directory, self.prog_name), 0)
            self.failed = True 
            return True

        return ResultsTest._do_not_run(self)

    def _local_postprocessing(self):
        ##raw_input('Verification')
##        verif = Verification(self._results_directory, self.new_results)

        ## Apply the same path resolving procedure to the new_results
        ## than the one that was applyed to the the expected results
        ## See the ResultsTest._local_postprocessing method for more
        ## comments.
        os.path.walk(self.new_results, path_resolve_dir, globalvars.resolving_paths)
        
        verif = IntelligentDiff(self._results_directory, self.new_results,
                                globalvars.forbidden_directories, vprint)
        diffs = verif.get_differences()
        if diffs == []:
#        if self._successful_verification():
            shutil.rmtree( self.new_results )

            run_results, last = plpath.splitprev(self.new_results) 
            if len(os.listdir(run_results)) == 0:
                shutil.rmtree( run_results )
                
            if self.bak and os.path.exists(self.bak):
                shutil.rmtree( self.bak )
            vprint(self.success_msg, 1)
        else:
            self.failed = True
            map(lambda err: vprint(err, 1), diffs)
        
class SyncTest(Test):
    """Manages to add and commit files relative to a test."""
    
    def __examine(self, p):
        path = relative_path(p)
        if os.path.exists(path):
            vprint('Pytest examines %s' % path, 2)
        return self._action(path)
    
    def __commit_sub_routine(self, root, directory, dirlist):
        remove_forbidden_dirs(dirlist, globalvars.forbidden_directories)
        os.system('rm *~ .*~ *.o *.pyc *.class ./~* #*# core* -f')

        self.__examine(directory)
        
        for name in dirlist:
            relname = os.path.join(directory, name)
            if ( os.path.exists(relname) and
                 not os.path.islink(relname) ):
                self.__examine( relname )

    def _action(self, path):
        cvs_add(path)

    def _do_not_run(self):
        directory_when_called = os.getcwd()
        os.chdir(test_suite_dir(self.directory))

        self.__examine(self.directory)        
        self.__examine(self.config)

        if os.path.exists( self._results_directory ):
            self.__examine(self._results_base)
            os.path.walk( self._results_base, self.__commit_sub_routine,
                          relative_path(self._results_directory)   )
        
        for relf in self.cvs_files:
            rfname = os.path.join(self.directory, relf)
            if os.path.isdir(rfname):
                os.path.walk( rfname, self.__commit_sub_routine,
                              relative_path(self.directory)    )
            else:
                self.__examine(rfname)
            
        cvs_commit( relative_path(self.directory), 'Pytest sync mode commit.' )
        os.chdir(directory_when_called)
        return True

        ##################################################################################
        # The current implementation of the CommitTest do not use parallel hosts.
        # No command are launched on another host, so this method always returns True,
        # meaning that the parallel processing should not be ran.
        return True

    def build_cmd_line(self, directory, prog_name, compile_options, arguments, cvs_files, compiler):
        self.config = os.path.join(directory, globalvars.pytest_config)
        return ''

