#!/usr/bin/env python

# pymake
# Copyright (C) 2001 Pascal Vincent
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

import os, sys, re, string, glob, socket, time, select, shutil, fnmatch, math
import plearn.utilities.toolkit as toolkit
from stat import *
from types import *
from popen2 import *

try:
    from random import shuffle
except ImportError:
    import whrandom
    def shuffle(list):
        l = len(list)
        for i in range(0,l-1):
            j = whrandom.randint(i+1,l-1)
            list[i], list[j] = list[j], list[i]

#####  Helper function definitions  ###########################################
# (plus some global variables)

def printversion():
    print "pymake 2.0 [ (C) 2001, Pascal Vincent. This is free software distributed under a BSD type license. Report problems to vincentp@iro.umontreal.ca ]"

###  Usage

def printshortusage():
    print 'Usage: pymake [options] <list of targets, files or directories>'
    print 'Where targets are .cc file names or base names or directories'
    print 'For long help: pymake -help'
def printusage():
    print 'Usage: pymake [options] <list of targets, files or directories>'
    print 'Where targets are .cc file names or base names or directories'
    print 'And options are a combination of the following:'
    print
    print 'Options from the configuration file:'
    if len(options_choices)==0:
        print
        print 'Do "pymake -getoptions <list of targets, files or directories>"'
        print 'to print target-specific options read from the configuration file.'
    else:
        for choice in options_choices:
            print ' * One of ' + string.join(map(lambda s: '-'+s, choice),', ') + ' (default is -' + choice[0] + ') where:'
            for item in choice:
                print '   -'+item + ': ' + pymake_options_defs[item].description
    print
    print 'Options from pymake:'
    print """
Options specifying the type of compiled file to produce:
  -dll: create a dll instead of an executable file.
        It probably works ONLY on Windows with MinGW installed.
  -dllno-cygwin: create a dll instead of an executable file, with no
                 dependence to the Cygwin dll. It probably works
                 only on Windows with Cygwin installed.
                 Note: SAS does not seem to like the -g option in
                 g++, thus one should use -opt if the dll is meant
                 to be used in SAS.
  -m32:  force compilation in 32bits mode.
         Works on AMD64 (linux-x86_64) and linux-i386 machines, probably also
         on ia64. Allows to compile on .pymake/linux-x86_64.hosts,
         linux-ia64.hosts and linux-i386.hosts.
  -so:   On linux this will create a shared object (libxxx.so) instead of an executable file.
         On Mac OS X darwin this will create a libxxx.dylib (using the -dylib option
         to produce a mach-o shared library that has file type MH_DYLIB)
  -pyso: Similar to -so, but for python extension modules
         This will create a .so (both on linux and Mac OS X darwin) but without
         preprending 'lib' to the .so name.
         On Mac OS X darwin it uses options -bundle -flat_namespace options
         to produce a mach-o bundle that has file type MH_BUNDLE         
  -static: produce a statically linked executable.

Options that will not affect the final compiled file:
  -dbi=<dbi_mode>: use the DBI interface instead of launching compilation
                   directly on remote hosts.
                   <dbi_mode> can be one of Bqtools, Condor, or Cluster.
  -force: force recompilation of all necessary files,
          even if they are up to date.
  -link: force relinking of the target, even if it is up to date.
  -local[=nb proc]: do not use parallel compilation, even if there is a
          .pymake/<platform>.hosts file. Will compile with nb_proc
          (default is 1) process on the local computer
  -local_ofiles: use parallel compilation, but copy all .o files
                 to /tmp/.pymake/local_ofiles/... prior to linking;
                 target is created locally and then copied to its
                 final destination.
                 N.B. you can set the local_ofiles_base_path global
                 variable in your config file to use another path
                 than /tmp/.pymake/local_ofiles/.
  -l32_64: when compiling with option '-pyso', add '-32' or '64' at the end
           of the '.so' symbolic link (.so -> .so-32 or .so-64)
  -ssh: run compilation commands on remote hosts with ssh instead of rsh
        (default).
  -symlinkobjs: at link time, will create links to the used object files
                (of the form 1.o, 2.o, etc) in order to obtain a smaller link
                command line.
  -tmp[=tmp_dir]: compile all objects into a single directory (default is
                  /tmp/OBJS). This option currently implies '-local'.
                  N.B. this option is not related to -local_ofiles.
  -v[...]: specify the verbosity level, '-vvv' is equivalent to '-v3'.
           1 -> get *** Running ... / ++++ Computing
           2 -> get Launched / Finished / Still waiting (default)
           3 -> get lauched command
           4 -> get extra information

There exist special options that will not compile or link the target, but
instead perform various operations. These special options are:
  -clean: remove all OBJS subrdirectories recursively in all given directories.
  -checkobj: report all source files that don't have *any* correponding .o in
             OBJS. This processes recursively from given directories and is
             typically called after a pymake -clean, and a pymake, to get the
             list of files that failed to compile.
  -dependency: information regarding the dependency graph will be produced:
      pymake -dependency target
          will generate the full dependency graph of target in targetname.dot
          and targetname.ps files.
      pymake -dependency target dependency
          will look for dependency (which must be a filename.cc or filename.h
          without full path or a library_name) and print out the first path it
          finds in the dependency graph that links target and dependency.
  -dependency_include: like -dependency execpt that this one list include
              dependency. i.e. Dependency that make file to be recompiled.
              -dependency list dependency that make file to be included in 
              the executable.
  -dist: extract all the sources necessary to compile the target, in a
         directory called <target>.dist, and create there a Makefile that is
         able to compile and link the target.
  -help: display this help message.
  -getoptions: print the specific options for the target
  -vcproj: a Visual Studio project file (.vcproj) for the target will be
           created.
  -o filename: the name of the output file (a symlink to the real file)
  -link-target filename: the name of the real output file (not a symlink as -o)
  
The configuration file 'config' for pymake is searched for
first in the .pymake subdirectory of the current directory, then similarly
in the .pymake subdirectory of parent directories of the current directory,
and as last resort in the .pymake subdirectory of your homedir. Also, if
there is a pymake.config.model in the directory from which a .pymake/config
is searched, and the pymake.config.model is more recent than the
.pymake/config (or the .pymake/config does not exist) then that 'model'
will be copied to the .pymake/config file.

In adition to the 'config' file, your .pymake directories can contain files
that list machines (one per line) for launching parallel
compilations. Those files are to be called <platform>.hosts, where
<platform> indicates the architecture and system of the machines it
lists. A good place for this file is in the .pymake of your home directory,
as these are likely to be comon to all projects. As an alternative, if no
*.hosts file are present, pymake can also understand the syntax for host
lists used by distcc, and will search for it the same way distcc does
(in $DISTCC_HOSTS, or $DISTCC_DIR/hosts, or ~/.distcc/hosts, or
/etc/distcc/hosts).

Note that you can easily override the compileroptions (defined in your
config file) for a particular <target>.cc file.  Just create a
.<target>.override_compile_options file (along your <target>.cc file) in
which you put a python dictionary defining what options to override and
how.
Ex: If a non time-critical file takes too long to compile in optimized mode
(or bugs), you can override its optimized flags by writing the following in
a corresponding .<target>.override_compile_options file:
{ 'opt': '-Wall -g',
  'opt_boundcheck': '-Wall -g -DBOUNDCHECK'  }


The environment variable PYMAKE_OPTIONS is prepended to the command line 
options. You can define your default options there if they do not conflict
with the ones from the command line.
The environment variable PYMAKE_OBJS_DIR can be set to change the OBJS dir to the value of this variable. This is usefull if you don't want to backup the OBJS dir.
"""


###  OS and filesystem

def abspath(mypath):
    """returns the absolute path of the file, with a hack to remove the leading /export/ that causes problems at DIRO"""
    p = convertWinToLinux(os.path.abspath(mypath))    # python 1.5 does not have abspath
    rmprefix = '/export/'
    lenprefix = len(rmprefix)
    if len(p)>lenprefix:
        if p[0:lenprefix]==rmprefix :
            p = '/'+p[lenprefix:]
    return p

def convertWinToLinux(path):
    """in windows, replace all directory separator \\ by / (for use in MinGW)"""
    if platform=='win32':
        return string.replace(path,"\\","/")
    else:
        return path

def cygwin_to_win_paths(paths):
    """Return the list of Windows paths corresponding to the colon-separated
       list provided as argument"""
    path_list = paths.split(':')
    test_cygpath = toolkit.command_output('which cygpath')
    if len(test_cygpath) != 1 or len(test_cygpath[0]) < 2 or \
       (test_cygpath[0][0] != '/' and test_cygpath[0][1] != ':'):
        return path_list
    result = []
    for path in path_list:
        result.append( \
            toolkit.command_output('cygpath -w %s' % path)[0].rstrip('\n'))
    return result

def get_platform():
    pymake_osarch = os.getenv('PYMAKE_OSARCH')
    if pymake_osarch:
        return pymake_osarch
    platform = sys.platform
    if platform=='linux2':
        linux_type = os.uname()[4]
        if linux_type == 'ppc':
            platform = 'linux-ppc'
        elif linux_type =='x86_64':
            platform = 'linux-x86_64'
        else:
            platform = 'linux-i386'
    return platform

def get_homedir():
    homedir = os.environ['HOME']
    if platform=='win32':
        homedir = 'R:/'
    return homedir

def get_hostname():
    myhostname = socket.gethostname()
    pos = string.find(myhostname,'.')
    if pos>=0:
        myhostname = myhostname[0:pos]
    return myhostname

def get_OBJS_dir():
    dir=os.getenv("PYMAKE_OBJS_DIR")
    if not dir:
        dir="OBJS"
    return dir

def join(*path_list):
    """simply call os.path.join and convertWinToLinux"""
    return convertWinToLinux(os.path.join(*path_list))

def lsdirs(basedir):
    """returns the recursive list of all subdirectories of the given directory,
    excluding OBJS, os.getenv("PYMAKE_OBJS_DIR") and CVS directories and those whose name starts with a dot.
    The first element of the returned list is the basedir"""
    if not os.path.isdir(basedir):
        return []
    dirs = [ basedir ]
    for dname in os.listdir(basedir):
        if dname!='CVS' and dname!='OBJS' and dname!=get_OBJS_dir() and dname[0]!='.':
            dname = join(basedir,dname)
            if os.path.isdir(dname):
                dirs.append(dname)
                dirs.extend(lsdirs(dname))
    return dirs

## hack for compiling in /tmp and others users can overwrite the file
def mychmod(path,mode):
    #print path
    try:
        os.chmod(path,mode)
    except: pass
    (head, tail) = os.path.split(path)
    if tail:
        mychmod(head,mode)


###  Lists

def appendunique(l, l2):
    """appends the elements of list l2 to list l, but only those that are not already in l"""
    for e in l2:
        if e not in l:
            l.append(e)

def unique(l):
    """returns the elements of l but without duplicates"""
    lfilt = []
    for e in l:
        if e not in lfilt:
            lfilt.append(e)
    return lfilt


###  This is used for buffering mtime calls
mtime_map = {}

def forget_mtime(filepath):
    try: del mtime_map[filepath]
    except: pass

def mtime(filepath):
    "a buffered os.path.getmtime"
    "returns 0 if the file does not exist"
    if not mtime_map.has_key(filepath):
        if os.path.exists(filepath):
            mtime_map[filepath] = os.path.getmtime(filepath)
        else:
            mtime_map[filepath] = 0
    return mtime_map[filepath]

def local_filepath(filepath):
    """
    gives the local version of a filepath
    i.e. appends local_ofiles_base_path to filepath
    """
    return os.path.normpath(local_ofiles_base_path + '/' + filepath)

def copyfile_verbose(src, dst):
    if verbose>=3:
        print ("[ COPYING\t" + src + "\n  -->\t\t" + dst + " ]")
    shutil.copy2(src, dst)

def mkdirs_public(path):
    """
    Creates all non-existing directories in path
    and sets rights for full access by anybody
    """
    if os.path.exists(path): return
    dir= os.path.dirname(path)
    mkdirs_public(dir) #create parent if needed
    os.mkdir(path)
    os.chmod(path, S_IRWXU | S_IRWXG | S_IRWXO)
    
def copy_ofile_locally(f):
    lf= local_filepath(f)
    ldir= os.path.dirname(lf)
    mkdirs_public(ldir)
    copyfile_verbose(f, lf)

def get_ofiles_to_copy(executables_to_link):
    if not local_ofiles:
        return []
    files_to_copy= []
    for e in executables_to_link: 
        for f in e.get_object_files_to_link():
            forget_mtime(f)
            lf= local_filepath(f)
            forget_mtime(lf)
            if mtime(f) > mtime(lf):
                files_to_copy+= [f]
    return files_to_copy

def copy_ofiles_locally(executables_to_link):
    if verbose > 1:
        print '++++ Copying remaining ofiles locally for ', string.join(map(lambda x: x.filebase, executables_to_link))
    files_to_copy= get_ofiles_to_copy(executables_to_link)
    for f in files_to_copy:
        copy_ofile_locally(f)

###  Processing of configuration files

def get_config_path(target):
    if os.path.isdir(target):
        target_path = os.path.abspath(target)
    else:
        target_path = os.path.dirname(os.path.abspath(target))

    return locateconfigfile('config',target_path,default_config_text)


