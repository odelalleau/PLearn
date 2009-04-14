import logging, os, sys, subprocess
from plearn.utilities import ppath
from plearn.utilities import moresh
from plearn.utilities import toolkit
from plearn.pyplearn.PyPLearnObject import PLOption

# PyTest modules
import core

## Hardcoded branch management:
## 
## @var plbranches: We call plearn branches the PLearn, LisaPLearn and apstatsoft libraries.
##
## The plbranches format is old and is inspired from a removed module
## (plpath). It may well be inappropriate: rethink all this when the
## branches will be moved to a config file.
plbranches = []
for mpath in ["PLEARNDIR", "APSTATSOFTDIR", "LISAPLEARNDIR"]:
    try:
        plbranches.append( ppath.ppath(mpath) )
    except:
        pass

#######  Helper Functions  ####################################################

def find_global_program(name):
    assert name != "source", "Do not use source, run .sh files instead."
    program_path = plcommand(name)

    if program_path is None:
        wlines = toolkit.command_output( "which %s"%name )
        if len(wlines) != 1:
            raise core.PyTestUsageError(
                "The only global programs currently supported are those found by 'which' "
                "or those found in a <plearn_branch>/commands/. Call 'which %s' resulted "
                "in those %d lines \n%s"%(name, len(wlines), ''.join(wlines)) )

        path = wlines[0]
        path = path.rstrip(' \n')
        if sys.platform == 'win32':
            # Need to convert the Cygwin path to the Windows path, otherwise
            # os.path.exists(path) will return False.
            path = toolkit.command_output( "cygpath -w %s" % path )
            path = path[0].rstrip('\n')
        if toolkit.exec_path_exists(path):
            program_path = os.path.abspath(path)

    assert program_path is not None, 'Could not find global program %s' % name
    return program_path

def find_local_program(name):
    name= ppath.expandEnvVariables(name)
    name2, ext= os.path.splitext(name)
    if ext=='.so':
        name= name2
    if os.path.exists( name ) or \
           os.path.exists( name+'.cc' ):
        return os.path.abspath(name)
    logging.debug("Could not find local program %s" % name)
    raise core.PyTestUsageError(
        "The '%s' program was not found in %s."%(name, os.getcwd()))

def plcommand(command_name):
    """The absolute path to the command named I{command_name}.

    @param command_name: The name of a command the user expect to be
    found in the 'commands' directory of one of the plearn branches.
    @type  command_name: String

    @return: A string representing the path to the command. The
    function returns None if the command is not found.
    """
    command_path = None
    for plbranch in plbranches:
        cmd_path = os.path.join(plbranch, 'commands', command_name)
        path     = cmd_path+'.cc'
        if os.path.exists( path ):
            command_path = cmd_path
            break

    return command_path

