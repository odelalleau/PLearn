#!/usr/bin/env python2.3

import os, sys, types, string
import plearn.utilities.toolkit   as     toolkit

from   optparse                   import *
from   Mode                       import *

INDENT = 4
LBOX = 20

__config = None
__config_is_valid = False
def config(*args):
    global __config_is_valid
    
    if len(args) == 1:
        __config.parser.check_mode_consistency(args[0])
        __config_is_valid = True
    elif len(args) == 2:
        if args[0][0] == '-':
            __config.parser.change_default(args[0], args[1])
        else:
            __config.parser.selected_mode().set_target(args[0], args[1])
    else:
        raise Exception("BAD CONFIG FILE FORMAT")

def lbox(nb):
    global LBOX
    LBOX = nb

def load_config(parser, config_file):
    global __config
    
    __config = ConfigurationFile()

    if os.path.exists(config_file):    
        execfile(config_file)

    if not __config_is_valid:
        print ("The value provided to the config option must be a\n"
               "valid configuration file.")
        ## raw_input("sys.exit()")
                           
def option_name(option):
    oname = '-'
    if option._long_opts:
        oname += "-"
    oname += option.dest
    return string.replace(oname, '_', '-')

class ConfigurationFile:
    parser = None
    
    def __init__(self, mode = None):
        if self.parser is None:
            raise RunTimeError("Creation of a configuration file before the creation\n"
                               "of the main ModeAndOptionParser.")

        self.option_list = self.parser.option_list
        self.set_mode(mode)
        
    def pretty_comment_print(self, comment):
        group_head = string.replace( string.center("",len(comment)+5), ' ', '#' )
        print '\n',group_head
        print '##',comment
        print group_head,'\n'

    def set_mode(self, mode):
        if mode is None:
            self.loading = True
        else:
            self.mode = mode
            self.option_groups = self.mode.option_groups
            self.loading = False
        
    def write(self):

        self.pretty_comment_print("Creation of the ConfigurationFile object")
        print ( "config(%s)" % self.mode.expanded_name() )
        
        self.config_obj_name = "config"
        for option in self.option_list:
            self.write_option_config(option)

        for group in self.option_groups:
            self.pretty_comment_print(group.title)
                
            for option in group.option_list:
                self.write_option_config(option)

        self.mode.write_targets_config(self.config_obj_name)

##         self.pretty_comment_print("Internal settings: do not modify.")
##         print(" = config")

    def write_option_config(self, option):
        if not option.dest:
            return
        print "## Option help:",option.help

##        option_config = ["%s.change_default(" % self.config_obj_name]
        option_config = ["%s(" % self.config_obj_name]

        opt_name = '"' + option_name(option) + '"'
        option_config.append( opt_name )
        
        option_config.append( ", " )
        
        opt_default = option.default
        if isinstance(opt_default, types.StringType):
            opt_default = '"' + opt_default + '"'
        else:
            opt_default = str(opt_default)
        option_config.append( opt_default )
        
        option_config.append( ")\n" )
        print string.join( option_config )

class ModeAndOptionParser(OptionParser):

    def __init__(self,
                 usage=None,
                 option_list=None,
                 option_class=Option,
                 version=None,
                 conflict_handler="error",
                 description=None,
                 formatter=None,
                 add_help_option=True,
                 prog=None,
                 with_config_mode=True):
        OptionParser.__init__( self, usage, option_list, option_class,
                               version, conflict_handler, description,
                               formatter, add_help_option, prog )
        self.mode_selector    = ModeSelector(self)                    
        self.with_config_mode = with_config_mode
        
        if self.with_config_mode:
            self.configuration_settings()

    def add_mode(self, *args, **kwargs):
        """add_mode(Mode)
        add_mode(opt_str, ..., kwarg=val, ...)
        """
        self.mode_selector.add_mode(*args, **kwargs)

    def add_option(self, *args, **kwargs):
        if not hasattr(self, 'global_options'):
            self.global_options = OptionGroup( self, "Global Options",
                                               "Available under all modes." )
            self.add_option_group(self.global_options)
        self.global_options.add_option(*args, **kwargs)
        
    def change_default(self, opt_name, default):
        ## raw_input((opt_name, default))

        opt = self.get_option(opt_name)
        value = getattr(self.values, opt.dest)
        ## raw_input(value)

        if ( value == self.defaults[opt.dest] and
             default != self.defaults[opt.dest] ):
            setattr(self.values, opt.dest, default)
        self.defaults[opt.dest] = default

        ## raw_input("+++++++++++++")

    def check_mode_consistency(self, mode_to_run):
        if self.mode_selector.selected_mode_name != mode_to_run:
            raise RunTimeException("The config file provided is for mode %s"
                                   "but the current mode is %s"
                                   %(mode_to_run, self.mode_selector.selected_mode_name))

    def __configuration_file(self):
        if not self.with_config_mode:
            raise RunTimeError("Call to ModeAndOptionParser.configuration_file with\n"
                               "with_config_mode set to False")

        mode_name = self.values.mode_to_config
        if not mode_name in self.mode_selector.supported_modes.keys():
            self.print_help(True)
            sys.exit()

        config = ConfigurationFile( self.mode_selector.supported_modes[mode_name] )
        config.write()
            
    def configuration_settings(self):
        ConfigurationFile.parser = self

        config_options = OptionGroup( self, "Config mode options",
                                      "These options are only available under the config mode." )

        config_options.add_option('-m', '--mode-to-config', default=Mode.DEFAULT)
        config_options.add_option('-f', '--config-file', default=None)
        
        self.add_mode( 'config', self.__configuration_file,
                       help="Prints a config file for this application.",
                       max_targets = 0,
                       option_groups = [config_options] )

        self.add_option( '--config',
                         action="store",
                         help="Loads the config file provided within the current mode.",
#                         callback=load_config,
                         default=None )

    def launch_selected_mode(self, exit_after=False):
        if not self.mode_selector.launch((self.rargs+self.largs), self.values, exit_after):
            self.print_help(True)
            sys.exit()

    def mode_is_running(self):
        return self.mode_selector.is_running()
    
    def get_prog_name(self):
        if self.prog is None:
            return os.path.basename(sys.argv[0])
        else:
            return self.prog
    
    def parse_args(self, args=None, values=None):
        self.mode_selector.parse_mode()
        return OptionParser.parse_args(self, args, values)

    def check_values(self, values, args):
        self.args = args
        if hasattr(values, 'config') and values.config is not None:
            ## raw_input(values)
            ## raw_input("------------")
            ## raw_input(self.defaults)
            ## raw_input("------------")
            
            load_config(self, values.config)
            
            ## raw_input(values)
            ## raw_input("------------")
            ## raw_input(self.defaults)
            ## raw_input("------------")

                        
        return (values, args)

    def print_help(self, short=False):
        print
        if short:
            OptionParser.print_help(self)
        print self.mode_selector.format_help(short)
        print

    def selected_mode(self):
        return self.mode_selector.selected_mode
            
