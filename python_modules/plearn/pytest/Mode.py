import string, types
from   threading                     import Thread
import plearn.utilities.toolkit      as     toolkit
from   plearn.utilities.FrozenObject import FrozenObject

LBOX = 20

class ModeDefaults:
    parser           = None
    name             = None
    mode_procedure   = None

    __declare_members__ = [ ( 'parser',          None),
                            ( 'name',            types.StringType),
                            ( 'mode_procedure',  types.FunctionType)
                            ]
    
    description      = None
    expected_targets = []
    max_targets      = None
    min_targets      = None
    option_groups    = []
    help             = ''
    help_lines       = None
    

class Mode(Thread, FrozenObject):
    DEFAULT = "#DEFAULT#"

    def __init__( self, *args, **overrides ):
        self._frozen = False
        Thread.__init__(self)
        self.setDaemon(True)
        
        if len(args) == 0:
            raise NotImplementedError
        
        if not callable(args[2]):
            raise ValueError("The procedure argument of the new mode is not callable.\n"
                             "Value: %s" % str(args[2]))
        overrides.update( { 'parser'         : args[0],
                            'name'           : args[1],
                            'mode_procedure' : args[2]
                            }
                          )
        FrozenObject.__init__(self, ModeDefaults, overrides)

        self.help_lines = string.split(self.help, '\n')            

    def __call__(self, targets, options):
        self.check_targets(targets)
        self.start()

    def run(self):
        self.mode_procedure()
        
    def __repr__(self):
        return str(self)

    def __str__(self):
        return string.join(["Mode",self.name])    

    def add_option_group(self, *args, **kwargs):
        # XXX lots of overlap with OptionContainer.add_option()
        if type(args[0]) is types.StringType:
            group = OptionGroup(self, *args, **kwargs)
        elif len(args) == 1 and not kwargs:
            group = args[0]
            if not isinstance(group, OptionGroup):
                raise TypeError, "not an OptionGroup instance: %r" % group
            if group.parser is not self.parser:
                raise ValueError, "invalid OptionGroup (wrong parser)"
        else:
            raise TypeError, "invalid arguments"

        if self.option_groups is None:
            self.option_groups = [group]
        else:
            self.option_groups.append(group)

        return group

    def check_targets(self,targets):
        if ( not self.min_targets is None
             and len(targets) < self.min_targets ) :
            self.parser.print_help()
            print("Mode %s takes at least %d target%s"
                  % (self.name, self.min_targets, plural(self.min_targets)) )
            sys.exit()

        if ( not self.max_targets is None
             and len(targets) > self.max_targets ) :
            self.parser.print_help()
            print("Mode %s takes at most %d target%s"
                  % (self.name, self.max_targets, plural(self.max_targets)) )
            sys.exit()

        if not self.expected_targets is None:
            targets = self.parser.args
            for i,target in enumerate(self.expected_targets):
                if i == len(targets):
                    break
                setattr(self, target, targets[i]) 

    def expanded_name(self):
        if self.name == Mode.DEFAULT:
            return "Mode.DEFAULT"
        return self.name
    
    def format_help(self, indent=0):
        result = []

        name = string.strip(self.name, '#')
        lname = len(name)
        left = string.ljust( string.rjust(name, lname+indent),
                             LBOX )

        for i,line in enumerate(self.help_lines):
            if i == 0:
                result.append(left+line)
            else:
                result.append( string.rjust(line, len(left)+len(line)) )
            result[i] += "\n"

        return "".join(result)

    def set_target(self, name, value):
        if name not in self.expected_targets:
            raise Exception("Invalid target %s (Valids: %s)"
                            % (name, str(self.expected_targets)) )
        setattr(self, name, value)
        
    def write_targets_config(self, obj_name):
        if not self.expected_targets is None:
            for target in self.expected_targets:
##                 target_config = [ "%s.set_target("
##                                   % obj_name ]
                target_config = [ "%s("
                                  % obj_name ]

                target_name = '"' + target + '"'
                target_config.append( target_name )
    
                target_config.append( ", <VALUE> )\n" )
                
                print string.join( target_config )