class Program(core.PyTestObject):

    #######  Options  #############################################################

    name = PLOption(None)
    
    compiler = PLOption(None)

    compile_options = PLOption(None)

    no_plearn_options = PLOption(False,
        doc="PLearn commands usually receive the --no-progess and --no-version options. "
            "If that option is False though, this won't be the case for this Program instance.")

    dependencies = PLOption([], doc="A list of programs on which this one depends.")
    

    #######  Class Variables  #####################################################

    # Default compiler: for programs assumed to be compilable but for which
    # no compiler were provided
    default_compiler = "pymake"

    # Cache to remember which executable already exist
    compiled_programs = {}

    # If True, no compilations done; assume old executables are valid
    compilation_disabled = False

    # Special pymake hack --- see handleCompileOptions()
    cmdline_compile_options = ""


    #######  Instance Methods  ####################################################

    def __init__(self, **overrides):
        core.PyTestObject.__init__(self, **overrides)
        assert self.name is not None

        # Some command line options can modify the default values for those...
        self.handleCompileOptions()

        # Methods are called even if messages are not logged...
        ##  The getProgramPath() method call is required here so as to
        ##  initialize the '_program_path' and '__is_global' members
        logging.debug("\nProgram: "+self.getProgramPath())

        internal_exec_path = self.getInternalExecPath(self._signature())
        logging.debug("Internal Exec Path: %s"%internal_exec_path)
        if self.isCompilable():
            if self.compiler is None:
                self.compiler = Program.default_compiler
            self.__attempted_to_compile = False
            self.__log_file_path = self.getCompilationLogPath()
        logging.debug("Program instance: %s\n"%repr(self))

    def handleCompileOptions(self):        
        assert self.compiler is not None or self.compile_options is None, \
            "One cannot provide compile options without providing a compiler..."
        logging.debug("Creating program using compiler %s with compile_options '%s'."%(self.compiler, self.compile_options))

        if self.cmdline_compile_options and self.compiler == "pymake":
            config_options = []
            if self.compile_options:
                config_options = [ opt.replace("-", "")
                                   for opt in self.compile_options.split() ]
            
            cmdline_options = self.cmdline_compile_options.split(",")

            options = list( set(config_options+cmdline_options) )
            options = " -".join([""]+options).strip()

            logging.debug("Test %s: Using compile options '%s' instead of '%s'...",
                          self.name, self.compile_options, options)
            self.compile_options = options
            
    def _signature(self):
        if self.compiler is None:
            signature = self.name
        else:
            if self.compile_options == "":
                self.compile_options = None
            signature = "%s__compiler_%s__options_%s"%(
                self.name, self.compiler, self.compile_options)
        signature = signature.replace('-', '') # Compile options...
        return signature.replace(' ', '_')

    def _optionFormat(self, option_pair, indent_level, inner_repr):
        optname, val = option_pair
        if val is None:
            return ""
        elif optname=="no_plearn_options" and not val:
            return "" # Don't print the default value
        elif optname=="dependencies" and not val:
            return "" # Don't print the default value
        else:
            return super(Program, self)._optionFormat(option_pair, indent_level, inner_repr)

    def compilationSucceeded(self):
        exec_exists = os.path.exists(self.getInternalExecPath())
        no_need_to_compile = self.compilation_disabled or not self.isCompilable()
        if no_need_to_compile and not exec_exists:
            ### NOTE: This could be changed by a 'cp getProgramPath() getInternalExecPath()'
            raise core.PyTestUsageError(
                "Called PyTest with --no-compile option but %s "
                "was not previously compiled." % self.getInternalExecPath())

        # Account for dependencies
        success = no_need_to_compile or (self.__attempted_to_compile and exec_exists)
        #logging.debug("compilationSucceeded(): %s %s %s %s %s", self.getInternalExecPath(),#self.name,
        #              success, no_need_to_compile, self.__attempted_to_compile, exec_exists)
        for dep in self.dependencies:            
            success = (success and dep.compilationSucceeded())
            #print "+++ DEP compilationSucceeded():", success
        return success

    def compile(self, publish_dirpath=""):
        succeeded = True
        if self.isCompilable():
            # Remove old compile log if any        
            publish_target = os.path.join(publish_dirpath,
                                          os.path.basename(self.__log_file_path))
            if os.path.islink(publish_target) or os.path.isfile(publish_target):
                os.remove(publish_target)
            assert not os.path.exists(publish_target)
            
            # Ensure compilation is needed
            if self.compilationSucceeded():
                logging.debug("Already successfully compiled %s"%self.getInternalExecPath())
                succeeded = True
            
            elif self.__attempted_to_compile:
                logging.debug("Already attempted to compile %s"%self.getInternalExecPath())
                succeeded = False
            
            # First compilation attempt
            else:
                #print "+++ SHORTCUT!!!", self.name
                #succeeded = True 
                succeeded = self.__first_compilation_attempt()               
                #print "+++ FIRST ATTEMPT", self.name, succeeded
            
            # Publish the compile log
            if succeeded and publish_dirpath:
                logging.debug("Publishing the compile log %s"%self.__log_file_path)
                toolkit.symlink(self.__log_file_path,
                                moresh.relative_path(publish_target))

        # Account for dependencies
        #print "+++ Success", self.name, succeeded        
        for dep in self.dependencies:
            succeeded = (succeeded and dep.compile(publish_dirpath))        
            #print "+++ DEPcompile", succeeded
            
        return succeeded
        
    def __first_compilation_attempt(self):

        #######  First compilation attempt  ####################################
        
        targetdir, exec_name = os.path.split(self.getInternalExecPath())
        sanity_check, log_fname = os.path.split(self.__log_file_path)
        assert sanity_check == targetdir

        if sys.platform == 'win32':
            # When there are characters like '=' in a filenme, Windows can get
            # confused if the filename is not quoted.
            log_fname = '"%s"' % log_fname
                                 
        # Remove outdated log file
        assert not os.path.exists(exec_name)
        if os.path.exists(log_fname):
            os.remove(log_fname)
            
        elif not os.path.exists(targetdir):
            os.makedirs(targetdir)

        # Actual compilation
        moresh.pushd(targetdir)
        logging.debug("\nIn %s:"%os.getcwd())
        
        if sys.platform == 'win32':
            # We assume the compiler is pymake.
            if self.compiler != "pymake":
                raise Not_Implemented
            the_compiler = "python " + \
                os.path.join(ppath.ppath('PLEARNDIR'), 'scripts', 'pymake')
        else:
            the_compiler = self.compiler

        compile_options = ""
        if self.compile_options is not None:
            compile_options = self.compile_options

        compile_cmd= self.getCompileCommand(the_compiler, compile_options)

        logging.debug(compile_cmd)
        p= subprocess.Popen(compile_cmd, 
                            shell= True,
                            stdout= open(log_fname, 'w'),
                            stderr= subprocess.STDOUT)
        compile_exit_code= p.wait()
        logging.debug("compile_exit_code <- %d\n"%compile_exit_code)

        moresh.popd()

        # Report success of fail and remember that compilation was attempted
        self.__attempted_to_compile = True
        if compile_exit_code!=0 and os.path.exists(self.getInternalExecPath()):
            os.remove(self.getInternalExecPath())

        # Strip C++ execs
        if self.isCompilable() and self.compilationSucceeded():
            os.system("strip %s"%self.getInternalExecPath())
            
        return compile_exit_code==0

    def getCompileCommand(self, the_compiler, compile_options):
        compile_cmd   = "%s %s %s -link-target %s" \
                        % ( the_compiler, compile_options,
                            self.getProgramPath(),
                            self.getInternalExecPath())
        return compile_cmd


    def getName(self):
        return self.name
        
    def getCompilationLogPath(self):
        return self.getInternalExecPath()+'.log'

    def getInternalExecPath(self, candidate=None):
        if hasattr(self, '_internal_exec_path'):
            assert candidate is None, 'candidate is not None:'+repr(candidate)
            return self._internal_exec_path

        logging.debug("Parsing for internal exec path; candidate=%s"%candidate)
        
        if candidate == self.name:
            self._internal_exec_path = self.getProgramPath()
        else:
            self._internal_exec_path = \
                os.path.join(core.pytest_config_path(), "compiled_programs", candidate)
        if sys.platform == 'win32':
            # Cygwin has trouble with the \ characters.
            self._internal_exec_path = \
                self._internal_exec_path.replace('\\', '/')
            # In addition, we need to add the '.exe' extension as otherwise it
            # may not be able to run it.
            self._internal_exec_path += '.exe'
        return self._internal_exec_path

    def getProgramPath(self):
        if hasattr(self, '_program_path'):
            return self._program_path        

        try:
            self._program_path = find_local_program(self.name)
            self.__is_global = False
        except core.PyTestUsageError, not_local:
            try:
                self._program_path = find_global_program(self.name)
                self.__is_global = True                
            except core.PyTestUsageError, neither_global:
                raise core.PyTestUsageError("%s %s"%(not_local, neither_global))
        return self._program_path

    def invoke(self, arguments, logfname):
        if self.isPLearnCommand() and not self.no_plearn_options:            
            # arguments = "--no-version %s"%arguments
            arguments = "--no-version --no-progress %s"%arguments
        #command = '%s %s >& %s'%(self.getInternalExecPath(), arguments, logfname)
        command = '%s %s' % (self.getInternalExecPath(), arguments)
        # Run the test.
        logging.debug("In %s: %s"%(os.getcwd(), command))
        test_output = toolkit.command_output(command)
        # Save test output into log file.
        logfile = open(logfname, 'w')
        logfile.writelines(test_output)
        logfile.close()
        if sys.platform == "win32":
            # Need to convert Windows line feeds to Unix ones, otherwise the
            # diff will fail when comparing expected and actual run logs.
            toolkit.command_output("dos2unix %s" % logfname)

    def isCompilable(self):
        try:
            return self.__is_compilable
        except AttributeError:
            # Compiler must be provided only for compilable program            
            if self.compiler:
                self.__is_compilable = True

            # Programs which doesn't exist yet are assumed to be created
            # through compilation
            elif not toolkit.exec_path_exists(self.getProgramPath()):
                self.__is_compilable = True

            # PyMake-style
            elif os.path.islink(self.getProgramPath()):
                self.__is_compilable = os.path.exists(self.getProgramPath()+".cc")

            # Otherwise assumed to be non-compilable
            else:
                self.__is_compilable = False

            # It is now cached... 
            return self.__is_compilable

    def areDependenciesCompilable(self):
        compilable = False
        for dep in self.dependencies:
            if dep.isCompilable():
                compilable = True
                break        
        return compilable
    
    def isGlobal(self):
        return self.__is_global

    def isPLearnCommand(self):
        return self.isGlobal() and self.isCompilable()