default_config_text = r"""
# List of directories in which to look for .h includes and corresponding
# .cc files to compile and link with your program (no need to include
# the current directory, it is implicit)
#sourcedirs = []

# directories other than those in sourcedirs, that will be added as list
# of includes (-I...) to all compilations
#mandatory_includedirs = []

# List of all C preprocessor variables possibly initialized at compilation
# time, and that are not defined or redefined in the code
#cpp_variables = [ 'BOUNDCHECK' ]

# Add available external libraries In the order in which the linkeroptions
# must appear on the linker command line (typically most basic libraries
# last) If you do not give any specific triggers, any included .h file
# found in the specified includedirs will trigger the library Triggers can
# be a list of includes that will trigger the use of the library, and they
# can have wildcards (such as ['GreatLibInc/*.h','Magick*.h'] for instance)

# optionalLibrary( name = 'lapack',
#                 triggers = 'Lapackincl/*.h',
#                 linkeroptions = '-llapack' )


# What linker options to put always after those from the optional libraries
#linkeroptions_tail = '-lstdc++ -lm'

# List of lists of mutually exclusive pymake options.
# First option that appears in each group is the default, and is assumed
# if you don't specify any option from that group
#options_choices = [
#  [ 'g++', 'CC'],
#  [ 'dbg', 'opt']
#]


# Description of options, and associated settings name and description
# are mandatory (description will appear in usage display of pymake)
# you can then specify any one or more of compiler, compileroptions,
# cpp_definitions, linker, linkeroptions

#pymakeOption( name = 'g++',
#              description = 'use g++ compiler and linker',
#              compiler = 'g++',
#              compileroptions = '-pedantic-errors',
#              linker = 'g++' )

#pymakeOption( name = 'CC',
#              description = 'use CC compiler and linker',
#              compiler = 'CC',
#              linker = 'CC' )

#pymakeOption( name = 'dbg',
#              description = 'debug mode (defines BOUNDCHECK)',
#              compileroptions = '-Wall -g',
#              cpp_definitions = [ 'DBOUNDCHECK' ] )

#pymakeOption( name = 'opt',
#              description = 'optimized mode',
#              compileroptions = '-Wall -O9 -funroll-loops -finline -fomit-frame-pointer -fstrength-reduce -ffast-math -fPIC' )
"""

def locateconfigfile(file,configdir,config_text=''):

    # first look in the current directory then look in its parents
    directory = convertWinToLinux(configdir)

    while os.path.isdir(directory) and os.access(directory,os.W_OK) and directory!='/':

        fpath = join(directory,'.pymake',file);

        modelpath = join(directory,'pymake.'+file+'.model')
        if os.path.isfile(modelpath) and mtime(modelpath)>mtime(fpath):
            print '*****************************************************'
            if os.path.isfile(fpath):
                print '* FOUND A MORE RECENT MODEL FILE: ' + modelpath + ' ***'
                print '* SAVING PREVIOUS CONFIG FILE AS: ' + fpath+'.bak '
                # Remove existing backup file if necessary: this is needed
                # under Windows, where an exception is raised otherwise.
                backup_file = fpath + '.bak'
                if os.path.isfile(backup_file):
                    os.remove(backup_file)
                os.rename(fpath, backup_file)
            print '* COPYING MODEL FILE ' + modelpath + ' TO ' + fpath
            print '* PLEASE ADAPT THE CONFIGURATION TO YOUR NEEDS'
            print '*****************************************************'
            try: os.makedirs(join(directory,'.pymake'))
            except: pass
            shutil.copyfile(modelpath,fpath)

        if os.path.isfile(fpath):
            return fpath

        directory = abspath(join(directory,'..'))

    # nothing was found in current directory or its parents, let's look in the homedir 
    fpath = join(homedir,'.pymake',file)
    if os.path.isfile(fpath):
        return fpath
    else:
        if config_text:
            print '*****************************************************'
            print '* COULD NOT LOCATE ANY PYMAKE '+file+' CONFIGURATION FILE.'
            print '* CREATING A DEFAULT CONFIG FILE IN ' + fpath
            print '* PLEASE ADAPT THE CONFIGURATION TO YOUR NEEDS'
            print '*****************************************************'
            try: os.makedirs(join(homedir,'.pymake'))
            except: pass
            f = open(fpath,'w')
            f.write(config_text)
            f.close()
            return fpath

    return ''


# For option definitions in config file
pymake_options_defs = {}

class PymakeOption:
    def __init__(self, name, description, compiler, compileroptions, cpp_definitions, linker, linkeroptions, in_output_dirname):
        self.name = name
        self.description = description
        self.compiler = compiler
        self.compileroptions = compileroptions
        self.cpp_definitions = cpp_definitions
        self.linker = linker
        self.linkeroptions = linkeroptions
        self.in_output_dirname = in_output_dirname

# adds a possible option to the pymake_options_defs
def pymakeOption( name, description, compiler='', compileroptions='', cpp_definitions=[], linker='', linkeroptions='', in_output_dirname=True):
    pymake_options_defs[name] = PymakeOption(name, description, compiler, compileroptions, cpp_definitions, linker, linkeroptions, in_output_dirname)


# adds a possible option to the pymake_options_defs that modify only the linker step
def pymakeLinkOption( name, description, triggers='', linker='', linkeroptions='' ):
    pymakeOption(name=name, description=description, linker=linker, linkeroptions=linkeroptions, in_output_dirname=False)
    
optional_libraries_defs = []

# It is now possible to use lists in the linker and compiler options of an optional library.
# These lists can contain strings and / or functions (that must return a string), and will
# be concatenated when the library is triggered (which means the function will not be called
# if the library is not triggered).
class OptionalLibrary:
    def __init__(self, name, includedirs, triggers, linkeroptions, compileroptions, compile_as_source):
        self.name = name
        self.includedirs = includedirs
        self.triggers = triggers
        self.linkeroptions = linkeroptions
        self.compileroptions = compileroptions
        self.compile_as_source = compile_as_source
        # print 'OptionalLibrary '+name+' '+repr(self.triggers)

    def is_triggered_by(self, include):
        """returns true if this particular include command is supposed to trigger this library"""
        known_trigger = False
        for trigger in self.triggers:
            if fnmatch.fnmatch(include,trigger):
                # print 'OPTIONAL LIBRARY '+self.name+': INCLUDE ' + include  + ' TRIGGERED BY TRIGGER ' + trigger
                known_trigger = True
                break

        # If no trigger is specified, look up if include file is to be found in the includedirs
        if len(self.triggers)==0 and include[0]!='/': # look for it in includedirs
            for incldir in self.includedirs:
                fullpath = join(incldir,include)
                if os.path.isfile(fullpath):
                    # print 'file exists!: ' + fullpath
                    # print 'INCLUDE ' + include  + ' TRIGGERED BY INCDIR ' + incldir
                    return fullpath

        return known_trigger

    # Replaces lists and potential functions in the compiler and linker options
    # by a single string.
    # This method is called as soon as the optional library is triggered.
    def convert_all_options_to_strings(self):

        # Function that takes either a string or a function returning a string,
        # and that returns the corresponding string.
        def convert_to_str(str_or_func):
            import types
            if type(str_or_func) is str:
                return str_or_func
            elif type(str_or_func) is types.FunctionType:
                return str_or_func()
            else:
                print 'In pymake - A compiler option must be either a string or a function'
                sys.exit(100)

        # Function that takes either a list of options (strings and / or functions)
        # or a single string, and return a single string.
        def convert_options_to_string(options):
            if type(options) is str:
                return options
            else: # It must be a list.
                return string.join( map(convert_to_str, options), '' )


        self.compileroptions = convert_options_to_string( self.compileroptions )
        self.linkeroptions   = convert_options_to_string( self.linkeroptions   )


# adds a library to the optional_libraries_defs
def optionalLibrary( name, linkeroptions, includedirs=[], triggers='', compileroptions='', compile_as_source=False ):
    if type(includedirs) != ListType:
        includedirs = [ includedirs ]
    if type(triggers) != ListType:
        if triggers=='':
            triggers=[]
        else:
            triggers = [ triggers ]
    optional_libraries_defs.append( OptionalLibrary(name, includedirs, triggers, linkeroptions, compileroptions, compile_as_source) )


###  Processing of options defined in the config files

# fill the list of options from the optionargs, adding necessary default options (if no option of a group choice was specified)
def getOptions(options_choices,optionargs):
    options = []

    for choice in options_choices:
        nmatches = 0
        for item in choice:
            if item in optionargs:
                while item in optionargs: optionargs.remove(item)
                if nmatches<1:
                    options.append(item)
                    nmatches = nmatches+1
                else:
                    print 'Error: options ' + item + ' and ' + options[-1] + ' are mutually exclusive. Aborting.'
                    sys.exit(100)
        if nmatches<1:
            options.append(choice[0])
    if optionargs: # there are remaining optionargs
        print 'Invalid options: ' + string.join(map(lambda s: '-'+s, optionargs))
        printshortusage()
        sys.exit(100)

    return options

###  Processing of the list of hosts used for compilation
# (will soon be removed in favor of an external batch launcher)... NOT

def _process_distcc_hosts(host_list):
    """Processes a distcc-style string describing hosts used for
    compilation. Returns a list of hosts that can be used by pymake."""
    hosts = []

    # Remove trailing \n
    if host_list[-1] == '\n':
        host_list = host_list[:-1]

    # Split into lines, filter comments.
    for l in host_list.split('\n'):
        # Filter hash comments
        i = l.find('#')
        if i != -1:
            l = l[:i]

        for token in l.split():
            # Each token should be of the form @host or @host/num
            # Example: @odin/2
            if token[0] != '@':
                print 'Does not know how to handle distcc host', token
                print 'Only @host specification (with optional /max at the end) is supported presently.'

            i = token.find('/')
            if i == -1:
                num = 1
                host = token[1:]
            else:
                num = int(token[i+1:])
                host = token[1:i]

            # Add num times the host to the list of hosts for compilation.
            hosts.extend([host] * num)

    # For distcc, localhost is given a lesser load because it is busy doing
    # the preprocessing. This does not hold for pymake, so add some more localhost
    # back.
    hosts.extend(['localhost'] * 2)

    return hosts


def get_distcc_hosts():
    """Searches in the locations distcc uses to store list of machines used
    for parallel compilation. Returns a list of machines usable by pymake,
    or None if no distcc configuration was found."""

    # First try $DISTCC_HOSTS environment variable.
    contents = os.getenv('DISTCC_HOSTS')
    if contents is not None:
        return _process_distcc_hosts(contents)

    # Try $DISTCC_DIR/hosts file.
    distcc_dir = os.getenv('DISTCC_DIR')
    if distcc_dir is not None:
        try:
            f = open(os.path.join(distcc_dir, 'hosts'), 'rt')
            contents = f.read()
            f.close()
        except IOError:
            # Move on to next file location...
            pass
        else:
            return _process_distcc_hosts(contents)

    # Try ~/.distcc/hosts
    try:
        f = open(os.path.join(os.path.expanduser('~'), '.distcc', 'hosts'),
                 'rt')
        contents = f.read()
        f.close()
    except IOError:
        # Move on to next file location...
        pass
    else:
        return _process_distcc_hosts(contents)

    # Finally, try /etc/distcc/hosts file.
    try:
        f = open('/etc/distcc/hosts', 'rt')
        contents = f.read()
        f.close()
    except IOError, e:
        # Not found anywhere. Give up.
        return None
    else:
        return _process_distcc_hosts(contents)

def process_hostspath_list(hostspath_list, default_nice_value, local_hostname):
    list_of_hosts = []
    nice_values = {} # dictionary containing hostname:nice_value
    for hostspath in hostspath_list:
        f = open(hostspath,'r')
        for line in f:
            nice_value = default_nice_value
            line = line.strip()

            # Remove comments
            i = line.find('#')
            if i != -1:
                line = line[:i].strip()

            # Get nice value to use (a number, preceded by a space)
            i = line.find(' ')
            if i != -1:
                nice_value = int(line[i+1:])
                line = line[:i]

            # Get number of processors to use (syntax: host/n_proc)
            i = line.find('/')
            if i == -1:
                host = line
                n_proc = 1
            else:
                host = line[:i]
                n_proc = int(line[i+1:])

            # Remove empty strings
            if host:
                list_of_hosts.extend([host] * n_proc)
                nice_values[host] = nice_value

        f.close()

    if 'localhost' not in list_of_hosts and local_hostname not in list_of_hosts:
        list_of_hosts.extend(['localhost'] * nprocesses_on_localhost)

    return (list_of_hosts, nice_values)


def get_list_of_hosts():
    nice_values = {}
    distcc_list_of_hosts = get_distcc_hosts()

    if force_32bits:
        hostspath_list = [locateconfigfile('linux-i386.hosts', homedir),
                          locateconfigfile('linux-x86_64.hosts', homedir),
                          locateconfigfile('linux-ia64.hosts', homedir)]
    else:
        hostspath_list = [locateconfigfile(platform+'.hosts',os.getcwd())]

    # Remove empty strings
    hostspath_list = [ h for h in hostspath_list if h ]

    if hostspath_list:
        if distcc_list_of_hosts is not None:
            print '*** Overriding distcc settings. (Remove pymake *.hosts file to use distcc settings.)'
        (list_of_hosts, nice_values) = process_hostspath_list(hostspath_list, default_nice_value,myhostname)
        print '*** Parallel compilation using',len(list_of_hosts),'process from file(s): ' + string.join( hostspath_list, ', ' )
    elif distcc_list_of_hosts is not None:
        print '*** Parallel compilation using distcc list of hosts (%d)' % len(distcc_list_of_hosts)
        list_of_hosts = distcc_list_of_hosts
    else:
        list_of_hosts = nprocesses_on_localhost * ['localhost']
        print '*** No hosts file found: no parallel compilation, using localhost'
        print '    (create a '+platform + '.hosts file in your .pymake directory to list hosts for parallel compilation.)'

    # randomize order of list_of_hosts
    shuffle(list_of_hosts)
    return (list_of_hosts, nice_values)


###  For text processing of the source files
def remove_escaped_endlines(text):
    escaped_endline = re.compile(r'\\(\r\n|\n|\r)')
    return escaped_endline.sub(' ', text)

def remove_comments(text):
    result = ''
    prevpos = 0
    while 1:
        pos_c = string.find(text, '/*', prevpos)
        pos_cc = string.find(text, '//', prevpos)
        if pos_c<0 and pos_cc<0:
            # There is no comment left
            result = result + text[prevpos:]
            break
        elif pos_cc>=0 and (pos_c<0 or pos_cc<pos_c):
            # The next comment is C++ style
            result = result + text[prevpos:pos_cc] + '\n'
            prevpos = string.find(text, '\n', pos_cc+2)
            if prevpos<0:
                break
            else:
                prevpos += 1
        elif pos_c>=0 and (pos_cc<0 or pos_c<pos_cc):
            # The next comment is C style
            result = result + text[prevpos:pos_c]
            prevpos = string.find(text, '*/', pos_c+2)
            if prevpos<0:
                break
            else:
                prevpos += 2
    return result

def get_cpp_code(text):
    includes_and_ifs = re.compile(r'^\s*#[ \t]*((?:include|if|elif|else|endif|include).*)', re.M)
    return '\n'.join( includes_and_ifs.findall(text) )

