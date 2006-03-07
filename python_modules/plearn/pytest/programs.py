__version_id__ = "$Id: programs.py 3647 2005-06-23 15:49:51Z dorionc $"

import logging, os, string, types
import plearn.utilities.ppath          as     ppath
import plearn.utilities.toolkit        as     toolkit

from core import *

########################################
##  Helper Functions  ##################
########################################

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

########################################
##  Helper Classes  ####################
########################################
    
class Program(PyTestObject):
    name                  = PLOption(None)

    # Internal variables
    _path                 = None
    _processing_directory = None
        
    def __init__(self, **overrides):
        PyTestObject.__init__(self, **overrides)
        assert self.name is not None

        self._processing_directory = os.getcwd()
        try:
            self.is_global() ## dummy call to ensure override
            self._path = self.get_path()
        except NotImplementedError:
            raise RuntimeError(
                "Can't create a Program object: use one of GlobalProgram, "
                "LocalProgram, GlobalCompilableProgram, LocalCompilableProgram "
                "instead."
                )

    def getName(self):
        return self.name
        
    def get_path(self):
        raise NotImplementedError

    def is_global(self):
        raise NotImplementedError

class GlobalProgram(Program):
    def get_path(self):
        command_path = plcommand(self.name)

        if command_path is None:
            wlines = toolkit.command_output( "which %s"%self.name )
            if len(wlines) != 1:
                if self.name == 'source':
                    command_path = self.name
                else:
                    raise PyTestUsageError("In %s: which %s resulted in those %d lines \n%s"
                                      %(self.classname(), self.name, len(wlines), ''.join(wlines)))
            else:
                path = wlines[0]
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
        path      = os.path.join(os.getcwd(), self.name)                
        if os.path.exists( path ) or \
               os.path.exists( path+'.cc' ):
            return path
        
        raise ValueError(
            "%s: The '%s' program was not found." % (self.classname(), path)
            )
    
    def is_global(self):
        return False

class Compilable(PyTestObject):
    ## This map will be used to ensure that there are no doubled
    ## compilation attempts.
    _compilation_status = {}

    # Options
    compiler        = PLOption('pymake')
    compile_options = PLOption('')
        
    def __init__(self, **overides):
        raise NotImplementedError(
            "Compilable is an abstract class: use one of "
            "GlobalCompilableProgram or LocalCompilableProgram instead."
            )

    def compilationSucceeded(self):
        if Compilable._compilation_status.has_key(self._path):
            return Compilable._compilation_status[self._path]

        ## Internal call: add the status to the map
        status = None
        if not os.path.exists( self._path ):
            status = False

        elif ( os.path.islink( self._path )
               and not os.path.exists( self.path_to_target() ) ):
            status = False

        else: 
            status = True

        assert isinstance(status, type(True))
        Compilable._compilation_status[self._path] = status
        return status
    
    def compile(self):
        if Compilable._compilation_status.has_key(self._path):
            logging.debug("Already compiled %s"%self._path)
            return Compilable._compilation_status[self._path]
        
        directory_when_called = os.getcwd()
        os.chdir( self._processing_directory )

        if not os.path.exists( ppath.pytest_dir ):
            os.makedirs( ppath.pytest_dir )

        ## Remove the symbolic link
        if os.path.islink( self._path ):
            os.remove(self._path)
            
        log_file_name = os.path.join(ppath.pytest_dir,self.name + '.compilation_log')
        compile_cmd   = ( "%s %s %s >& %s"
                          % ( self.compiler, self.compile_options,
                              self._path,     log_file_name         )
                          )
        
        logging.debug(compile_cmd)
        os.system(compile_cmd)
        
        os.chdir( directory_when_called )

        ## This initializes the compilation status
        return self.compilationSucceeded()

    def path_to_target(self):
        raise NotImplementedError
    
class GlobalCompilableProgram(GlobalProgram, Compilable):
    def getName(self):
        return self.name+' --no-version'

    def path_to_target(self):
        link_target = os.readlink( self._path )
        to_target   = os.path.join( os.path.dirname(self._path), link_target)
        return to_target
        
class LocalCompilableProgram(LocalProgram, Compilable): 
    def path_to_target(self):
        link_target  = os.readlink( self._path )
        to_target    = os.path.join( self._processing_directory, link_target)
        return to_target