class PythonSharedLibrary(Program):
    """
    Version of Program for shared libraries used as python extension modules.
    WARNING: the compilation produces an .so file in the same directory as
    the source (instead of e.g. ~/.plearn/pytest/compiled_programs)
    """
    def __init__(self, **overrides):
        Program.__init__(self, **overrides)

    def getProgramPath(self):
        if hasattr(self, '_program_path'):
            return self._program_path        
        pp= super(PythonSharedLibrary, self).getProgramPath()
        ppd, ext= os.path.splitext(pp)
        if ext == '':
            pp+= '.cc'
        self._program_path= pp
        return self._program_path

    def _signature(self):
        return self.getProgramPath()[:-2]+'so'

    def getCompileCommand(self, the_compiler, compile_options):
        """
        WARNING: this replaces the shared object in-place
        """
        compile_cmd   = "%s %s %s" \
                        % ( the_compiler, compile_options,
                            self.getProgramPath())
        return compile_cmd

        
class PythonSharedLibrary2(Program):
    """
    DO NOT USE (unless you fix it).
    Incomplete version of PythonSharedLibrary;
    should produce executable in ~/.plearn/pytest/compiled_programs
    should replace PythonSharedLibrary once fixed
    """

    def __init__(self, **overrides):
        Program.__init__(self, **overrides)
        internal_exec_path= self.getInternalExecPath()
        internal_exec_dir= os.path.dirname(internal_exec_path)
        module_path= self.getPythonModulePath()

        base_exec_dir= internal_exec_dir[:-len(module_path)]
        base_exec_dir= os.path.dirname(base_exec_dir)

        def mkInit(d):
            logging.debug("** "+d)
            try:
                os.mkdir(d)
            except:
                pass
            try:
                os.mknod(os.path.join(d, '__init__.py'))
                logging.debug("\t**! "+d)
            except:
                pass

        to_init= base_exec_dir
        mkInit(to_init)
        subdirs= module_path.split('/')
        for d in subdirs:
            to_init= os.path.join(to_init, d)
            mkInit(to_init)
            
        os.environ['PYTHONPATH']= base_exec_dir+':'+os.getenv('PYTHONPATH','')
        
        logging.debug("PYTHONPATH: "+os.environ['PYTHONPATH']+"\tinternal_exec_path="+internal_exec_path)
        logging.debug("NAME: "+self.name)
        p= ppath.expandEnvVariables(self.name)
        logging.debug("path: "+p)


    def getPythonModulePath(self):
        if hasattr(self, '_python_module_path'):
            return self._python_module_path
        pythonpath= os.getenv('PYTHONPATH','').split(':')
        module_path= None
        common_prefix= None
        name= ppath.expandEnvVariables(self.name)
        for p in pythonpath:
            c= os.path.commonprefix([p, name])
            if c == p:
                logging.debug("Common Prefix Match: "+c+' -- '+repr(name[len(p):].split('/')))
                module_path= name[len(p):]
                while module_path[0] == '/':
                    module_path= module_path[1:]
                while module_path[-1] == '/':
                    module_path= module_path[:-1]
                module_path= os.path.dirname(module_path)
                common_prefix= c
                break
            else:
                logging.debug("NO Common Prefix: "+p+' -- '+name)
        self._python_module_path= module_path
        return module_path

    def _signature(self):
        """
        
        """
        if self.compiler is None:
            signature = self.name
        else:
            name= os.path.basename(self.name)
            if self.compile_options == "":
                self.compile_options = None
            module_path= self.getPythonModulePath()
            signature = "SHARED_OBJS__compiler_%s__options_%s"%(
               self.compiler, self.compile_options)
            signature= os.path.join(signature, module_path, name)
        signature = signature.replace('-', '') # Compile options...
        return signature.replace(' ', '_')