### Compile-time definition of C preprocessor variables

# Takes a list of defined options and of C preprocessor variables
# that should be defined at compile time (passing -DOPTION... to the
# compiler), and return a list of undefined variables, a list of defined
# variables, and a dict of variables with their values.
def get_cpp_definitions(cpp_definitions, options, cpp_variables):
    # By default, all variables are undefined
    undefined_cpp_vars = cpp_variables[:]
    defined_cpp_vars = []
    cpp_vars_values = {}

    for definition in cpp_definitions:
        process_cpp_definition( definition, undefined_cpp_vars,
                                defined_cpp_vars, cpp_vars_values )

    for opt in options:
        optdef = pymake_options_defs[opt]
        for definition in optdef.cpp_definitions:
            process_cpp_definition( definition, undefined_cpp_vars,
                                    defined_cpp_vars, cpp_vars_values )

    return undefined_cpp_vars, defined_cpp_vars, cpp_vars_values

# Process one definition
def process_cpp_definition(cpp_definition,
                           undefined_cpp_vars,
                           defined_cpp_vars,
                           cpp_vars_values):
    name_value = cpp_definition.split("=",1)
    # name_value[0] contains the variable name
    var_name = name_value[0]

    # mark this variable as defined
    defined_cpp_vars.append(var_name)

    # unmark it as undefined
    while var_name in undefined_cpp_vars:
        undefined_cpp_vars.remove(var_name)

    # if a value is present, store it
    if len(name_value) > 1:
        # name_value[1] contains the variable value
        var_value = name_value[1]
        cpp_vars_values[var_name] = var_value

###  Management of C++ files and dependencies
file_info_map = {}

def file_info(filepath):
    "returns a FileInfo object for this file"
    "parsing the file if necessary, and remembering the result for later buffered reuse"
    absfilepath = abspath(filepath)
    if platform == 'win32':
        # Under Windows, file paths are case-insensitive.
        absfilepath = absfilepath.lower()
    if file_info_map.has_key(absfilepath):
        return file_info_map[absfilepath]
    else:
        new_file_info = FileInfo(filepath)
        file_info_map[absfilepath] = new_file_info
        new_file_info.analyseFile()
        return new_file_info

def get_ccpath_from_noncc_path(filepath):
    "returns the path to the .cc (or similar extension) corresponding"
    "to the given .h (or similar extension) file if it exists"
    "or None if there is no such file"
    (base,ext) = os.path.splitext(filepath)
    for newext in cpp_exts:
        ccpath = base + newext
        if mtime(ccpath):
            return ccpath
    return None

def isccfile(filepath):
    (base,ext) = os.path.splitext(filepath)
    return ext in cpp_exts

def get_ccfiles_to_compile_and_link(target, ccfiles_to_compile, ccfiles_to_link, executables_to_link,linkname):
    """A target can be a .cc file, a binary target, or a directory.
    The function updates (by appending to them) ccfiles_to_compile and ccfiles_to_link
    ccfiles_to_compile is a dictionary containing FileInfo of all .cc files to compile
    ccfiles_to_link is a dictionary containing FileInfo of all .cc files on
    which target depends
    executables_to_link is a dictionary containing FileInfo of all .cc files containing a
    main whose corresponding executable should be made."""
    target = abspath(target) # get absolute path
    if os.path.basename(target)[0] == '.': # ignore files and directories starting with a dot
        return
    if os.path.isdir(target):
        if os.path.basename(target) not in ['OBJS','CVS',get_OBJS_dir()]: # skip OBJS and CVS directories
            print "Entering " + target
            for direntry in os.listdir(target):
                newtarget = join(target,direntry)
                if os.path.isdir(newtarget) or isccfile(newtarget):
                    get_ccfiles_to_compile_and_link(newtarget, ccfiles_to_compile, ccfiles_to_link, executables_to_link, linkname)

    else:
        if isccfile(target):
            cctarget = target
        else:
            cctarget = get_ccpath_from_noncc_path(target)

        if not cctarget:
            print "Warning: bad target", target
        else:
            info = file_info(cctarget)
            ccfiles_to_link[info] = 1
            if info.hasmain or create_dll or create_so or create_pyso:
                if not force_link and not force_recompilation and info.corresponding_output_is_up_to_date() and not create_dll:
                    # Refresh symbolic link.
                    info.make_symbolic_link(linkname, None, info.corresponding_output)

                    if link_target_override and os.path.islink(info.corresponding_output):
                        src=os.path.realpath(info.corresponding_output)
                        os.remove(info.corresponding_output)
                        shutil.copyfile(src, info.corresponding_output)
                        shutil.copymode(src, info.corresponding_output)
                        print "The link target was a symlink. We replaced it with a binary."

                    print 'Target', info.filebase, 'is up to date.'
                else:
                    executables_to_link[info] = 1
                    for ccfile in info.get_ccfiles_to_link():
                        ccfiles_to_link[ccfile] = 1
                        if force_recompilation or not ccfile.corresponding_ofile_is_up_to_date():
                            #print ccfile.filebase
                            ccfiles_to_compile[ccfile] = 1
            elif force_recompilation or not info.corresponding_ofile_is_up_to_date():
                ccfiles_to_compile[info] = 1


###  File compilation and linking

def parallel_compile(files_to_compile, list_of_hosts, nice_values={},
        num_retries=3, ofiles_to_copy=[]):
    """files_to_compile is a list of FileInfo of .cc files"""

    def wait_for_some_completion(outs, errs, ofiles_to_copy, files_to_check):
        # copy ofiles while waiting
        if local_ofiles and ofiles_to_copy != []:
            copy_ofile_locally(ofiles_to_copy[0])
            del ofiles_to_copy[0]
        # if the process finished because the host was unreachable,
        # we do not want to reuse it
        iwtd, owtd, ewtd = select.select(outs.keys()+errs.keys(), [], [], 0.0001)
        if iwtd == []:
            return ''
        f = iwtd[0]
        if errs.has_key(f):
            info = errs[f]
        elif outs.has_key(f):
            info = outs[f]
        del errs[info.launched.childerr]
        del outs[info.launched.fromchild]

        # print error messages, warnings, and get failure/success status
        info.finished_compilation()
        if info.remove_hostname:
            if verbose>=3:
                print "Removing host from list:", info.hostname
            list_of_hosts.remove(info.hostname)
            info.hostname = ''

        # check for new ofiles to copy
        if local_ofiles:
            for f in files_to_check:
                if f.corresponding_ofile_is_up_to_date():
                    ofiles_to_copy+= [f.corresponding_ofile]
                    del files_to_check[files_to_check.index(f)]
            if hasattr(info, "compilation_status") and info.compilation_status == 0:
                if info.corresponding_ofile_is_up_to_date():
                    ofiles_to_copy+= [info.corresponding_ofile]
                else:
                    files_to_check+= [info]
        return info.hostname


    hostnum = 0
    hostname= ''
    outs = {}  # a dictionary indexed by the stdout of the process launched by popen, and containing the corresponding FileInfo object 
    errs = {}  # a dictionary indexed by the stderrs of the process launched by popen, and containing the corresponding FileInfo object
    files_to_check= []


    for ccfile in files_to_compile:

        ccfile.make_objs_dir() # make sure the OBJS dir exists, otherwise the command will fail
        #print len(outs)

        if len(outs) < len(list_of_hosts):# len(outs) number of launched compilation
            if len(files_to_compile)==1: # if there's just one file to compile, we compile it locally to avoid a long "Waiting for NFS to catch up!"
                hostname = 'localhost'
            else:
                hostname = list_of_hosts[hostnum]
            nice_value = nice_values.get(hostname, default_nice_value)
            ccfile.launch_compilation(hostname, nice_value)
            outs[ccfile.launched.fromchild] = ccfile
            errs[ccfile.launched.childerr] = ccfile
            hostnum = hostnum + 1
        else: # all processes are busy, wait for something to finish...
            hostname = ''
            while hostname == '':
                hostname = wait_for_some_completion(outs, errs, ofiles_to_copy, files_to_check)
                # This should not happen unless localhost is unable to compile
                if not list_of_hosts:
                    raise Exception("Couldn't access ANY of the listed hosts for compilation")
            else:
                nice_value = nice_values.get(hostname, default_nice_value)
                ccfile.launch_compilation(hostname, nice_value)
                outs[ccfile.launched.fromchild] = ccfile
                errs[ccfile.launched.childerr] = ccfile

    # Now wait until everybody is finished
    while outs:
        if len(outs)<=10 and hostname != '':
            if verbose>=2:
                print '[ STILL WAITING FOR:',string.join(map( lambda(f): f.filebase+f.fileext + ' on ' + f.hostname, outs.values()), ', '),']'
        hostname= wait_for_some_completion(outs, errs, ofiles_to_copy, files_to_check)

    # Check if we should retry some compilations...
    ccfiles_to_retry = [c for c in files_to_compile if c.retry_compilation]
    if len(ccfiles_to_retry) != 0:
        if num_retries > 0:
            if verbose >= 2:
                print '[ RETRYING COMPILATION FOR %d FILES ]' % len(ccfiles_to_retry)
            parallel_compile(ccfiles_to_retry, list_of_hosts, nice_values,
                    num_retries-1, ofiles_to_copy)
        else:
            if verbose >= 2:
                print '[ %d FILES TO RETRY, BUT NO MORE TRIES AVAILABLE ]' % len(ccfiles_to_retry)


def dbi_parallel_compile(files_to_compile, dbi_mode):
    #FIXME: Prepare something?
    commands = []

    for ccfile in files_to_compile:
        ccfile.make_objs_dir() # make sure the OBJS dir exists, otherwise the command will fail
        # Remove .o file if it exists
        try:
            os.remove(ccfile.corresponding_ofile)
        except OSError:
            pass

        # Maybe the "echo $?" part isn't useful
        commands.append("cd " + ccfile.filedir + "; " + ccfile.compile_command() + "; echo $?")

    from plearn.parallel.dbi import DBI
    jobs = DBI(commands, dbi_mode)
    jobs.micro = 10 # Compile 10 files on each node
    jobs.run()

def win32_parallel_compile(files_to_compile):
    """files_to_compile is a list of FileInfo of .cc files"""

    for ccfile in files_to_compile:
        ccfile.make_objs_dir() # make sure the OBJS dir exists, otherwise the command will fail
        ccfile.launch_compilation('localhost', default_nice_value)
        ccfile.finished_compilation() # print error messages, warnings, and get failure/success status

def sequential_link(executables_to_link, linkname):
    """executables_to_link is a list of FileInfo of .cc files that contain a main()
    and whose corresponding executable should be made"""
    link_exit_code = 0
    for ccfile in executables_to_link:
        failures =  ccfile.failed_ccfiles_to_link()
        if failures:
            print '[ Executable target',ccfile.filebase,'not remade because of previous compilation errors. ]'
            print '   Errors were while compiling',len(failures),'file(s):'
            print '   '+string.join(failures,'\n   ')
            if link_exit_code == 0:
                link_exit_code = 1
        else:
            if not local_compilation:
                if verbose>=2:
                    print 'Waiting for NFS to catch up...',
                # Flush stdhout to make sure the previous message shows up,
                # so if wait_for_all_...() doesn't work properly, then at
                # least we learn we are waiting for NFS now and not when we
                # press Ctrl+C.
                sys.stdout.flush()
                ccfile.nfs_wait_for_all_linkable_ofiles()
                print 'done'
            if local_ofiles:
                copy_ofiles_locally(executables_to_link)
            if verbose>=2:
                print '[ LINKING',ccfile.filebase,']'
            new_link_exit_code = ccfile.launch_linking()
            if link_exit_code == 0:
                link_exit_code = new_link_exit_code
            if create_so or create_pyso:
                so_filename = os.path.basename(ccfile.corresponding_output)
                link_so_filename = so_filename
                # Add machine-dependent info ('-32' or '-64') if necessary
                if create_pyso and link_32_64:
                    if platform.startswith('linux-x86_64') or platform.startswith('linux-ia64'):
                        link_so_filename += '-64'
                    else:
                        link_so_filename += '-32'
                ccfile.make_symbolic_link(link_so_filename, so_filename)
            else:
                ccfile.make_symbolic_link(linkname)
    return link_exit_code

def sequential_dll(target_file_info):
    """target_file_info is a FileInfo of .cc file whose corresponding dll should be made"""
    failures =  target_file_info.failed_ccfiles_to_link()
    if failures:
        print '[ Executable target',target_file_info.filebase,'not remade because of previous compilation errors. ]'

        print '   Errors were while compiling files:'
        print '   '+string.join(failures,'\n   ')
    else:
        print 'Waiting for NFS to catch up...'
        target_file_info.nfs_wait_for_all_linkable_ofiles()
        print '[ CREATING DLL',target_file_info.filebase+'.dll',']'
        target_file_info.launch_dll_wrapping()


def get_so_options():
    """Returns the linker options related to shared libraries"""
    so_options = ""    
    if create_so:
        if platform=='darwin':
            so_options = " -dylib -flat_namespace "            
        else:
            so_options = " -shared "
    elif create_pyso:
        if platform=='darwin':
            so_options = " -bundle -flat_namespace "            
        else:
            so_options = " -shared "
    elif static_linking:                
        so_options = " -static "

    return so_options


###  Special calling options, that don't actually compile anything

