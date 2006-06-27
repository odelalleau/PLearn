#!/usr/bin/env python2.3
__version_id__ = "$Id$"

import inspect, os, string, sys, types

from optparse                       import *
from plearn.utilities               import toolkit
from plearn.utilities               import metaprog
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject

__all__ = [ 'OptionGroup', 'Mode', 'ModeAndOptionParser' ]

INDENT = 4
LBOX = 20
                           
#BETA:def option_name(option):
#BETA:    oname = '-'
#BETA:    if option._long_opts:
#BETA:        oname += "-"
#BETA:    oname += option.dest
#BETA:    return string.replace(oname, '_', '-')

class Mode( PyPLearnObject ):
    _subclasses = {}
    _subclass_filter = classmethod( lambda cls: cls.__name__ == cls.__name__.lower() )

    #
    #  Class methods
    #
    def _by_value( cls ):
        return True
    _by_value = classmethod( _by_value )

    def aliases( cls ):
        return []
    aliases = classmethod( aliases )
    
    def description( cls ):
        return toolkit.doc( cls )
    description = classmethod( description )
    
    def help( cls ):
        return toolkit.short_doc( cls )
    help = classmethod( help )

    def mode_list( cls ):
        mode_list = cls._subclasses.values()        
        mode_list.sort( lambda a,b: cmp(a.__name__, b.__name__) )
        return mode_list
    mode_list = classmethod( mode_list )
                
    def option_groups( cls, parser ):
        return []
    option_groups = classmethod( option_groups )
    
    #
    #  Instance methods
    #
    def __init__( self, targets, options ):
        PyPLearnObject.__init__( self )
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

class ModeParsingOptions( PyPLearnObject ):
    usage             = PLOption(None)
    option_list       = PLOption(None)
    # option_class      = Option
    version           = PLOption(None)
    conflict_handler  = PLOption("error")
    description       = PLOption(None)
    formatter         = PLOption(None)
    add_help_option   = PLOption(True)
    prog              = PLOption(None)    
    
class ModeAndOptionParser( OptionParser ):
    def __init__(self, **overrides):
        defaults = ModeParsingOptions()
        
        optparser_overrides = dict( defaults.iteritems() )        
        optparser_overrides.update( overrides )        

        OptionParser.__init__( self, **optparser_overrides )

        self.selected_mode   = None

        mode_list = Mode.mode_list( )
        self.supported_modes = {}
        for mode in mode_list:
            self.supported_modes[mode.__name__] = mode
            
            aliases = mode.aliases()
            for alias in aliases:
                self.supported_modes[alias] = mode
                
    def add_option(self, *args, **kwargs):
        if not hasattr(self, 'global_options'):
            self.global_options = OptionGroup( self, "Global Options",
                                               "Available under all modes." )
            self.add_option_group(self.global_options)
        self.global_options.add_option(*args, **kwargs)        

    #
    #  Help
    #
    def print_help(self):
        if self.selected_mode is None:
            self.short_help()
        else:
            self.long_help()            
        print

    def short_help(self):
        help_str = "\nSupported modes are:\n"

        mode_list = Mode.mode_list( )
        for mode in mode_list:
            aliases = mode.aliases()
            s = ''
            if aliases:
                s += ' (%s)' % ', '.join( aliases )
            lhs = '%s%s:' % (mode.__name__, s)
            
            name_box      = 15
            name          = string.ljust(lhs, name_box)

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
    
    def parse_args(self, args=None, values=None, default_mode_name=None ):
        self.parse_mode( default_mode_name )
        return OptionParser.parse_args(self, args, values)

    def parse_mode( self, default_mode_name = None ):
        # HACK
        if '--version' in sys.argv:
            return
        
        args      = sys.argv[1:]
        mode_name = None

        # No mode: default mode OR very short help.
        if len(args) == 0:
            if default_mode_name is None:
                self.print_usage()
                print("Type '%s --help' for a list of the supported modes." % self.get_prog_name())
                sys.exit(32)
            else:
                mode_name = default_mode_name

        # The mode must be the first argument passed to the program
        else:
            mode_name = args[0]
            if mode_name in ['-h', '--help']:
                self.print_usage()
                return
            if mode_name in self.supported_modes:
                sys.argv.remove( args.pop(0) )
            else:
                mode_name = default_mode_name

        # Sanity check
        if not mode_name in self.supported_modes:
            print "Error: mode '%s' not supported." % mode_name
            self.short_help()
            sys.exit(32)

        self.selected_mode = self.supported_modes[mode_name]                
        for group in self.selected_mode.option_groups(self):
            self.add_option_group( group )            
