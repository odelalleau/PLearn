#!/usr/bin/env python2.3

import sys, os, time, string, random
from optparse import *

def cp_array(a):
    b = []
    for e in a:
        b.append(e)
    return b

def doc(obj):
    docstr = obj.__doc__
    if docstr is None:
        return ''
    return docstr

def listdirs(dirs):
    dirs_list = []
    for dirc in dirs:
        dirs_list.extend( os.listdir(dirc) )
    return dirs_list
    
def get_path(branches, fpath):
    """Return the first existing branch/fname path, for branch in branches.

    Returns None if no branch/fname path exists.
    """
    for branch in branches:
        bpath = os.path.join(branch, fpath)
        if os.path.exists(bpath):
            return bpath
    return None
    
## Pymake
def get_platform():
    platform = sys.platform
    if platform=='linux2':
        if os.uname()[4] == 'ppc':
            platform = 'linux-ppc'
        else:
            platform = 'linux-i386'
    return platform

homedir = os.environ['HOME']
platform = get_platform()
rshcommand = 'rsh '

def locatehostsfile( hosts_fname=(platform+".hosts") ):
    """returns the path for the .pymake/<platform>.hosts file"""
    directory = os.getcwd()
    while os.path.isdir(directory) and os.access(directory,os.W_OK) and directory!='/':
        fpath = os.path.join(directory,'.pymake/' + hosts_fname)
        if os.path.isfile(fpath):
            return fpath
        directory = os.path.abspath(os.path.join(directory,'..'))
        # print directory
    # nothing was found in current directory or its parents, let's look in the homedir 
    fpath = os.path.join(homedir,'.pymake/' + hosts_fname)
    if os.path.isfile(fpath):
        return fpath
    return ''

__debug_ = 0
def DEBUG(str, stop=False):
    if not debug:
        return

    global __debug_
    __debug_ = __debug_ + 1
    print("\nDEBUG" + fpformat.fix(__debug_, 0))
    
    if stop:
        raw_input(str)
    else:
        print(str)

def ERROR(*msg):
    msg = string.join( map(str, msg) )
    print "ERROR:",msg
    sys.exit()

def command_output(command):
    ## In parallel processing, it's important to differentiate
    ## between possible multiple calls to this function.
    ## The time string is not enough since two calls can be
    ## make at the same second!!!
    tmp_file = ( ".appStart_command_output_tmp_file_" +
                 time_string() +
                 "_" + str(random.random())  )

    cmd_out = string.join([command, ">", tmp_file])
    os.system(cmd_out)

    ofile = open(tmp_file, 'r')
    lines = ofile.readlines()
    ofile.close()
    os.remove(tmp_file)
    return lines

def set_typed_attr(object, attr, value, required_type):
    if not isinstance(value, required_type):
        ERROR("Expects ",attr,"of type",str(required_type),
              "(Currently",type(value), ")")
    setattr(object, attr, value)

def select_branch(dir, branches):
    br_max = -1
    branch = None
    for brdir in branches:
        br = os.path.commonprefix([brdir, dir])
        if br > br_max:
            br_max = br
            branch = brdir
    return branch
    
def short_doc(obj):
    docstr = doc(obj)
    if not docstr:
        return ''
    doc_lines = string.split(docstr, '\n')    
    return doc_lines[0]
    
__option_parser = None
def declareOptionParser(parser):
    global __option_parser
    __option_parser = parser

def plural(nb, sing='', plur='s'):
    if nb > 1:
        return plur
    return sing
    
def quote(txt):
    return "'%s'" % txt

def time_string():
    t = time.localtime()
    return ( str(t[0]) + "_" + str(t[1]) + "_" + str(t[2])
             + "_" +
             str(t[3]) + ":" + str(t[4]) + ":" + str(t[5]) )

def tostring(num, prec=2):
    return fpformat.fix(num, prec)
    
class Verbosity:

    def __init__(self, verbosity, default_priority=0):
        self.verbosity = int(verbosity)
        self.default_priority = default_priority

    def __call__(self, msg, priority=None):
        if priority is None:
            priority = self.default_priority

        if self.verbosity >= priority:
            if hasattr(self, 'file'):
                self.file.write(msg)

            if hasattr(self, 'output'):
                self.output.append( msg )
                
            print msg
            
    def add_file(self, file_name):
        if file_name:
            self.file = open(file_name, 'w')

    def close(self):
        if hasattr(self, 'file'):
            self.file.close()
        if hasattr(self, 'output'):            
            return self.output
        return None

    def keep_output(self):
        self.output = []

class WithOptions:

    PRIVATE   = '__'
    PROTECTED = '_'
    PUBLIC    = ''
    
    def __init__(self, options={}, option_type=None):
        if option_type is None:            
            self.option_type = self.PROTECTED
        else:
            self.option_type = option_type

        self.options = {}
        self.define_options( options, self.option_type )

    def classname(self):
        return self.__class__.__name__
    
    def define_options(self, options, option_type=None):
        if option_type is None:
            option_type = self.option_type
                
        for opt in options.keys():
            setattr(self, option_type+opt, options[opt])
            self.options[opt] = options[opt]

    def command_line_options(self, parser):
        option_group = OptionGroup( parser, '%s Options'%self.classname(),
                                    'These options define a %s instance'%self.classname())

        option_names = self.options.keys()
        for name in option_names: 
            option_group.add_option('--'+string.replace(name, '_', '-'),
                                    default=None)

        return option_group
    
    def get_option(self, opt_name, option_type=None):
        if option_type is None:
            option_type = self.option_type

        opt_names = self.options.keys()
        if opt_name not in opt_names:
            raise ValueError( "Try to get an invalid option: %s"
                              "(Valids: %s)" % (opt_name, str(opt_names)) )
        ## raw_input(("value:", value))
        return getattr(self, option_type+opt_name)
        
    def set_option(self, opt_name, value, option_type=None):
        if option_type is None:
            option_type = self.option_type
        
        opt_names = self.options.keys()
        if opt_name not in opt_names:
            raise ValueError( "Try to set an invalid option: %s"
                              "(Valids: %s)" % (opt_name, str(opt_names)) )
        ## raw_input(("value:", value))
        setattr(self, option_type+opt_name, value)

    def set_options(self, options, option_type=None):
        if option_type is None:
            option_type = self.option_type
            
        for opt in options.keys():
            self.set_option(opt, options[opt], option_type)