#      def getInternalExecPath(self, candidate=None):
#         if hasattr(self, '_internal_exec_path'):
#             return self._internal_exec_path
#         iep= super(PythonSharedLibrary, self).getInternalExecPath(self, candidate)
#         if iep.endswith('.so'):
#             return iep
#         self._internal_exec_path= iep+'.so'
#         return self._internal_exec_path



         
#         if hasattr(self, '_internal_exec_path'):
#             assert candidate is None
#             return self._internal_exec_path

#         logging.debug("Parsing for internal exec path; candidate=%s"%candidate)

#         if candidate == self.name:
#             self._internal_exec_path = self.getProgramPath()
#         else:
#             self._internal_exec_path = \
#                 os.path.join(core.pytest_config_path(), "compiled_programs", candidate)
#         if sys.platform == 'win32':
#             # Cygwin has trouble with the \ characters.
#             self._internal_exec_path = \
#                 self._internal_exec_path.replace('\\', '/')
#             # In addition, we need to add the '.exe' extension as otherwise it
#             # may not be able to run it.
#             self._internal_exec_path += '.exe'
#         return self._internal_exec_path

    def getProgramPath(self):
        if hasattr(self, '_program_path'):
            return self._program_path        
        pp= super(PythonSharedLibrary, self).getProgramPath()
        ppd, ext= os.path.splitext(pp)
        if ext == '':
            pp+= '.cc'
        self._program_path= pp
        return self._program_path
         
