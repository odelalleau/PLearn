import os, string, types
import plearn.utilities.plpath         as     plpath
import plearn.utilities.toolkit        as     toolkit

from   plearn.utilities.verbosity      import vprint
from   plearn.utilities.FrozenObject   import FrozenObject

class PyTestUsageError(Exception):  
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg
    
class ProgramDefaults:
    name = None
    __declare_members__ = [ ('name', types.StringType) ]

class Program(FrozenObject):
    def __init__(self, defaults=ProgramDefaults, **overrides):
        if not issubclass(defaults, ProgramDefaults):
            raise ValueError
        FrozenObject.__init__(self, defaults, overrides)

        self.set_attribute( 'processing_directory', os.getcwd() )

        try:
            self.is_global() ## dummy call to ensure override
            self.set_attribute('path', self.get_path())
        except NotImplementedError:
            raise RuntimeError(
                "Can't create a Program object: use one of GlobalProgram, "
                "LocalProgram, GlobalCompilableProgram, LocalCompilableProgram "
                "instead."
                )

    def get_name(self):
        return self.name
        
    def get_path(self):
        raise NotImplementedError

    def is_global(self):
        raise NotImplementedError

class GlobalProgram(Program):
    def get_path(self):
        command_path = plpath.plcommand(self.name)

        if command_path is None:
            path = toolkit.command_output( "which %s"%self.name )[0]
            path = string.rstrip(path, ' \n')
            if os.path.exists(path):
                command_path = os.path.abspath(path)
        
        if command_path is None:
            raise PyTestUsageError(
                "The only GlobalProgram and GlobalCompilableProgram currently "
                "supported must be found in <plearn_branch>/commands/. Program "
                "%s was not." % self.name
                )
        return command_path

    def is_global(self):
        return True


class LocalProgram(Program):
    def get_path(self):
        path    = os.path.join(os.getcwd(), self.name)
        if os.path.exists( path+'.cc' ):
            return path
        
        raise ValueError(
            "%s: The '%s' program was not found." % (self.classname(), path)
            )
    
    def is_global(self):
        return False

class CompilableProgramDefaults(ProgramDefaults):
    compiler        = 'pymake'
    compile_options = ''
    __declare_members__ = ( ProgramDefaults.__declare_members__ +
                            [ ('compiler', types.StringTypes),
                              ('compile_options', types.StringTypes) ] )

class Compilable:
    def __init__(self):
        raise NotImplementedError(
            "Compilable is an abstract class: use one of "
            "GlobalCompilableProgram or LocalCompilableProgram instead."
            )

    def compilation_succeeded(self):
        if not os.path.exists( self.path ):
            return False

        if ( os.path.islink( self.path )
             and not os.path.exists( self.path_to_target() ) ):
                return False

        return True
    
    def compile(self):
        directory_when_called = os.getcwd()
        os.chdir( self.processing_directory )

        if not os.path.exists( plpath.pytest_dir ):
            os.makedirs( plpath.pytest_dir )
            
        log_file_name = os.path.join(plpath.pytest_dir,self.name + '.compilation_log')
        compile_cmd   = ( "%s %s %s >& %s"
                          % ( self.compiler, self.compile_options,
                              self.path,     log_file_name         )
                          )
        
        vprint(compile_cmd, 1)
        os.system(compile_cmd)
        
        os.chdir( directory_when_called )

    def path_to_target(self):
        raise NotImplementedError
    
class GlobalCompilableProgram(GlobalProgram, Compilable):
    def __init__(self, **overrides):
        GlobalProgram.__init__(self, CompilableProgramDefaults, **overrides)

    def get_name(self):
        return self.name+' --no-version'

    def path_to_target(self):
        link_target = os.readlink( self.path )
        to_target   = os.path.join( os.path.dirname(self.path), link_target)
        return to_target
        
class LocalCompilableProgram(LocalProgram, Compilable): 
    def __init__(self, **overrides):
        LocalProgram.__init__(self, CompilableProgramDefaults, **overrides)

    def path_to_target(self):
        link_target  = os.readlink( self.path )
        to_target    = os.path.join( self.processing_directory, link_target)
        return to_target
