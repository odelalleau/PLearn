"""xp 0.1

usage::
    xp [mode [options]] expkey*

Default mode is listdir

Supported modes are:

    1. L{duplicates}:
    2. L{listdir}:  
    3. L{group}:    
    4. L{running}:  

Type 'xp mode --help' for further help on a mode
"""
__version_id__ = '$Id$'

import new, os, sys
from Experiment import *
from plearn.utilities.moresh              import *
from plearn.utilities.ModeAndOptionParser import *

class XpMode( Mode ):
    def __init__( self, targets, options ):        
        self.routine( targets, options, Experiment.match( targets ) )

class duplicates( XpMode ):
    def aliases( cls ):
        return ['dup']
    aliases = classmethod( aliases )

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

class listdir( XpMode ):
    """List matching experiments directory. B{Default mode.}
    
    Among experiments in the current directory, this mode lists the
    experiments that matches the experiment key::

        xp [listdir [key[=value]]*]

    Providing the listdir mode name is optional since the listdir mode is
    the default one.

    If no key is provided, all experiments in the directory are listed and
    their settings reported. If a key is provided without value, all
    experiment having the given key in their settings are matched.
    """
    def aliases( cls ):
        return ['ls']
    aliases = classmethod( aliases )
    
    def routine( self, targets, options, experiments ):
        print "\n","\n".join([ str(x) for x in experiments ])
        print "(%d experiments)" % len(experiments)

class group( XpMode ):
    def option_groups( cls, parser ):
        group_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under group mode only." )

        group_options.add_option( "--move",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Should the experiments directory be moved (instead of linked) in "
                                  "the created group directory."                       
                                  )
        
        group_options.add_option( "--move",
                                  default = None,
                                  help    = "The name that should be given the created group directory. The default "
                                  "name is built from the experiment key."                       
                                  )

        return [ group_options ]
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
                    os.group( dirname )

                pushd( dirname )
                if not os.path.exists( exp.path ):
                    reffunc( os.path.join('..',exp.path), exp.path )
                popd( )

class running( XpMode ):
    def routine( self, targets, options, experiments ):
        assert not targets
        for exp in experiments:
            if not exp.infos:
                print exp

#            
#  Main program
#
def main( xp_version = lambda : '0.1' ):
    parser = ModeAndOptionParser( usage = ( "%prog [mode [options]] expkey*\n" +
                                            (' '*7) +"Default mode is listdir" ),
                                  version = "%prog " + xp_version( )   )

    #
    # Actual Launch
    #
    options, targets = parser.parse_args( default_mode_name='listdir' )

    print "xp %s" % xp_version( )    
    parser.selected_mode( targets, options )

if __name__ == '__main__':
    main( )