## this function takes a target, and extract all the sources necessary
## to compile the target, in a directory call your_target.dist
## in this directory, there will be a Makefile that is able to compile
## and link your target
def distribute_source(target, ccfiles_to_compile, executables_to_link, linkname):
    print '+++++ Distributing ';
    dist_dir = os.path.basename(target) + ".dist";
    if os.path.isdir(dist_dir):  # make sure the OBJS dir exists, otherwise the command will fail
        print "Directory " + dist_dir + " already exists"
        sys.exit(100)
    os.makedirs(dist_dir)
    os.makedirs(os.path.join(dist_dir,get_OBJS_dir()))
    makefile = open(os.path.join(dist_dir,"Makefile"),"w")
    compiler = default_compiler
    compileroptions = ""

    linker = default_linker
    linkeroptions = ""
    so_options = get_so_options()
    if force_32bits:
        linkeroptions = linkeroptions + ' -m32'
    for opt in options:
        optdef = pymake_options_defs[opt]
        if optdef.linkeroptions:
            linkeroptions = linkeroptions + ' ' + optdef.linkeroptions
        if optdef.linker:
            linker = optdef.linker

    if force_32bits:
        compileroptions = compileroptions + ' -m32'
    for opt in options:
        optdef = pymake_options_defs[opt]
        compileroptions = compileroptions + ' ' + optdef.compileroptions
        compileroptions += ''.join([' -D'+d for d in (cpp_definitions + optdef.cpp_definitions)])
        if optdef.compiler:
            compiler = optdef.compiler

    for f in file_info_map.values():
        [dir_f, file_f] = f.get_rel_dir_file()
        if not os.path.isdir(os.path.join(dist_dir,dir_f)):
            os.makedirs(os.path.join(dist_dir,dir_f))
        shutil.copyfile(f.filepath, os.path.join(dist_dir,dir_f,file_f))

    for ccfile in executables_to_link:
        objsfilelist = []
        for lf in ccfile.get_ccfiles_to_link():
            objsfilelist.append(os.path.join(get_OBJS_dir(),
                                             os.path.basename(lf.corresponding_ofile)))

        command = linker + so_options + ' -o ' + os.path.basename(ccfile.corresponding_output) +\
                  ' ' + string.join(objsfilelist,' ') + ' ' + \
                  ccfile.get_optional_libraries_linkeroptions() + ' ' + linkeroptions + ' ' +\
                  linkeroptions_tail
        makefile.write(os.path.basename(ccfile.corresponding_output) + " : " + string.join(objsfilelist,' ') + "\n")
        makefile.write('\t' + command)
        makefile.write("\n")
        makefile.write("\n")

    for ccfile in ccfiles_to_compile.keys():
        [dir_cc, file_cc] = ccfile.get_rel_dir_file()
        out_file_o=os.path.join(get_OBJS_dir(),ccfile.filebase + ".o")
        makefile.write(out_file_o+" : " + os.path.join(dir_cc,file_cc) + "\n")
        makefile.write('\t' + compiler + ' ' + compileflags + ' ' + \
                           compileroptions + ' -o ' + out_file_o + " -c " + \
                           os.path.join(dir_cc,file_cc))
        makefile.write("\n")
        makefile.write("\n")

    makefile.close()

def find_dependency(args,type='link'):
    '''their is 2 type supported: link and incude
    link type list dependency at link time, so all .h file 'include'
        indirectly their .cc file
    include type is the dependency that make that file need to be recompiled'''
    global sourcedirs

    target = args[0]
    configpath = get_config_path(target)
    execfile( configpath, globals() )
    global options
    options = getOptions(options_choices, optionargs)

    sourcedirs = unique(sourcedirs)

    if isccfile(target) or type=='include':
        cctarget = target
    else:
        cctarget = get_ccpath_from_noncc_path(target)

    if not cctarget:
        raise IOError("File not found: "+target)
    elif not os.path.exists(cctarget):
        raise IOError("File not found: "+cctarget)

    info = file_info(cctarget)

    if len(args) == 1: # only the target was specified: generate the full graph
        print 'Generating dependency graph in '+target+'.dot ...'
        info.save_dependency_graph(target+'.dot', type)
        print 'Generating dependency graph view in '+target+'.ps ...'
        os.system('dot -T ps '+target+'.dot > '+target+'.ps')
        print 'Generating dependency graph view in '+target+'.png ...'
        os.system('dot -T png '+target+'.dot > '+target+'.png')
    elif len(args) == 2: # target was specified with a possible source dependency
        dep = args[1]
        print 'First encountered dependency path linking '+target+' to '+dep+ ' :'
        if not info.print_dependency_path(dep,[], type):
            print 'THERE APPEARS TO BE NO SUCH DEPENDENCY.'


def generate_vcproj_files(target, ccfiles_to_compile, executables_to_link, linkname):
    list_of_file_objects = ccfiles_to_compile.keys()

    (project_base_name_with_dir, ext) = os.path.splitext(target)
    project_base_name = os.path.basename(project_base_name_with_dir)
    vcproj_file = project_base_name + '.vcproj'
    print 'Creating Visual Studio project file %s for target %s.' % \
        (vcproj_file, target)
    import xml.dom.ext
    import xml.dom.minidom
    vcproj_xml = xml.dom.minidom.Document()
    root = vcproj_xml.createElement("VisualStudioProject")
    root.setAttribute('Name', project_base_name)
    root.setAttribute('ProjectType', 'Visual C++')
    root.setAttribute('Version', '7.10')
    root.setAttribute('Keyword', 'Win32Proj')
    platforms_node = vcproj_xml.createElement('Platforms')
    platform_node = vcproj_xml.createElement('Platform')
    platform_node.setAttribute('Name', 'Win32')
    platforms_node.appendChild(platform_node)
    root.appendChild(platforms_node)

    configurations_node = vcproj_xml.createElement('Configurations')
    configuration_node = vcproj_xml.createElement('Configuration')
    configuration_node.setAttribute('Name', 'Debug|Win32')
    configuration_node.setAttribute('OutputDirectory', 'Debug')
    configuration_node.setAttribute('IntermediateDirectory', 'Debug')
    configuration_node.setAttribute('ConfigurationType', '1')
    configuration_node.setAttribute('CharacterSet', '2')
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCCLCompilerTool')
    comp_options = list_of_file_objects[0].compiler_options()
    comp_options += ' -DBOUNDCHECK' # Enable BOUNDCHECK in debug.
    comp_options += ' /GR' # Enable runtime type information
                           # for dynamic casts.
    # Note that the following line implies that a function named
    # PL_repository_revision has been defined in the config file,
    # as it is currently the case in pymake.config.model.
    # A future (better) version of this code should rather gather
    # individual compilation options for each file, and set them
    # accordingly in the Visual Studio project file.
    comp_options += ' -DPL_REPOSITORY_REVISION=%s' \
                    % PL_repository_revision()

    # Get rid of potential MinGW definition when pymake is run under
    # MinGW to generate the Visual Studio project.
    comp_options = comp_options.replace('-D_MINGW_', '')
    tool_node.setAttribute('AdditionalOptions', comp_options)
    tool_node.setAttribute('Optimization', '0')

    # Automatically add include directories from CPATH
    cpath_env = os.getenv('CPATH')
    add_include_dirs = [ plearndir ]
    if cpath_env is not None:
        add_include_dirs += cygwin_to_win_paths(cpath_env)

    tool_node.setAttribute('AdditionalIncludeDirectories', \
                           ';'.join(add_include_dirs))
    tool_node.setAttribute('PreprocessorDefinitions', 'WIN32;_DEBUG;_CONSOLE')
    tool_node.setAttribute('MinimalRebuild', 'TRUE')
    tool_node.setAttribute('BasicRuntimeChecks', '3')
    tool_node.setAttribute('RuntimeLibrary', '5')
    tool_node.setAttribute('ForceConformanceInForLoopScope', 'TRUE')
    tool_node.setAttribute('UsePrecompiledHeader', '0')
    tool_node.setAttribute('WarningLevel', '3')
    tool_node.setAttribute('Detect64BitPortabilityProblems', 'TRUE')
    tool_node.setAttribute('DebugInformationFormat', '4')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCCustomBuildTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCLinkerTool')

    # Add additional library dependencies.
    for linkf in executables_to_link.keys():
        # Obtain the link command: we set 'symlinkobjs' to 0 to
        # ensure no file links are created.
        tmp = symlinkobjs
        symlinkobjs = 0
        link_command = linkf.link_command()
        symlinkobjs = tmp
        # Extract libraries that must be linked with, i.e. linker
        # options of the form '-lxxx'.
        add_libs = []
        start = 0
        while (start >= 0):
            start = link_command.find(' -l', start)
            if start >= 0:
                end = link_command.find(' ', start+1)
                if end < 0:
                    # Might happen if '-lxxx' is the very last
                    # statement on the link command line.
                    end = len(link_command)
                libname = link_command[start+3:end]
                # Handle some special cases.
                if libname == 'python24':
                    result = '%s.lib' % libname
                elif libname in ['numarray', 'boost_regex', 'm']:
                    # These libraries are not needed with Visual
                    # Studio, and can be ignored.
                    # Note: dependence on the Boost libraries is
                    # apparently obtained automatically by Visual
                    # Studio.
                    result = None
                else:
                    # Default name is 'libxxx.lib'.
                    result = 'lib%s.lib' % libname
                if result is not None:
                    add_libs.append(result)
                start += 1

    tool_node.setAttribute('AdditionalDependencies', \
                           ' '.join(add_libs))

    tool_node.setAttribute('OutputFile',
                           '$(OutDir)/%s.exe' % project_base_name)
    tool_node.setAttribute('LinkIncremental', '2')

    # Automatically add library directories from LIBRARY_PATH.
    library_path_env = os.getenv('LIBRARY_PATH')
    if library_path_env is not None:
        add_lib_dirs = cygwin_to_win_paths(library_path_env)
        tool_node.setAttribute('AdditionalLibraryDirectories', \
                               ';'.join(add_lib_dirs))
    tool_node.setAttribute('GenerateDebugInformation', 'TRUE')
    tool_node.setAttribute('ProgramDatabaseFile', '$(OutDir)/%s.pdb' % project_base_name)
    tool_node.setAttribute('SubSystem', '1')
    tool_node.setAttribute('TargetMachine', '1')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCMIDLTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCPostBuildEventTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCPreBuildEventTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCPreLinkEventTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCResourceCompilerTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCWebServiceProxyGeneratorTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCXMLDataGeneratorTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCWebDeploymentTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCManagedWrapperGeneratorTool')
    configuration_node.appendChild(tool_node)
    tool_node = vcproj_xml.createElement('Tool')
    tool_node.setAttribute('Name', 'VCAuxiliaryManagedWrapperGeneratorTool')
    configuration_node.appendChild(tool_node)
    configurations_node.appendChild(configuration_node)
    root.appendChild(configurations_node)

    references_node = vcproj_xml.createElement('References')
    root.appendChild(references_node)
    files_node = vcproj_xml.createElement('Files')
    filter_source_node = vcproj_xml.createElement('Filter')
    filter_source_node.setAttribute('Name', 'Source Files')
    filter_source_node.setAttribute('Filter', \
                          'cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx')
    files_node.appendChild(filter_source_node)
    filter_header_node = vcproj_xml.createElement('Filter')
    filter_header_node.setAttribute('Name', 'Header Files')
    filter_header_node.setAttribute('Filter', \
                          'h;hpp;hxx;hm;inl;inc;xsd')
    files_node.appendChild(filter_header_node)
    root.appendChild(files_node)
    globals_node = vcproj_xml.createElement('Globals')
    root.appendChild(globals_node)
    vcproj_xml.appendChild(root)
    #xml.dom.ext.PrettyPrint(vcproj_xml)

    # Find all files to add to the project.
    # TODO Note that there must be missing header files (those with
    # no corresponding cc file).
    list_of_files = []
    for ccfile in list_of_file_objects:
        list_of_files.append(ccfile.filepath)
        (basename, ext) = \
            os.path.splitext(os.path.realpath(ccfile.filepath))
        if os.path.exists(basename + '.h'):
            list_of_files.append(basename + '.h')

    # Now go through all files and add them to the project, also
    # creating the corresponding directory hierarchy.
    plearn_root = os.path.realpath(plearndir)
    current_dir = os.path.normcase(os.path.realpath(os.getcwd()))
    for ccfile in list_of_files:
        file = os.path.normcase(os.path.realpath(ccfile))
        # Find relative Windows path.
        file = os.path.normcase(file)
        prefix = os.path.commonprefix((file, current_dir))
        # Get rid of potential truncated directory name due to the
        # way commonprefix(..) works (character by character).
        (prefix, tail) = os.path.split(prefix)
        win_path = ''
        if prefix != '':
            head = current_dir
            while head != prefix:
                (head, tail) = os.path.split(head)
                win_path = os.path.join(win_path, '..')
        else:
            print 'Cannot currently recover relative path if ' \
                  'there is no common prefix (between %s and %s)' \
                  % (file, current_dir)
            sys.exit(1)
        win_path += file[len(prefix):]
        win_path = win_path.replace('/', '\\')

        # Find out if this file is in PLearn.
        if os.path.commonprefix((file, plearn_root)) == plearn_root:
            is_in_plearn = True
            file = file[len(plearn_root) + 1:]
            # print file
        else:
            is_in_plearn = False
        directory = os.path.dirname(file)
        tail = 'dummy'
        list_of_dirs = []
        while directory != '' and tail != '':
            (directory, tail) = os.path.split(directory)
            if tail != '':
                list_of_dirs.append(tail)
        if is_in_plearn:
            list_of_dirs.append('PLearn')
        list_of_dirs.reverse()
        # print list_of_dirs
        (base, ext) = os.path.splitext(file)
        if ext == '.cc' or ext == '.cpp':
            current_files_node = filter_source_node
        elif ext == '.h':
            current_files_node = filter_header_node
        else:
            print 'File %s has an unknown extension, aborting!' \
                    % file
            sys.exit(1)
        for directory in list_of_dirs:
            # Check if this directory already exists.
            found = False
            for n in current_files_node.childNodes:
                if n.localName == 'Filter' and n.getAttribute('Name') == directory:
                    found = True
                    current_files_node = n
            if not found:
                new_node = vcproj_xml.createElement('Filter')
                new_node.setAttribute('Name', directory)
                #new_node.setAttribute('Filter', '')
                current_files_node.appendChild(new_node)
                current_files_node = new_node
        new_file_node = vcproj_xml.createElement('File')
        new_file_node.setAttribute('RelativePath', win_path)
        current_files_node.appendChild(new_file_node)

    saved_file_tmp_name = vcproj_file + '.tmp'
    saved_file_tmp = open(saved_file_tmp_name, 'w')
    xml.dom.ext.PrettyPrint(vcproj_xml, saved_file_tmp)
    saved_file_tmp.close()
    # This 'magic' sed command is meant to make sure the 'Name'
    # attribute is always first, as otherwise Visual Studio does
    # not want to load the project file.
    command_to_run = \
        "sed -e \"s/<\\([^ ]\\+\\) \\(.*\\)\\( Name='[^']*'\\)\\(.*\\)>/<\\1\\3 \\2\\4>/g\" %s" % saved_file_tmp_name
    final_content = toolkit.command_output(command_to_run)
    saved_file = open(vcproj_file, 'w')
    saved_file.writelines(final_content)
    os.remove(saved_file_tmp_name)


