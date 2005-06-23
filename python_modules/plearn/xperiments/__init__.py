"""xpdir 0.1

usage::
    xpdir [mode [options]] expkey*

Default mode is listdir

Supported modes are:

    1. L{duplicates}:
    2. L{listdir}:  
    3. L{mkdir}:    
    4. L{parse_mode}:
    5. L{running}:  

Type 'xpdir mode --help' for further help on a mode
"""
__version_id__ = '$Id:$'

import new, os, sys
from Xperiment import *
from plearn.utilities.moresh              import *
from plearn.utilities.ModeAndOptionParser import *

class XpdirMode( Mode ):
    def __init__( self, targets, options ):        
        if options.load is None:
            experiments = Xperiment.load_experiments( targets )
        else:
            Xperiment.load_filed_experiments( options.load )
            experiments = Xperiment.match( targets )

        # The mode's actual routine 
        self.routine( targets, options, experiments )

        if options.cache is not None:
            Xperiment.save_cache( options.cache )

class duplicates( XpdirMode ):
    def routine( self, targets, options, experiments ):
        while experiments:
            exp = experiments.pop()
            duplicates = []
            for x in experiments:
                if x == exp:
                    duplicates.append( x.path )
            if duplicates:
                print exp
                print "Duplicated by", " ".join(duplicates)
                print

class listdir( XpdirMode ):
    """List matching experiments directory. B{Default mode.}
    
    Among experiments in the current directory, this mode lists the
    experiments that matches the experiment key::

        xpdir [listdir [key[=value]]*]

    Providing the listdir mode name is optional since the listdir mode is
    the default one.

    If no key is provided, all experiments in the directory are listed and
    their settings reported. If a key is provided without value, all
    experiment having the given key in their settings are matched.
    """
    def routine( self, targets, options, experiments ):
        print "\n","\n".join([ str(x) for x in experiments ])
        print "(%d experiments)" % len(experiments)

class mkdir( XpdirMode ):
    def option_groups( cls, parser ):
        mkdir_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under mkdir mode only." )

        mkdir_options.add_option( "--move",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Should the experiments directory be moved (instead of linked) in "
                                  "the created directory."                       
                                  )
        
        mkdir_options.add_option( "--move",
                                  default = None,
                                  help    = "The name that should be given the created directory. The default "
                                  "name is built from the experiment key."                       
                                  )

        return [ mkdir_options ]
    option_groups = classmethod( option_groups )
    
    def routine( self, targets, options, experiments ):    
        reffunc = os.symlink
        if options.move:
            reffunc = lambda src,dest: os.system("mv %s %s"%(src,dest))        

        for exp in experiments:
            if exp.infos:
                if options.name is None:
                    dirname = "_".join([ "%s=%s" % (lhs, str(rhs))
                                         for (lhs, rhs) in exp.infos.iteritems() ])
                else:
                    dirname = options.name

                if not os.path.exists( dirname ):
                    os.mkdir( dirname )

                pushd( dirname )
                if not os.path.exists( exp.path ):
                    reffunc( os.path.join('..',exp.path), exp.path )
                popd()

class running( XpdirMode ):
    def routine( self, targets, options, experiments ):
        assert not targets
        for exp in experiments:
            if not exp.infos:
                print exp

class parse_mode( XpdirMode ):
    def routine( self, targets, supported_modes, default_mode ):
        if targets:
            candidate_mode = targets[0]
            for mode in supported_modes:
                if candidate_mode == mode.__name__:                
                    del targets[0]
                    return mode
        return default_mode
#            
#  Main program
#
def main( xpdir_version = lambda : '0.1' ):
    print "xpdir %s" % xpdir_version( )
    
    parser = ModeAndOptionParser( usage = ( "%prog [mode [options]] expkey*\n" +
                                            (' '*7) +"Default mode is listdir" ),
                                  version = "%prog " + xpdir_version( )   )
    
    parser.add_option( "--cache",
                       default = None,
                       help    = "The path were the parsed expdirs should be cached for further use. "
                       "NOTE: this option should be deprecated as soon as a more intelligent caching "
                       "system is developed."
                       )
    
    parser.add_option( "--load",
                       default = None,
                       help    = "The previously cached results from which the expdir should be parsed. "
                       "NOTE: this option should be deprecated as soon as a more intelligent caching "
                       "system is developed."
                       )

    #
    # Actual Launch
    #
    options, targets = parser.parse_args( default_mode_name='listdir' )
        
    parser.selected_mode( targets, options )

if __name__ == '__main__':
    main( )
