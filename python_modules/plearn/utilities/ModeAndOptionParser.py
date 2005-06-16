#!/usr/bin/env python2.3
__cvs_id__ = "$Id: ModeAndOptionParser.py,v 1.1 2005/06/16 20:11:58 dorionc Exp $"

import inspect, os, string, sys, types

from optparse                       import *
from plearn.utilities               import toolkit
from plearn.utilities               import metaprog
from plearn.pyplearn.PyPLearnObject import PythonObject

__all__ = [ 'OptionGroup', 'Mode', 'ModeAndOptionParser' ]

INDENT = 4
LBOX = 20
                           
#BETA:def option_name(option):
#BETA:    oname = '-'
#BETA:    if option._long_opts:
#BETA:        oname += "-"
#BETA:    oname += option.dest
#BETA:    return string.replace(oname, '_', '-')

class Mode( PythonObject ):
    supported_modes = {}
    class __metaclass__( PythonObject.__metaclass__ ):
        def __init__( cls, clsname, bases, dic ):
            super(cls.__metaclass__, cls).__init__(clsname, bases, dic)            
            lower = clsname.lower()
            if clsname == lower:
                cls.supported_modes[clsname] = cls

    #
    #  Class methods
    #    
    def description( cls ):
        return toolkit.doc( cls )
    description = classmethod( description )
    
    def help( cls ):
        return toolkit.short_doc( cls )
    help = classmethod( help )

    def mode_list( cls ):
        mode_list = cls.supported_modes.keys()        
        mode_list.sort( lambda a,b: cmp(a, b) )
        return mode_list
    mode_list = classmethod( mode_list )
                
    def option_groups( cls, parser ):
        return []
    option_groups = classmethod( option_groups )
    
    #
    #  Instance methods
    #
    def __init__( self, targets, options ):
        PythonObject.__init__( self )
        self.targets = targets
        self.options = options
        
    def format_help(self, indent=0):
        result = []

        name = string.strip(self.name, '#')
        lname = len(name)
        left = string.ljust( string.rjust(name, lname+indent), LBOX )

        for i,line in enumerate(self.help_lines):
            if i == 0:
                result.append(left+line)
            else:
                result.append( string.rjust(line, len(left)+len(line)) )
            result[i] += "\n"

        return "".join(result)

class ModeAndOptionParser( OptionParser ):
    class Defaults:
         usage             = None
         option_list       = None
         option_class      = Option
         version           = None
         conflict_handler  = "error"
         description       = None
         formatter         = None
         add_help_option   = True
         prog              = None
    
    def __init__(self, **overrides):
        optparser_overrides = metaprog.public_members( self.Defaults )        
        optparser_overrides.update( overrides )        
        OptionParser.__init__( self, **overrides )

        self.selected_mode   = None
        self.supported_modes = Mode.supported_modes

    def add_option(self, *args, **kwargs):
        if not hasattr(self, 'global_options'):
            self.global_options = OptionGroup( self, "Global Options",
                                               "Available under all modes." )
            self.add_option_group(self.global_options)
        self.global_options.add_option(*args, **kwargs)        

    #BETA: def add_mode(self, mode):        
    #BETA:     if not isinstance(mode, Mode):
    #BETA:         raise TypeError("Not a Mode instance: %r" % mode)
    #BETA:     self.supported_modes[mode.classname()] = mode
    #BETA:     
    #BETA:     return mode

    ##################################################################
    ### Help
    def print_help(self):
        if self.selected_mode is None:
            self.short_help()
        else:
            self.long_help()            
        print

    def short_help(self):
        help_str = "\nSupported modes are:\n"

        mode_list = Mode.mode_list( )
        for mode_name in mode_list:
            mode = self.supported_modes[mode_name]
            
            name_box      = 10
            name          = string.ljust(mode_name+':', name_box)

            help          = toolkit.boxed_string( mode.help(), 60 )

            indent        = string.ljust('', name_box+4)                
            indented_help = string.replace( help, '\n', '\n'+indent )

            help_str = ( "%s    %s%s\n" 
                         % ( help_str, name, indented_help )
                         )

        print ( "%s\nType '%s mode --help' for further help on a mode"
                % ( help_str, self.get_prog_name() )
                )

    def long_help(self):
        lhelp = ( "\n%s %s mode: %s\n\n"
                  % ( self.get_prog_name(), self.selected_mode.classname(),
                      self.selected_mode.description() )
                  )
        lhelp += self.format_option_help()
        print lhelp
    

    ### Help
    ##################################################################

    def get_prog_name(self):
        if self.prog is None:
            return os.path.basename(sys.argv[0])
        else:
            return self.prog
    
    def parse_args(self, args=None, values=None):
        self.parse_mode()
        return OptionParser.parse_args(self, args, values)

    def parse_mode(self):
        # HACK
        if '--version' in sys.argv:
            return

        if len(sys.argv) <= 1:
            self.print_usage()
            print("Type '%s --help' for a list of the supported modes." % self.get_prog_name())
            sys.exit()
            
        mode_name = sys.argv[1] 
        if mode_name in ['-h', '--help']:
            return
        sys.argv.pop(1)
        
        if not self.supported_modes.has_key(mode_name):
            print ( "Mode %s not supported. Type '%s --help' "
                    "for a list of the supported modes."
                    % (mode_name, self.get_prog_name())
                    )
            sys.exit()
        
        self.selected_mode = self.supported_modes[mode_name]                
        for group in self.selected_mode.option_groups(self):
            self.add_option_group( group )            
