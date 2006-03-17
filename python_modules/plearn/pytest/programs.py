import logging, os
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

#######  TMP  #################################################################
#######  TMP  #################################################################
#######  TMP  #################################################################
## def LocalProgram(**overrides):
##     return Program(**overrides)
## def LocalCompilableProgram(**overrides):
##     return Program(**overrides)
## def GlobalProgram(**overrides):
##     return Program(**overrides)
## def GlobalCompilableProgram(**overrides):
##     return Program(**overrides)
#######  TMP  #################################################################
#######  TMP  #################################################################

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
        if os.path.exists(path):
            program_path = os.path.abspath(path)

    assert program_path is not None
    return program_path

def find_local_program(name):
    if os.path.exists( name ) or \
           os.path.exists( name+'.cc' ):
        return os.path.abspath(name)
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

def Singleton(SuperMeta):
    class _Singleton(SuperMeta):
        def __init__(self, *args):
            type.__init__(self, *args)
            self._singletons = {}
    
        def __call__(self, **overrides):
            signature = self._signature(**overrides)
            if not signature in self._singletons:
                overrides['_signature'] = signature
                self._singletons[signature] = type.__call__(self, **overrides)
            return self._singletons[signature]
    return _Singleton

class Program(core.PyTestObject):

    #######  Options  #############################################################

    name = PLOption(None)
    compiler = PLOption(None)
    compile_options = PLOption(None)

    #######  Program instances are singletons  ####################################

    __metaclass__ = Singleton(core.PyTestObject.__metaclass__)
    def _signature(cls, **overrides):
        options = dict([ (opt, None) for opt in cls.class_options() ])
        options.update(overrides)                
        if options['compiler'] is None:
            signature = options['name']
        else:
            if options['compile_options'] == "":                
                options['compile_options'] = None
            signature =\
                "%(name)s__compiler=%(compiler)s__options=%(compile_options)s"%options
        return signature.replace(' ', '_')
    _signature = classmethod(_signature)

    #######  Class Variables  #####################################################

    # Cache to remember which executable already exist
    compiled_programs = {}

    # If True, no compilations done; assume old executables are valid
    compilation_disabled = False

    # Special pymake hacks --- see hackCompileOptions()
    pymake_opt_options = [ 'dbg', 'opt', 'pintel', 'gprof',
                           'safegprof', 'safeopt', 'safeoptdbg', 'checkopt' ]
    pymake_opt_override = "dbg"


    #######  Instance Methods  ####################################################

    def __init__(self, **overrides):
        ## ##### TMP
        ## overrides.pop('log_file_path', None)                    
        ## ##### TMP
        ## if 'compile_options' in overrides and overrides['compile_options'] == "":
        ##     overrides['compile_options'] = None
            
        core.PyTestObject.__init__(self, **overrides)
        assert self.name is not None
        assert self.compiler is not None or self.compile_options is None

        # Methods are called even if messages are not logged
        logging.debug( self.getProgramPath() )

        self.__is_compilable = False
        internal_exec_path = self.getInternalExecPath(overrides.pop('_signature'))
        logging.debug("Internal Exec Path: %s"%internal_exec_path)
        if self.compiler:
            self.__is_compilable = True
            self.__attempted_to_compile = False
            self.__log_file_path = internal_exec_path+'.log'
            if not self.compilation_disabled and os.path.exists(internal_exec_path):
                os.remove(internal_exec_path)
                logging.debug("*** REMOVED %s"%internal_exec_path)
        logging.debug(internal_exec_path)

    def _optionFormat(self, option_pair, indent_level, inner_repr):
        optname, val = option_pair
        if val is None:
            return ""
        return super(Program, self)._optionFormat(option_pair, indent_level, inner_repr)

    def compilationSucceeded(self):
        return os.path.exists(self.getInternalExecPath())

    def compile(self):

        #######  Ensure compilation is needed  #################################
        
        if self.compilationSucceeded():
            logging.debug("Already successfully compiled %s"%self.getInternalExecPath())
            return True

        elif self.__attempted_to_compile:
            logging.debug("Already attempted to compile %s"%self.getInternalExecPath())
            return False

        elif self.compilation_disabled:
            ### NOTE: This could be changed by a 'cp getProgramPath() getInternalExecPath()'
            raise core.PyTestUsageError(
                "Called PyTest with --no-compile option but %s was not previously compiled."
                % self.getInternalExecPath() )
        
        #######  First compilation attempt  ####################################
        
        targetdir, exec_name = os.path.split(self.getInternalExecPath())
        sanity_check, log_fname = os.path.split(self.__log_file_path)
        assert sanity_check == targetdir
                                 
        # Remove outdated log file
        assert not os.path.exists(exec_name)
        if os.path.exists(log_fname):
            os.remove(log_fname)
            
        elif not os.path.exists(targetdir):
            os.makedirs(targetdir)

        # Actual compilation
        moresh.pushd(targetdir)
        logging.debug("In %s"%os.getcwd())
        
        compile_cmd = "%s %s %s -link-target %s >& %s" \
            % (self.compiler, self.compile_options,
               self.getProgramPath(), self.getInternalExecPath(), log_fname)
        
        logging.debug(compile_cmd)
        os.system(compile_cmd)
        moresh.popd()

        # Report success of fail and remember that compilation was attempted
        self.__attempted_to_compile = True
        return os.path.exists(self.getInternalExecPath())

    def getName(self):
        return self.name
        
    def getInternalExecPath(self, candidate=None):
        if hasattr(self, '_internal_exec_path'):
            assert candidate is None
            return self._internal_exec_path

        if candidate == self.name:            
            self._internal_exec_path = self.getProgramPath()
        else:
            self._internal_exec_path =\
                os.path.join(core.pytest_config_path(), "compiled_programs", candidate)
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
                if self.compiler is not None:
                    self.hackCompileOptions()                        
            except core.PyTestUsageError, neither_global:
                raise core.PyTestUsageError("%s %s"%(not_local, neither_global))
        return self._program_path

    def hackCompileOptions(self):
        assert self.compiler
        if self.compile_options is None: self.compile_options = ''
            
        if self.compiler == "pymake" and self.pymake_opt_override!="dbg":
            try:
                actual_options = self.compile_options.split()
                logging.debug("- actual_options: %s"%actual_options)
                if actual_options:
                    for opt_option in self.pymake_opt_options:
                        assert ('-%s'%opt_option) not in actual_options, opt_option

                # No AssertionError raised
                self.compile_options = ' '.join(
                    ['-%s'%self.pymake_opt_override, self.compile_options])

            except AssertionError, opt_option:
                logging.debug("+ pytest.config provided '-%s' prevails"%opt_option)    

    def invoke(self, arguments, logfname):
        if self.isPLearnCommand():            
            # arguments = "--no-version %s"%arguments
            arguments = "--no-version --no-progress %s"%arguments
        command = "%s %s >& %s"%(self.getInternalExecPath(), arguments, logfname)
        logging.debug("In %s: %s"%(os.getcwd(), command))
        return os.system(command)

    def isCompilable(self):
        return self.__is_compilable
        
    def isGlobal(self):
        return self.__is_global

    def isPLearnCommand(self):
        return self.isGlobal() and self.isCompilable()

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
    Program.pymake_opt_override = 'opt'
    plearn_tests = Program(name="plearn_tests", compiler="pymake")
    print plearn_tests
    print Program(name="plearn_tests", compiler="pymake", compile_options='-dbg')
    
    print plearn_tests.compile()