#         if hasattr(self, '_program_path'):
#             return self._program_path        

#         try:
#             self._program_path = find_local_program(self.name)
#             self.__is_global = False
#         except core.PyTestUsageError, not_local:
#             try:
#                 self._program_path = find_global_program(self.name)
#                 self.__is_global = True                
#             except core.PyTestUsageError, neither_global:
#                 raise core.PyTestUsageError("%s %s"%(not_local, neither_global))
#         return self._program_path



if __name__ == "__main__":
    # In Python2.4
    # logging.basicConfig(level=logging.DEBUG)
    hdlr = logging.StreamHandler()
    hdlr.setFormatter( logging.Formatter("%(message)s") )
    logging.root.addHandler(hdlr)
    logging.root.setLevel(logging.DEBUG)

    os.chdir( ppath.ppath("PLEARNDIR:test_suite/SimpleNet") )
    print Program(name="python")
    print Program(name="plearn", compiler="pymake")
    print Program(name="simplenet", compiler="pymake", compile_options="-safeopt")
    
    print "\nShould not go through __init__:\n"
    print Program(name="python")
    print Program(name="plearn", compiler="pymake")
    print Program(name="simplenet", compiler="pymake", compile_options="-safeopt")

    print "\nOpt hack:"
    Program.cmdline_compile_options = 'opt'
    plearn_tests = Program(name="plearn_tests", compiler="pymake")
    print plearn_tests
    print Program(name="plearn_tests", compiler="pymake", compile_options='-dbg')
    
    print plearn_tests.compile()