def reportMissingObj(arg, dirpath, names):
    if 'OBJS' in names: names.remove('OBJS')
    if 'CVS' in names: names.remove('CVS')
    objs_dir_to_use=get_OBJS_dir()
    if objs_dir_to_use in names: names.remove(objs_dir_to_use)
    for fname in names:
        fpath = join(dirpath,fname)
        if os.path.isfile(fpath):
            basename, ext = os.path.splitext(fname)
            if ext in cpp_exts:
                foundobj = 0 # found the .o file?
                for f in glob.glob(join(dirpath,objs_dir_to_use,
                                        '*',basename+'.o')):
                    if os.path.isfile(f): foundobj = 1
                if not foundobj:
                    print fpath

def rmOBJS(arg, dirpath, names):
    objs_dir_to_use=get_OBJS_dir()
    if os.path.basename(dirpath) == 'OBJS' or\
            os.path.basename(dirpath)==objs_dir_to_use:
        print 'Removing', dirpath
        shutil.rmtree(dirpath)


#####  Definition of FileInfo class  ##########################################

class FileInfo:
    """
    This class is specifically designed to hold all useful information about .cc and .h files.

    Contains the following fields:
    - mtime: last modification time of this file
    - filepath: full absolute path of file
    - filedir:  dirtectory part of filepath
    - filebase: file basename part of filepath
    - fileext:  file extension
    - is_ccfile: true if this file has a .cc or similar extension, false if it has a .h or similar extension
    - includes_from_sourcedirs: list of FileInfo of the files that this one includes found in the sourcedirs
    - objsfilelist: list of object files used at link time
    - triggered_libraries: list of OptionalLibrary objects triggered by this file's includes

    QT specific:

    - isqt : (for .cc files) true if this file is the brother .cc of a .h that 'hasqobject==true', or if for this .cc 'hasqobject==true'
    - hasqobject : true if file contains a Q_OBJECT declaration (in which case we need to preprocess it with moc)
    - mocfilename : the name of the associated moc file, if appropriate, that is, if hasqobject is true
    - corresponding_moc_cc : the fileinfo of the .cc generated by moc'ing the file

    .h files will also have the following fields:
      - corresponding_ccfile: FileInfo of corresponding .cc file if any, 0 otherwise 

    .cc files will also have the following fields
      - corresponding_ofile: full path of corresponding .o file    
      - hasmain: true if the sourcefile has a main function
      If hasmain is true, then the following field will also be set:
        - corresponding_output: full path of corresponding executable (in the same directory as the obj)
      If create_dll is true, then the following fields will be set:
        - corresponding_output: full path of corresponding dll file (in the same directory as the source file)
        - corresponding_def_file: full path of corresponding def file (in the same directory as the source file)
      If create_so is true, then the following field will be set:
        - corresponding_output: full path of corresponding .so file (in the same directory as the source obj)


    These attributes should not be accessed directly, but through their get method
    - depmtime (optionally built by a call to get_depmtime())
    - ccfiles_to_link list containing the FileInfos of all .cc files whose .o must be linked to produce the executable

    When launching a compilation, the following are also set:
    - hostname: name of host on which the compilation has been launched
    - launched: a Popen3 object for the launched process
    - errormsgs: a list of error messages taken from the launched process' stdout
    - warningmsgs: a list of warning messages taken from the launched process' stderr
    - retry_compilation: whether to retry the compilation because the current failure was caused by an out of memory error, etc.
    - remove_hostname: whether to remove hostname from list_of_hosts, because the current failure was caused by this machine
    """

    # static patterns useful for parsing the file
    qobject_regexp = re.compile(r'\bQ\_OBJECT\b',re.M)
    hasmain_regexp = re.compile(r'\b(main|IMPLEMENT_APP)\s*\(',re.M)

    def __init__(self,filepath):
        if not os.path.exists(filepath):
            raise IOError("Couldn't find file " + filepath)
        if platform == 'win32':
            self.filepath = filepath
            if self.filepath[1] == ":":
                # We must ensure that the drive letter is always uppercase,
                # otherwise there may be duplicates in the object files list,
                # leading to link errors.
                self.filepath = self.filepath.capitalize()
        else:
            # Remove potential links in the path.
            self.filepath = os.path.realpath(filepath)


    def parse_file(self):
        """ Parses the file, and sets self.includes_from_sourcedirs, self.triggered_libraries and self.hasmain"""
        import minicpreproc

        # print "Parsing", self.filepath

        f = open(self.filepath,"r")
        text = f.read()
        f.close()
        text = remove_escaped_endlines(text)
        text = remove_comments(text)
        cpp_code = get_cpp_code(text)
        self.includes_from_sourcedirs = []
        self.triggered_libraries = []
        for includefile in minicpreproc.get_include_list(cpp_code, undefined_cpp_vars, defined_cpp_vars, cpp_vars_values):
            #print 'Considering include ' + includefile
            for optlib in optional_libraries_defs:                
                triggered_by = optlib.is_triggered_by(includefile)

                if triggered_by and not optlib in self.triggered_libraries:
                    # print 'triggered: ' + optlib.name + '(file = ' + self.filepath + ', ' + includefile + ')'
                    # print optlib.name + ' TRIGGERED BY INCLUDE ' + `includefile` + ' IN ' + `self.filepath` + ' triggers = '+repr(optlib.triggers) 
                    optlib.convert_all_options_to_strings()
                    self.triggered_libraries.append(optlib)
                    # print optlib.name + ' TRIGGERED BY INCLUDE ' + includefile + ' IN ' + self.filepath + ' triggers = '+repr(optlib.triggers) 

                # Some optional libraries may have to be recompiled...
                if isinstance(triggered_by, type("")) and optlib.compile_as_source:
                    self.includes_from_sourcedirs.append(abspath(triggered_by))

            fullpath = join(self.filedir, includefile)
            if os.path.isfile(fullpath):
                self.includes_from_sourcedirs.append(abspath(fullpath))
            else:
                for srcdir in sourcedirs:
                    fullpath = join(srcdir, includefile)
                    if os.path.isfile(fullpath):
                        self.includes_from_sourcedirs.append(abspath(fullpath))

        self.hasmain = FileInfo.hasmain_regexp.search(text)
        if platform!='win32':
            self.hasqobject = FileInfo.qobject_regexp.search(text)
        else:
            self.hasqobject = False
        self.mocfilename = "moc_"+self.filebase+".cc"

    def mocFile(self):
        """QT specific : if moc_'filename'.cc is older than 'filename'.h,
        then moc 'filename'.h (generates) moc_'filename'.cc"""

        if qtdir == '':
            print "\nError : The qtdir variable is unset but the file " + self.filepath + " seems to use QT (a Q_OBJECT declaration was found). In your .pymake/config file, assign the qtdir variable to the path to Trolltech's QT installation. Example : qtdir = '/usr/lib/qt3/'"
            sys.exit(100)

        if (os.path.isfile(self.mocfilename)) and self.get_depmtime() <= os.path.getmtime(self.mocfilename):
            return 0

        return os.WEXITSTATUS(os.system(qtdir+"bin/moc "+self.filepath+" -o "+self.mocfilename))

    def analyseFile(self):
        global useqt

        self.mtime = self.get_mtime()
        self.filedir, fname = os.path.split(self.filepath)
        self.filebase, self.fileext = os.path.splitext(fname)

        # Parse the file to get includes and check if there's a main()
        self.parse_file()
        # transform the list of includes from a list of file names to a list of correspondinf FileInfo
        self.includes_from_sourcedirs = map(file_info,self.includes_from_sourcedirs)

        if self.fileext in h_exts:
            self.is_ccfile = 0

            # get the corresponding .cc file's FileInfo (if there is such a file)
            self.corresponding_ccfile = 0
            for newext in cpp_exts:
                ccpath = join(self.filedir, self.filebase + newext)
                if mtime(ccpath):
                    self.corresponding_ccfile = file_info(ccpath)
                    break

        elif self.fileext in cpp_exts:
            self.is_ccfile = 1

            if objspolicy == 1:
              self.corresponding_ofile = join(self.filedir, objsdir, self.filebase+'.o')
            elif objspolicy == 2:
              self.corresponding_ofile = join(objsdir, self.filebase+'.o')

            if link_target_override:
                self.corresponding_output = link_target_override

            elif create_dll:
                self.corresponding_output = join(self.filedir, self.filebase+'.dll')
                self.corresponding_def_file = join(self.filedir, self.filebase+'.def')

            elif create_so:
                if platform=='darwin':
                    self.corresponding_output = join(self.filedir, objsdir, 'lib'+self.filebase+'.dylib')
                else:
                    self.corresponding_output = join(self.filedir, objsdir, 'lib'+self.filebase+'.so')
                    
            elif create_pyso:
                self.corresponding_output = join(self.filedir, objsdir, self.filebase+'.so')

            elif self.hasmain:
                self.corresponding_output = join(self.filedir, objsdir, self.filebase)
                # We append options to the file name if they are not appended to the objsdir name
                for opt in options:
                    pyopt = pymake_options_defs[opt]
                    if not pyopt.in_output_dirname:
                        self.corresponding_output = self.corresponding_output + '_' + opt

        else:
            raise 'Attempting to build a FileInfo from a file that is not a .cc or .h or similar file ('+self.filepath+')'

        if self.hasqobject:
            useqt = 1
            self.mocFile()
            self.corresponding_moc_cc = file_info(self.mocfilename)

    def collect_ccfiles_to_link(self, ccfiles_to_link, visited_hfiles):
        """completes the ccfiles_to_link list by appending the FileInfo
        for all .cc files related to this file through includes and .cc
        files corresponding to .h files"""
        if self.is_ccfile:
            if self not in ccfiles_to_link:
                ccfiles_to_link.append(self)
                #print 'APPENDING CC: %s%s' % (self.filebase, self.fileext)
                for include in self.includes_from_sourcedirs:
                    include.collect_ccfiles_to_link(ccfiles_to_link,
                                                    visited_hfiles)
        else: # it's a .h file
            if self not in visited_hfiles:
                visited_hfiles.append(self)
                #print 'APPENDING H: %s%s' % (self.filebase, self.fileext)
                if self.corresponding_ccfile:
                    #print 'HAS_CC: %s%s' % (self.corresponding_ccfile.filebase,
                    #                        self.corresponding_ccfile.fileext)
                    self.corresponding_ccfile.collect_ccfiles_to_link(
                            ccfiles_to_link, visited_hfiles)

                if self.hasqobject:
                    self.corresponding_moc_cc.collect_ccfiles_to_link(
                            ccfiles_to_link, visited_hfiles)

                for include in self.includes_from_sourcedirs:
                    include.collect_ccfiles_to_link(ccfiles_to_link,
                                                    visited_hfiles)

    def get_ccfiles_to_link(self):
        """returns the list of FileInfos of all .cc files that need to be linked together to produce the corresponding_output"""
        if not hasattr(self,"ccfiles_to_link"):
            #if not self.hasmain or not self.is_ccfile:
            if (not self.hasmain and not create_dll and not create_so and not create_pyso) or not self.is_ccfile:
                raise Exception("called get_ccfiles_to_link on a file that is not a .cc file or that does not contain a main()")
            self.ccfiles_to_link = []
            visited_hfiles = []
            self.collect_ccfiles_to_link(self.ccfiles_to_link,visited_hfiles)

        return self.ccfiles_to_link


    def collect_optional_libraries(self, optlibs, visited_files):
        if self not in visited_files:
            # print 'collecting optional libraries for file', self.filebase+self.fileext
            # print 'ENTERING'
            visited_files.append(self)
            if hasattr(self,'optional_libraries'):
                # print 'hasattr: ', map(lambda(x): x.name, self.optional_libraries)
                appendunique(optlibs, self.optional_libraries)
            else:
                # print 'appending triggered libraries'
                appendunique(optlibs, self.triggered_libraries)
                for include in self.includes_from_sourcedirs:
                    # print 'lookin up include', include.filebase+include.fileext
                    include.collect_optional_libraries(optlibs, visited_files)

    def get_optional_libraries(self):
        """returns the list of all OptionalLibrary that this file triggers directly or indirectly"""
        if not hasattr(self,'optional_libraries'):
            optlibs = []
            self.collect_optional_libraries(optlibs, [])
            self.optional_libraries = optlibs
        return self.optional_libraries

    def get_all_optional_libraries_to_link(self):
        """returns the list of all optional libraries to link with this .cc"""
        # reorder self.optional_libraries in the order in which they appear in optional_libraries_defs
        # self.optional_libraries = [ x for x in optional_libraries_defs if x in self.optional_libraries ]
        # old-fashioned python that will work on troll (xsm):
        optlibs = []
        for ccfile in self.get_ccfiles_to_link():
            appendunique(optlibs, ccfile.get_optional_libraries())
        sol= []
        for x in optional_libraries_defs:
            if x in optlibs:
                sol = sol + [x]
        return sol