class ModeError(Exception):
    def __init__(self, msg, mode):
        set_typed_attr(self, 'mode', mode, type(Mode()))
        self.msg = msg
        
    def __str__(self):
        return self.mode.name+": %s" % self.msg

class ModeSelector:
    def __init__(self, parser):
        self.parser          = parser
        self.default         = None
        self.mode_list       = []
        self.supported_modes = {}
        
    def add_mode(self, *args, **kwargs):
        if type(args[0]) is types.StringType:
            mode = Mode( self.parser, *args, **kwargs)
        elif len(args) == 1 and not kwargs:
            mode = args[0]
            if not isinstance(mode, Mode):
                raise TypeError, "not an Mode instance: %r" % mode
        else:
            raise TypeError("invalid arguments")

        if mode.name == Mode.DEFAULT:
            self.default = mode
        else:
            self.mode_list.append(mode)

        self.supported_modes[mode.name] = mode
        
        return mode

    def parse_mode(self):
        modes = self.supported_modes.keys()
        self.selected_mode = None
        self.selected_mode_name = None

        # HACK
        if '--version' in sys.argv:
            return

        if len(sys.argv) > 1:
            candidate_mode = sys.argv[1]
            if not candidate_mode in modes:
                if not Mode.DEFAULT in modes:
                    self.parser.print_help(True)
                    sys.exit()
                else:
                    self.selected_mode_name = Mode.DEFAULT
            else:
                self.selected_mode_name = sys.argv.pop(1)

        if self.selected_mode_name in modes:
            self.selected_mode = self.supported_modes[self.selected_mode_name]
                
        self.syncronyse_option_groups()

    def syncronyse_option_groups(self):
        if self.selected_mode is None:
            return

        for group in self.selected_mode.option_groups:
            self.parser.add_option_group( group )

    def short_help(self, indent):
        result = []
        if self.default is None:
            result.append("\nSupported modes are:\n")
        else:
            result.append( "If no modes are provided:\n" )
            result.append( self.default.format_help(indent+INDENT) )
            result.append("\nOther suported modes are:\n")

        
        self.mode_list.sort( lambda a,b: cmp(a.name, b.name) )
        for mode in self.mode_list:
            result.append( mode.format_help(indent+INDENT) )

        return "".join(result)

    def long_help(self, indent):
        lhelp = ( "%s %s mode: %s\n\n"
                  % (self.parser.get_prog_name(), string.strip(self.selected_mode_name, '#'),
                     self.selected_mode.description) )
        lhelp += self.parser.format_option_help()
        return lhelp
    
    def format_help(self, short=False, indent=0):
        formatted = ''

        if short:
            formatted += self.short_help(indent)
            formatted += ( "\nType '%s mode --help' for further help on a mode"
                           % self.parser.get_prog_name() )
            if not self.default is None:
                formatted += "\n(If none, the default mode is assumed)"

        elif self.selected_mode_name == Mode.DEFAULT:
            formatted += self.short_help(indent)
            formatted += "\n\n"

        if not short:
            formatted += self.long_help(indent)

        return formatted

    def launch(self, targets, options, exit_after=False):
        if self.selected_mode is None:
            return False
        self.selected_mode(targets, options)
            
        if exit_after:
            sys.exit()

        return True

##     def is_launchable(self, targets, options, exit_after=False):
##         if self.selected_mode is None:
##             return False

##         self.targets    = targets
##         self.options    = options
##         self.exit_after = exit_after
##         return True

##     def run(self):
##         if not hasattr(self, targets):
##             raise RuntimeError("Mode.is_launchable must be called prior to Mode.launch.")
        
##         self.selected_mode(targets, options)

##         if exit_after:
##             sys.exit()

##         return True