#     def get_optional_libraries(self):
#         """returns the list of *all* OptionalLibrary that need to be linked with any program that uses this file, in the right order"""
#         print "go(",self.filebase+self.fileext,")"
#         if not hasattr(self,'optional_libraries'):
#             self.optional_libraries = []
#             appendunique(self.optional_libraries, self.triggered_libraries)
#             if not self.is_ccfile and self.corresponding_ccfile:
#                 print self.filebase+self.fileext, " moving into corresponding .cc file"
#                 appendunique(self.optional_libraries, self.corresponding_ccfile.get_optional_libraries())
#             print self.filebase+self.fileext, " moving into INCLUDES: ", map( lambda(x): x.filebase, self.includes_from_sourcedirs)
#             for include in self.includes_from_sourcedirs:
#                 appendunique(self.optional_libraries, include.get_optional_libraries())
#             # reorder self.optional_libraries in the order in which they appear in optional_libraries_defs
#             # self.optional_libraries = [ x for x in optional_libraries_defs if x in self.optional_libraries ]
#             # old-fashioned python that will work on troll (xsm):
#             sol= []
#             for x in optional_libraries_defs:
#                 if x in self.optional_libraries:
#                     sol = sol + [x]
#             self.optional_libraries = sol
#         print "optlib(",self.filebase+self.fileext, " = ", map(lambda(x): x.name, self.optional_libraries)
#         return self.optional_libraries

    def get_optional_libraries_compileroptions(self):
        compopts = []
        for optlib in self.get_optional_libraries():
            compopts.append(optlib.compileroptions)
            # print '+> ', optlib.name, optlib.compileroptions
        return compopts

    def get_optional_libraries_includedirs(self):
        incldirs = []
        for optlib in self.get_optional_libraries():
            appendunique(incldirs, optlib.includedirs)
            # print '+> ', optlib.name, optlib.includedirs

        if useqt:
            appendunique(incldirs, [ qtdir+"include/" ] )

        return incldirs

    def get_optional_libraries_linkeroptions(self):
        linkeroptions = ''
        for optlib in self.get_all_optional_libraries_to_link():
            linkeroptions = linkeroptions + optlib.linkeroptions + ' '

        if useqt:
            if os.path.exists(qtdir + 'lib/libqt.so'):
                linkeroptions = linkeroptions + '-L'+qtdir + 'lib/ -lqt'
            else:
                linkeroptions = linkeroptions + '-L'+qtdir + 'lib/ -lqt-mt'
        return linkeroptions

    def get_mtime(self):
        if not hasattr(self,"mtime"):
            self.mtime = os.path.getmtime(self.filepath)
        return self.mtime

    def get_depmtime(self):
        "returns the single latest last modification time of"
        "this file and all its .h dependencies through includes"
        if not hasattr(self,"depmtime"):
            # YB DEBUGGING
            self.mtime = self.get_mtime()
            #
            self.depmtime = self.mtime
            for includefile in self.includes_from_sourcedirs:
                depmtime = includefile.get_depmtime()
                if depmtime>self.depmtime:
                    self.depmtime = depmtime
                    #print "time of ",includefile.filepath," = ",depmtime
        return self.depmtime

    def file_is_modified(self):
        """return false if the corresponding .o file is up to date,
        without taking dependencies into accout"""
        try:
            if self.fileext in cpp_exts:
                if not os.path.exists(self.corresponding_ofile):
                    return False
                else:
                    t1 = self.get_mtime()
                    t2 = os.path.getmtime(self.corresponding_ofile)
                    return t1 > t2
            elif self.fileext in h_exts:
                f=self.corresponding_ccfile
                if not f or not os.path.exists(f.filepath):
                    return False
                else:
                    t1 = self.get_mtime()
                    t2 = os.path.getmtime(f.corresponding_ofile)
#                    print self.filename(), f.filename(), t1,t2,os.path.getmtime(self.filepath), t1<=t2, t1>t2
                    return t1 > t2
                pass
            else:
                print self.filepath, "not handled here."
                return False
        except OSError: # OSError: [Errno 116] Stale NFS file handle
            return False
        
    def corresponding_ofile_is_up_to_date(self):
        """returns true if the corresponding .o file is up to date,
        false if it needs recompiling."""
        try:
            if not os.path.exists(self.corresponding_ofile):
                # print "path of ",self.corresponding_ofile," does not exist!"
                return 0
            else:
                t1 = self.get_depmtime()
                t2 = os.path.getmtime(self.corresponding_ofile)
                #print "t1 = source of ",self.filepath," time = ",t1
                #print "t2 = object ",self.corresponding_ofile," time = ",t2
                return t1 <= t2
        except OSError: # OSError: [Errno 116] Stale NFS file handle
            print "OSError... (probably NFS latency); Will be retrying"
            return 0

    def corresponding_output_is_up_to_date(self):
        """returns true if the corresponding executable is up to date, false if it needs rebuilding"""
        if not os.path.exists(self.corresponding_output):
            return 0
        else:
            exec_mtime = os.path.getmtime(self.corresponding_output)
            ccfilelist = self.get_ccfiles_to_link()
            for ccfile in ccfilelist:
                if not os.path.exists(ccfile.corresponding_ofile):
                    return 0
                ofile_mtime = os.path.getmtime(ccfile.corresponding_ofile)
                if ccfile.get_depmtime()>ofile_mtime or ofile_mtime>exec_mtime:
                    return 0
            return 1

    def nfs_wait_for_corresponding_ofile(self):
        """wait until nfs catches up...."""
        while not self.corresponding_ofile_is_up_to_date():
            time.sleep(0.1) #no active wait (was 'pass') -xsm

    def nfs_wait_for_all_linkable_ofiles(self):
        for ccfile in self.get_ccfiles_to_link():
            ccfile.nfs_wait_for_corresponding_ofile()

    def make_objs_dir(self):
        """ makes the OBJS dir and subdirectory if they don't already exist,
        to hold the corresponding_ofile and corresponding_output """
        odir = join(self.filedir, objsdir)
        if not os.path.isdir(odir):  # make sure the OBJS dir exists, otherwise the command will fail
            os.makedirs(odir)

    def make_symbolic_link(self, linkname, symlink_source_basename = None, symlink_to = None):
        """Recreates the symbolic link in filedir pointing to the
        executable in subdirectory objsdir.

        If the second argument, which is the user defined path and name for
        the symbolic link, is specified then the symbolic link will have
        this path and name, and the object files will be placed in the OBJS
        subdirectory of the directory where the source file is.

        If the third argument is given, it overrides the target of the link,
        i.e. the location of the executable object."""

        # In the following, we create the link 'symlink_from' -> 'symlink_to'.

        #if we overrided the link-target, we should link to it.
        if link_target_override and not symlink_to:
            symlink_to=os.path.abspath(link_target_override)

        # First, we change to the directory of the source file. This is to
        # ensure that if the target is given by a relative path (in OBJS/...)
        # then this relative path is taken relative to the source file path.
        backup_cwd = os.getcwd()
        os.chdir(self.filedir)

        # User-provided base name for the file we point to?
        if symlink_source_basename is not None:
            symlink_to_base = symlink_source_basename
        else:
            symlink_to_base = self.corresponding_output

        if not symlink_to:
            symlink_to = join(objsdir, symlink_to_base)
            if linkname != '':
                symlink_to = join(self.filedir, symlink_to)

        # User-provided path for the link itself?
        if linkname == '':
            linkbase = self.filebase
            if create_so:
                linkbase = 'lib%s.so' % linkbase
            elif create_pyso:
                suffix = ''
                if link_32_64:
                    if platform.startswith('linux-x86_64') or platform.startswith('linux-ia64'):
                        suffix = '-64'
                    else:
                        suffix = '-32'
                linkbase = '%s.so%s' % (linkbase, suffix)
            symlink_from = join(self.filedir, linkbase)
        else:
            symlink_from = linkname

        #we don't create a symlink to itself.
        #otherwise their is a bug if link_target_override if the same as the destination of 

        if not link_target_override or os.path.abspath(link_target_override)!=os.path.abspath(symlink_from):
            # Create symbolic link.
            toolkit.symlink(symlink_to, symlink_from, True, True)

        # Restore original working directory.
        os.chdir(backup_cwd)

    def compiler_options(self):
        """returns the compilation options to give on the compiler command
           line when compiling this .cc file
        """
        fname = join(self.filedir,'.'+self.filebase+'.override_compile_options')
        options_override = {}
        if os.path.isfile(fname):
            f = open(fname,'r')
            options_override = eval(f.read())
            f.close()

        compileroptions = ""

        if force_32bits:
            compileroptions += ' -m32'

        for opt in options:
            optdef = pymake_options_defs[opt]
            if options_override.has_key(opt):
                compileroptions += ' ' + options_override[opt]
            else:
                compileroptions += ' ' + optdef.compileroptions
                compileroptions += ''.join([' -D'+d for d in optdef.cpp_definitions])

        optlib_compopt = self.get_optional_libraries_compileroptions();

        compileroptions = compileflags + ''.join([' -D'+d for d in cpp_definitions]) + compileroptions + ' ' + string.join(optlib_compopt, ' ')

        includedirs = [] + sourcedirs + mandatory_includedirs + self.get_optional_libraries_includedirs()
        if includedirs:
            joined_includedirs = string.join(includedirs,' -I')
            # Make sure we have no '\' character, which Cygwin / Windows does
            # not always handle properly in paths.
            joined_includedirs = joined_includedirs.replace("\\", "/")
            compileroptions += ' -I' + joined_includedirs

        return compileroptions

    def compile_command(self, nice_value=0):
        """returns the command line necessary to compile this .cc file"""

        compiler = default_compiler

        for opt in options:
            optdef = pymake_options_defs[opt]
            if optdef.compiler:
                compiler = optdef.compiler


        if platform == 'win32' or nice_value == 0:
            nice = ''
        else:
            nice = nice_command + str(nice_value) + ' '

        command = nice + compiler + ' ' + self.compiler_options() + \
                  ' -o ' + self.corresponding_ofile + ' -c ' + self.filepath

        return command

    def get_rel_dir_file(self):
        if self.filedir.startswith(os.getcwd()):
            return ["", self.filebase + self.fileext]
        includedirs = [] + mandatory_includedirs + self.get_optional_libraries_includedirs()
        for inc in includedirs:
            if self.filedir.startswith(inc):
                rel_dir = self.filedir[len(inc)+1:]
                return [rel_dir, self.filebase + self.fileext]
        print "Unable to find where the file is include : " + self.filepath
        return []



    def launch_compilation(self, hostname, nice_value):
        """Launches the compilation of this .cc file on given host using rsh
        (or ssh if the -ssh option is given), with given nice value.
        The call returns the pid of the launched process.
        This will create a field 'launched', which is a Popen3 object,
        and 'hostname' to remember on which host it has been launched.
        """

        # Remove .o file if it exists
        try:
            os.remove(self.corresponding_ofile)
        except OSError:
            pass

        # We do an 'echo $?'  at the end because rsh doesn't transmit the status byte correctly.
        quotedcommand = "'cd " + self.filedir + "; " + self.compile_command(nice_value) + "; echo $?'"
        self.hostname = hostname
        if hostname=='localhost' or hostname==myhostname:
            if platform=='win32':
                self.launched = popen3('sh -c ' + quotedcommand, -1, 't')
            else:
                self.shcommand = 'sh -c ' + quotedcommand
                self.launched = Popen3(self.shcommand, 1, 10000)
        else:
            self.shcommand = rshcommand + ' ' + self.hostname + ' ' + quotedcommand
            self.launched = Popen3(self.shcommand, 1, 10000)
        if verbose>=2:
            print '[ LAUNCHED',self.filebase+self.fileext,'on',self.hostname,']'

        if verbose>=3:
            print quotedcommand

    def update_compilation_messages(self):
        """This method should be called whenever you sense activity on the self.launched streams,
        it will fill errormsgs and warningmsgs"""
        if platform=='win32':
            self.errormsgs.extend(self.launched[0].readlines())
            self.warningmsgs.extend(self.launched[2].readlines())
        else:
            self.errormsgs.extend(self.launched.fromchild.readlines())
            self.warningmsgs.extend(self.launched.childerr.readlines())

    def finished_compilation(self):
        """You should call this when the compilation process has just finished
        This will print the warnings and error messages of the compilation process, create the
        compilation_status field to the appropriate value, and delete the launched field,
        freeing the ressources used by the pipe.
        """
        if verbose>=2:
            print '[ FINISHED',self.filebase+self.fileext,'on',self.hostname,']'

        warningmsgs=''
        errormsgs = ''
        if platform=='win32':
            warningmsgs = self.launched[2].readlines() # print warnings
            errormsgs = self.launched[0].readlines()
        else:
            warningmsgs = self.launched.childerr.readlines() # print warnings
            errormsgs = self.launched.fromchild.readlines()

        if errormsgs:
            self.compilation_status = int(errormsgs[-1])  # last line was an echo $? (because rsh doesn't transmit the status byte correctly)
            if verbose>=4:
                print 'RETURN STATUS: ', self.compilation_status
            del errormsgs[-1]

        msg = warningmsgs + errormsgs
        self.remove_hostname = False

        # Somewhat dumb heuristic to figure out why compilation failed.
        #
        # If the host does not exist or refuses ssh connection,
        # no errormsgs is returned at all, and the first line of
        # warningmsgs begins by 'ssh: '.
        #
        # You might want to add other special cases of warningmsgs here.
        #
        # If it failed because of an out of memory error: for out of
        # memory error, exit code will be non-zero, but output on
        # stdout/stderr will be only a line or so ("cc1plus: out of
        # memory allocating...").
        #
        # Use the fact that for an out or memory error, there will be
        # no line number (like "foo.cc:42: syntax error") in the error
        # message to distinguish that case from the case of (say) a
        # single error message for a syntax error, etc.
        if not hasattr(self,"compilation_status"):
            if warningmsgs:
                if warningmsgs[0].startswith('ssh: ') \
                        or warningmsgs[0].startswith('Connection closed by') \
                        or warningmsgs[0].startswith('@@@@@@@'):
                    # The hostname has a problem, so we remove it from the list
                    # and retry on another machine
                    self.remove_hostname = True
                    self.retry_compilation = True
                elif warningmsgs[0].startswith('ssh_exchange_identification: '):
                    # It happens sometimes when logging multiple times onto the
                    # same machine. No need to remove it from the list, we will
                    # try again.
                    self.retry_compilation = True
                elif warningmsgs[0].startswith('Could not chdir to home directory '):
                    #this happen when the /tmp folder is full
                    self.remove_hostname = True
                    self.retry_compilation = True
                else:
                    # Warning messages were uninformative, abort
                    self.compilation_status = -2
                    self.retry_compilation = False
            else:
                # There was an undefined problem,
                # we consider the compilation has failed
                self.compilation_status = -2 # should I put another value?
                self.retry_compilation = False
        elif self.compilation_status != 0 and 1 <= len(msg) <= 2 and \
               not re.findall(r':\d+:', ''.join(msg)):
            self.retry_compilation = True
        else:
            self.retry_compilation = False

        if msg and verbose == 1 and self.hostname!="localhost":
            print "On host:",self.hostname
        if msg:
            print self.filebase+self.fileext,':', string.join(msg,'')

        if platform!='win32':
            try:
                self.launched.wait() # we don't like defunct and zombies around, do we?
            except OSError:
                pass # don't know why, but sometimes i get a OSError: [Errno 10] No child processes

        # don't know if this is useful, but anyway...
        if platform=='win32':
            self.launched[1].close()
            self.launched[0].close()
            self.launched[2].close()
        else:
            self.launched.tochild.close()
            self.launched.fromchild.close()
            self.launched.childerr.close()
        del self.launched # free up everything

    def failed_ccfiles_to_link(self):
        """returns a list of names of the ccfiles_to_link that we tried to recompile but failed"""
        failures = []
        for ccfile in self.get_ccfiles_to_link():
            if hasattr(ccfile,"compilation_status"):
                if ccfile.compilation_status!=0:
                    failures.append(ccfile.filepath)
        return failures

    def dotid(self):
        """returns a string suitable as an ID for the dot graph language"""
        return self.filebase+'_'+self.fileext[1:]

    def filename(self):
        """returns the filename (without the directory part) of the current node"""
        return self.filebase+self.fileext

    def print_dependency_path(self, dep, visited_files, type='link'):
        if self in visited_files:
            return False
        visited_files.append(self)

        current_filename = self.filebase+self.fileext
        # print 'Checking '+current_filename+' for '+dep
        if current_filename==dep:
            print '  '+self.filepath
            return True
        elif dep in [ lib.name for lib in  self.triggered_libraries ]:
            print '  LIBRARY '+dep
            return True
        else:
            if type=='link' and not self.is_ccfile and self.corresponding_ccfile and self.corresponding_ccfile.print_dependency_path(dep, visited_files, type):
                print '  '+self.filepath
                return True
            for hfile in self.includes_from_sourcedirs:
                if hfile.print_dependency_path(dep, visited_files, type):
                    print '  '+self.filepath
                    return True
            return False

    def build_dependency_graph(self, dotfile, visited_files, type='link'):
        if self not in visited_files:
            visited_files.append(self)

            if self.is_ccfile:
                dotfile.write(self.dotid()+'[shape="box",label="'+self.filename()+'",fontsize=10,height=0.2,width=0.4,fontname="Helvetica",color="darkgreen",style="filled",fontcolor="white"];\n')
            else: # it's a .h file
                dotfile.write(self.dotid()+'[shape="box",label="'+self.filename()+'",fontsize=10,height=0.2,width=0.4,fontname="Helvetica",color="blue4",style="filled",fontcolor="white"];\n')
                if type=='link' and self.corresponding_ccfile:
                    self.corresponding_ccfile.build_dependency_graph(dotfile, visited_files, type)
                    dotfile.write(self.dotid()+' -> '+self.corresponding_ccfile.dotid()+' [dir=none,color="darkgreen",fontsize=10,style="dashed",fontname="Helvetica"];\n')

            for include in self.includes_from_sourcedirs:
                include.build_dependency_graph(dotfile, visited_files, type)
                dotfile.write(self.dotid()+' -> '+include.dotid()+' [dir=forward,color="blue4",fontsize=10,style="solid",fontname="Helvetica"];\n')

            for lib in self.triggered_libraries:
                if lib not in visited_files:
                    visited_files.append(lib)
                    dotfile.write(lib.name+'[shape="box",label="'+lib.name+'",fontsize=10,height=0.2,width=0.4,fontname="Helvetica",color="red3",style="filled",fontcolor="white"];\n')
                dotfile.write(self.dotid()+' -> '+lib.name+' [dir=forward,color="red3",fontsize=10,style="solid",fontname="Helvetica"];\n')
                print self.filename(),'->',lib.name


    def save_dependency_graph(self, dotfilename, type='link'):
        """Will save the dependency graph originating from this node into the dotfilename file
        This file should end in .dot The dot program can be used to generate a postscript file from it.
        The dependency graph considers include dependencies, as well as linkage dependencies,
        and triggered optional libraries.
        This allows one to see why all the files are compiled and linked together by pymake.
        """
        dotfile = open(dotfilename,'w')
        dotfile.write("""
digraph G
{
  edge [fontname="Helvetica",fontsize=10,labelfontname="Helvetica",labelfontsize=10];
  node [fontname="Helvetica",fontsize=10,shape=record];
""")
        visited_files = []
        self.build_dependency_graph(dotfile, visited_files, type)
        dotfile.write("}\n")
        dotfile.close()

    def get_object_files_to_link(self):
        """Return the list of object files involved in the link command.
        These objects may be dummy objects (links to .o files) if the
        'symlinkobjs' option is used, in which case they are created in this
        function.
        """
        result = []
        objs_count = 0
        for ccfile in self.get_ccfiles_to_link():
            if symlinkobjs:
                dummy_obj_file = '%d.o' % objs_count;
                if os.access(dummy_obj_file, os.F_OK):
                    # First remove an old object file.
                    os.remove(dummy_obj_file)
                # Note: os.symlink is not implemented on all platforms.
                # TODO Could use toolkit.symlink instead, but we really need
                # a true link and not a copy (and under Windows, toolkit.symlink
                # actually does a copy).
                symlink_command = 'ln -s %s %s' % (ccfile.corresponding_ofile,
                                                   dummy_obj_file)
                #print 'LINK: ' + symlink_command
                os.system(symlink_command)
                objs_count += 1
                result.append(dummy_obj_file)
            else:
                result.append(ccfile.corresponding_ofile)
        return result

    def link_command(self):
        """returns the command for making executable target from the .o files in objsfilelist"""
        self.objsfilelist = self.get_object_files_to_link()
        if local_ofiles:
            self.objsfilelist= map(local_filepath, self.objsfilelist)
        linker = default_linker
        linkeroptions = ""
        so_options = get_so_options()
        if force_32bits:
            linkeroptions = linkeroptions + ' -m32'
        for opt in options:
            optdef = pymake_options_defs[opt]
            if optdef.linkeroptions:
                linkeroptions = linkeroptions + ' ' + optdef.linkeroptions
            if optdef.linker:
                linker = optdef.linker

        command = linker + ' ' + string.join(self.objsfilelist,' ') + so_options + ' -o ' + self.corresponding_output + ' ' + self.get_optional_libraries_linkeroptions() + ' ' + linkeroptions + ' ' + linkeroptions_tail

        return command

    def clean_objlinks(self, linkslist):
        """Remove links created when the symlinkobjs option is used"""
        # Note that under Windows, these links' true filenames may end with
        # the '.lnk' extension.
        if symlinkobjs:
            for objfile in linkslist:
                objfile_lnk = objfile + '.lnk'
                if os.access(objfile, os.F_OK):
                    os.remove(objfile)
                if os.access(objfile_lnk, os.F_OK):
                    remove_cmd = 'rm %s' % (objfile_lnk)
                    os.system(remove_cmd)

    def launch_linking(self):
        corresponding_output = self.corresponding_output
        new_corresponding_output = self.corresponding_output+".new"
        if local_ofiles:
            self.corresponding_output = local_filepath(new_corresponding_output)
            mkdirs_public(os.path.dirname(self.corresponding_output))
        else:
            self.corresponding_output = new_corresponding_output
        try:
            os.remove(self.corresponding_output)
        except OSError:
            pass
        command = self.link_command()
        if verbose>=3:
            print command

        # The 'WEXITSTATUS' function is only available under Unix.
        # Under Windows, the value returned by 'os.system' is meaningless,
        # according to the Python documentation, but it is all we have
        # access to, apparently.
        if platform != 'win32':
            link_exit_code = os.WEXITSTATUS(os.system(command))
        else:
            link_exit_code = os.system(command)

        self.clean_objlinks(self.objsfilelist)

        if not os.path.exists(self.corresponding_output):
            print "Link command failed to create file " + self.corresponding_output
        else:
            try:
                os.remove(corresponding_output)
            except OSError:
                pass

            if local_ofiles:
                copyfile_verbose(self.corresponding_output, new_corresponding_output)

            os.rename(new_corresponding_output, corresponding_output)

            if verbose>=2:
                print "Successfully created " + new_corresponding_output + " and renamed it to " + corresponding_output

        self.corresponding_output = corresponding_output
        return link_exit_code

    def dll_commands(self):
        """returns the command for making dll target from the .o files in objsfilelist and the corresponding .def file"""
        self.objsfilelist = self.get_object_files_to_link()

        opt_linkeroptions = self.get_optional_libraries_linkeroptions()

        # Note: the few lines below are copied from 'link_command', which is a
        # bit (if not a lot) ugly.
        for opt in options:
            optdef = pymake_options_defs[opt]
            if optdef.linkeroptions:
                opt_linkeroptions += ' ' + optdef.linkeroptions

        # List of commands to run to generate the dll.
        list_of_commands = []

        dll_wrapper = default_wrapper

        if relocatable_dll:
            # These three steps of relocatable DLL creation were taken from
            # Colin Peter's tutorial available here:
            # http://www.emmestech.com/colin_peters_tutorial/dll/make.html
            out_dir  = os.path.dirname(self.corresponding_output)
            junk_tmp = os.path.join(out_dir, 'junk.tmp')
            base_tmp = os.path.join(out_dir, 'base.tmp')
            temp_exp = os.path.join(out_dir, 'temp.exp')
            base_command = 'g++ -mdll -o ' + junk_tmp + ' '\
                    + '-Wl,--base-file,' + base_tmp + ' '  \
                    + string.join(self.objsfilelist) + ' ' \
                    + opt_linkeroptions
            if no_cygwin:
                base_command += ' -mno-cygwin'
            list_of_commands.append(base_command)

            dll_name = os.path.basename(self.corresponding_output)
            wrap_command = dll_wrapper + ' ' \
                    + '--dllname ' + dll_name + ' ' \
                    + '--base-file ' + base_tmp + ' ' \
                    + '--output-exp ' + temp_exp + ' ' \
                    + '--def ' + self.corresponding_def_file
            list_of_commands.append(wrap_command)

            final_command = 'g++ -mdll -o ' + self.corresponding_output + ' '\
                    + string.join(self.objsfilelist) + ' ' \
                    + '-Wl,' + temp_exp + ' ' \
                    + opt_linkeroptions
            if no_cygwin:
                final_command += ' -mno-cygwin'
            list_of_commands.append(final_command)

            list_of_commands.append('rm -f ' + junk_tmp)
            list_of_commands.append('rm -f ' + base_tmp)
            list_of_commands.append('rm -f ' + temp_exp)

        else:
            linkeroptions = ""
            if self.corresponding_def_file:
                linkeroptions = '--def='+self.corresponding_def_file
                linkeroptions = linkeroptions + ' ' + dllwrap_basic_options

                wrap_command = dll_wrapper + ' ' + linkeroptions + ' -o ' + self.corresponding_output + ' ' + string.join(self.objsfilelist,' ') + ' ' + opt_linkeroptions
                list_of_commands.append(wrap_command)

        return list_of_commands

    def launch_dll_wrapping(self):
        last_failed_exit_code = 0
        list_of_commands = self.dll_commands()

        for command in list_of_commands:
            print command
            if sys.platform == 'win32':
                # os.WEXITSTATUS is not implemented under Windows.
                e = os.system(command)
            else:
                e = os.WEXITSTATUS(os.system(command))
            if e != 0:
                last_failed_exit_code = e

        # Clean potential links to object files.
        self.clean_objlinks(self.objsfilelist)
        return last_failed_exit_code


#####  Main function  #########################################################

def main( args ):

    ######## Initialization of variables
    #
    # We declare "global" a lot of variables, so that when we execute
    # the configuration file (.pymake/config), it will have read and
    # write access to these variables.
    #
    # TODO: reduce this number, or find another way?
    # maybe we could play with dictionaries?

    # Variables that are usually modified in the config file
    global platform, target_platform, homedir, myhostname, \
            useqt, qtdir, \
            default_compiler, default_linker, \
            sourcedirs, mandatory_includedirs, linkeroptions_tail, \
            options_choices, nprocesses_on_localhost, rshcommand, \
            compileflags, cpp_variables, cpp_definitions, \
            objspolicy, objsdir, \
            default_wrapper, dllwrap_basic_options, \
            nice_command, default_nice_value, verbose

    # Variables that can be useful to have read access to in the config file
    global optionargs, otherargs, linkname, link_target_override, \
            create_dll, relocatable_dll, no_cygwin, force_32bits, create_so, \
            create_pyso, link_32_64, \
            static_linking, force_recompilation, force_link, \
            local_compilation, symlinkobjs, temp_objs, distribute, vcproj, \
            local_ofiles, local_ofiles_base_path

    # Variables that wouldn't need to be global
    # TODO: fix it
    global cpp_exts, h_exts, \
            undefined_cpp_vars, defined_cpp_vars, cpp_vars_values, \
            options

    # initialize a few variables from the environment
    platform = get_platform()
    target_platform = platform # for possible cross-compilation
    homedir = get_homedir()
    myhostname = get_hostname()

    # Filetype extensions
    cpp_exts = ['.cc','.c','.C','.cpp','.CC', '.cxx']
    h_exts = ['.h','.H','.hpp']

    # QT specific stuff. If this is true, then qt's include dir and
    # library are added for compilation and linkage
    useqt = 0
    qtdir=''

    # initialize a few variables to their default values...
    # these may be overridden by the config file
    default_compiler = 'g++'
    default_linker = 'g++'
    sourcedirs = []
    mandatory_includedirs = []
    linkeroptions_tail = '-lstdc++ -lm'
    options_choices = []
    nprocesses_on_localhost = 1
    rshcommand = 'rsh '
    compileflags = ''
    # List of all C preprocessor variables possibly initialized at compilation
    # time, and that are not defined or redefined in the code
    cpp_variables = []
    # C preprocessor definitions
    cpp_definitions = []

    # Default value, to be used in -dependency mode
    undefined_cpp_vars = []
    defined_cpp_vars = []
    cpp_vars_values = {}

    #objspolicy=1 -> for dir/truc.cc object file in   dir/platform_opt/truc.o (objects file are in a directory corresponding to the cc file directory)
    #objspolicy=2 -> for dir/truc.cc object file in   objsdir/platform_opt/truc.o (all objects file are in the same directory)
    objspolicy=1
    objsdir=''

    # some variables used when creating a dll...
    default_wrapper = 'dllwrap'
    dllwrap_basic_options = '--driver-name=c++ --add-stdcall-alias'

    # nice default command and value
    nice_command = 'env nice -n'
    default_nice_value = 10
    verbose=2

    ######## Regular pymake processing

    ######  Processing of the arguments
    if len(args)==0:
        printshortusage()
        sys.exit(100)

    ####  Storing arguments
    optionargs = []  # will contain all the -... options, with the leading '-' removed
    otherargs = [] # will contain all other arguments to pymake (should be file or directory names)

    i=0
    linkname = ''
    link_target_override = None

    env_options=os.getenv('PYMAKE_OPTIONS')
    option_to_parse=[]
    if env_options:
        option_to_parse=env_options.split()
    option_to_parse+=args
    
    while i < len(option_to_parse):
        if option_to_parse[i] == '-o':
            linkname = option_to_parse[i+1]
            i = i + 1
        elif option_to_parse[i] == '-link-target':
            link_target_override = option_to_parse[i+1]
            i = i + 1

        elif option_to_parse[i][0]=='-':
            optionargs.append(option_to_parse[i][1:])

        else:
            otherargs.append(option_to_parse[i])
        i = i + 1
    del option_to_parse, env_options
    del i # We don't need it anymore and it might be confusing
    

    ####  Checking optionargs to know which task to perform
    # I do multiple for on optionarfs, as their is a bug that make that not all
    # elements of optionargs are computed
    for option in optionargs:
        if option[0] == 'v':
            remove_verbosity_option = True
            if option == 'v' or option == 'v1' or option == 'v0':
                verbose = 1
            elif option == 'v'*2 or option == 'v2':
                verbose = 2
            elif option == 'v'*3 or option == 'v3':
                verbose = 3
            elif option[:4] == 'v'*4 or option == 'v4':
                verbose = 4
            else:
                # Not a verbosity option, just an option starting with a 'v'.
                remove_verbosity_option = False
            if remove_verbosity_option:
                optionargs.remove(option)
    if verbose > 1:
        printversion()
        print '*** Current platform is: ' + platform
        print


    ##  Options specifying the type of compiled file to produce
    # do we want to create a dll instead of an executable file
    if 'dll' in optionargs:
        create_dll = 1
        optionargs.remove('dll')
    else:
        create_dll = 0

    if 'dllno-cygwin' in optionargs:
        create_dll = 1
        relocatable_dll = 1
        optionargs.remove('dllno-cygwin')
        default_wrapper = 'dlltool'
        dllwrap_basic_options = ''
        if 'g++no-cygwin' not in optionargs:
            optionargs.append('g++no-cygwin')
    else:
        relocatable_dll = 0

    # Set to '1' when using the '-mno-cygwin' option (in order to avoid dependency
    # to the Cygwin DLL).
    # TODO: remove that, pymake should not assume the name of options
    # defined in configuration file!
    if 'g++no-cygwin' in optionargs:
        no_cygwin = 1
    else:
        no_cygwin = 0

    if 'm32' in optionargs:
        force_32bits = 1
        if platform.startswith('linux-x86_64') or platform.startswith('linux-ia64'):
            target_platform = 'linux-i386'
        elif platform.startswith('linux-i386'):
            pass
        else:
            print 'Warning: you probably should not be using the "-m32" option'
        optionargs.remove('m32')
    else:
        force_32bits = 0

    # do we want to create a .so instead of an executable file
    if 'so' in optionargs:
        create_so = 1
        optionargs.remove('so')
    else:
        create_so = 0

    if 'pyso' in optionargs:
        create_pyso = 1
        optionargs.remove('pyso')
    else:
        create_pyso = 0

    # add machine-dependent info to link file
    if 'l32_64' in optionargs:
        link_32_64 = 1
        optionargs.remove('l32_64')
    else:
        link_32_64 = 0

    # do we want to create a statically linked executable
    if 'static' in optionargs:
        static_linking = 1
        optionargs.remove('static')
    else:
        static_linking = 0

    # Check for incompatibilities
    if create_so + create_dll + create_pyso > 1:
        print ('Error: cannot create a DLL, Shared Object and/or Python Shared Object '
               +'at the same time.  Remove "-dll", "-so" or "-pyso" option.')
        sys.exit(100)

    if static_linking and (create_so or create_dll or create_pyso):
        print 'Incompatible command line options specified: you may specify only one of -static, -so, -dll, -pyso'
        sys.exit(100)

    ##  Options that will not affect the final compiled file
    # Use the DBI interface instead of compiling locally or directly using SSH
    dbi_mode = None
    for option in optionargs:
        if option.startswith('dbi'):
            optionargs.remove(option)
            #FIXME: is default behaviour if no dbi mode is specified?
            if len(option) > 3 and option[3]=='=':
                dbi_mode = option[4:]
            else:
                print dedent('''\
                        Syntax of \'dbi\' option is \'-dbi=<dbi_mode>\',
                        where <dbi_mode> is one of (...)''')
                #FIXME: is there a way to get a list of supported modes?

    # force recompilation of everything?
    if 'force' in optionargs:
        force_recompilation = 1
        optionargs.remove('force')
    else:
        force_recompilation = 0

    if 'link' in optionargs:
        force_link = 1;
        optionargs.remove('link')
    else:
        force_link = 0;

    # do we want to do everything locally?
    if platform == 'win32':
        local_compilation = 1 # in windows, we ALWAYS work locally
    else:
        local_compilation = 0

    if 'local_ofiles' in optionargs:
        local_ofiles = 1
        while 'local_ofiles' in optionargs: optionargs.remove('local_ofiles')
    else:
        local_ofiles = 0
    #we must use a copy of optionargs as we should not modify a list that we iterate over.
    #this cause bug if multiple local are present. In that case, we should keep the last one.
    for option in optionargs[:]:
        if option.count('local', 0, 5)==1:
            local_compilation = 1
            optionargs.remove(option)
            if (option != 'local'):
                if (option[5] != '='):
                    print 'Syntax is \'-local=<nb_proc>\''\
                          '. Read \'' + option + '\'. Will ignoring the option'
                    # Keep default value (defined in config file or above
                    # nprocesses_on_localhost=1
                else:
                    nprocesses_on_localhost=int(option[6:])
            else:
                    nprocesses_on_localhost=1
    local_ofiles_base_path= '/tmp/.pymake/local_ofiles/' # could add an option for that...

    if 'ssh' in optionargs:
        # Re-define 'rshcommand' in order to use ssh instead.
        rshcommand = 'ssh -x'
        optionargs.remove('ssh')

    if 'symlinkobjs' in optionargs:
        symlinkobjs = 1
        optionargs.remove('symlinkobjs')
    else:
        symlinkobjs = 0

    for option in optionargs:
        if option.count('tmp', 0, 3) == 1:
            if (option != 'tmp'):
                if (option[3] != '='):
                    print 'Syntax for \'-tmp\' option is \'-tmp=<directory>\', but' \
                          ' read \'' + option + '\': the default tmp directory '    \
                          'will be used'
                    objsdir = ''
                else:
                    objsdir = option[4:]
                optionargs.remove(option)
                optionargs.append('tmp')

    if 'tmp' in optionargs:
        objspolicy = 2
        temp_objs=1
        local_compilation = 1
        if (objsdir == ''):
            objsdir = '/tmp/'+get_OBJS_dir()
        optionargs.remove('tmp')
    else:
        temp_objs = 0

    ##  Special options that will not compile, but perform various operations
    if 'clean' in optionargs:
        #some people put options in their PYMAKE_OPTIONS env variable and
        #we want them to be able to do pymake -clean .
        env_options = os.getenv('PYMAKE_OPTIONS')
        if env_options:
            for i in env_options.split():
                i=i[1:]
                if i in optionargs: optionargs.remove(i)
                        
        if len(optionargs)!=1 or len(otherargs)==0:
            print 'BAD ARGUMENTS: with -clean, specify one or more directories to clean, but no other -option:', optionargs
            sys.exit(100)
        else:
            print '>> Removing the following OBJS directories: '
            for dirname in otherargs:
                os.path.walk(dirname,rmOBJS,'')
            sys.exit()

    if 'checkobj' in optionargs: # report .cc files without any corresponding .o file in OBJS
        if len(optionargs)!=1 or len(otherargs)==0:
            print 'BAD ARGUMENTS: with -checkobj, specify one or more directories to check, but no other -option'
            sys.exit(100)
        else:
            print '>> The following files do not have *any* corresponding .o in OBJS:'
            for dirname in otherargs:
                os.path.walk(abspath(dirname),reportMissingObj, '')
            sys.exit()

    if 'dependency' in optionargs:
        if 1 <= len(otherargs) <= 2:
            optionargs.remove('dependency')
            find_dependency(otherargs)
            sys.exit()
        else:
            print 'BAD ARGUMENTS: with -dependency, usage is'
            print '"pymake -dependency target [dependency]"'
            sys.exit(100)
    elif 'dependency_include' in optionargs:
        if 1 <= len(otherargs) <= 2:
            optionargs.remove('dependency_include')
            find_dependency(otherargs,'include')
            sys.exit()
        else:
            print 'BAD ARGUMENTS: with -dependency_include, usage is'
            print '"pymake -dependency_include target [dependency]"'
            sys.exit(100)

    if 'dist' in optionargs:
        distribute = 1
        force_link = 1
        force_recompilation = 1
        optionargs.remove('dist')
    else:
        distribute = 0

    if 'help' in optionargs:
        printversion()
        printusage()
        sys.exit()

    if 'getoptions' in optionargs:
        if len(otherargs)==0:
            print 'Usage of "-getoptions" is: pymake -getoptions <list of targets, files or directories>'
            sys.exit()
        for target in otherargs:

            configpath = get_config_path(target)
            execfile( configpath, globals() )
            printusage()

        sys.exit()

    if 'vcproj' in optionargs:
        vcproj = 1
        force_recompilation = 1
        optionargs.remove('vcproj')
        if 'vc++' not in optionargs:
            optionargs.append('vc++')
        if 'genericvc++' not in optionargs:
            optionargs.append('genericvc++')
    else:
        vcproj = 0

    ####  Get the list of hosts on which to compile, and nice values
    # (Will be obsoleted when the batch job interface will come out)
    nice_values = {}
    if local_compilation:
        list_of_hosts = nprocesses_on_localhost * ['localhost']
    else:
        (list_of_hosts, nice_values) = get_list_of_hosts()



    ######  The compilation and linking

    return_code = 0

    for target in otherargs:
        configpath = get_config_path(target)
        execfile( configpath, globals() )

        # remove duplicates from sourcedirs
        sourcedirs = unique(sourcedirs)

        ccfiles_to_compile = {}
        ccfiles_to_link = {}
        executables_to_link = {}

        options = getOptions(options_choices,optionargs)

        # Get the global preprocessor variables definitions.
        undefined_cpp_vars, defined_cpp_vars, cpp_vars_values = \
                get_cpp_definitions(cpp_definitions, options, cpp_variables)

        # Building name of object subdirectory
        if  objspolicy== 1:
            objsdir = join(get_OBJS_dir(), target_platform + '__')
        elif objspolicy == 2:
            objsdir = join(objsdir, target_platform + '__')
        # We append options name to the objsdir name if they modify the compiled objects file
        # Otherwise we append them to the target_name
        for opt in options:
            pyopt = pymake_options_defs[opt]
            if pyopt.in_output_dirname:
                objsdir = objsdir + '_' + opt
        if verbose>2:
            print '*** Running pymake on '+os.path.basename(target)+' using configuration file: ' + configpath
        if verbose>1:
            print '*** Running pymake on '+os.path.basename(target)+' using options: ' + string.join(map(lambda o: '-'+o if o else '', options))
            print '++++ Computing dependencies of '+target
        get_ccfiles_to_compile_and_link(target, ccfiles_to_compile, ccfiles_to_link, executables_to_link, linkname)
        if verbose>1:
            print '++++ Dependencies computed'

        if distribute:
            # We dont want to compile. We will extract the necessary file to compile
            # the target with a makefile
            distribute_source(target, ccfiles_to_compile, executables_to_link, linkname)

        elif vcproj: # We only want to generate a Visual Studio project file.
            generate_vcproj_files(target, ccfiles_to_compile, executables_to_link, linkname)

        else:
            if verbose >=4:
                print "Link files:"
                for i in ccfiles_to_link:
                    print i.filebase
                print
                print
                print "Files to compile: "
                for i in ccfiles_to_compile:
                    print i.filebase
            if len(ccfiles_to_compile)>0:
                ccf=reduce(lambda x,y: x+y.file_is_modified(),ccfiles_to_compile,0)
                l=[x for x in file_info_map.values() if x.fileext in h_exts]
                hf=reduce(lambda x,y: x+y.file_is_modified(),l,0)
                print '++++ Compiling',
                print str(len(ccfiles_to_compile))+'/'+str(len(ccfiles_to_link)),
                print 'files. '+str(ccf)+' code and '+str(hf),
                print 'headers file were modified.'
                if verbose >=4:
                    for i in ccfiles_to_compile:
                        if i.file_is_modified():
                            print i.filename(),"were modified"

            if platform=='win32':
                win32_parallel_compile(ccfiles_to_compile.keys())
            elif dbi_mode is not None:
                #TODO: use ofiles here?
                dbi_parallel_compile(ccfiles_to_compile.keys(), dbi_mode)
            else:
                ofiles_to_copy = get_ofiles_to_copy(executables_to_link.keys())
                ofiles_to_copy = [x for x in ofiles_to_copy if x not in [y.corresponding_ofile for y in ccfiles_to_compile.keys()]]
                parallel_compile(ccfiles_to_compile.keys(), list_of_hosts, nice_values,
                        ofiles_to_copy = ofiles_to_copy)

            if force_link or (executables_to_link.keys() and not create_dll):
                print '++++ Linking', string.join(map(lambda x: x.filebase, executables_to_link.keys()))
                ret = sequential_link(executables_to_link.keys(),linkname)
                if ret != 0:
                    return_code = ret
                    continue    # Move on to next file.

            if create_dll:
                print '++++ Creating DLL of', target
                ret = sequential_dll(file_info(target))
                if ret != 0:
                    return_code = ret
                    continue    # Move on to next file.

            if temp_objs:
                os.system('chmod 777 --silent '+objsdir+'/*')
                mychmod(objsdir,33279)

    if return_code != 0:
        sys.exit(return_code)

