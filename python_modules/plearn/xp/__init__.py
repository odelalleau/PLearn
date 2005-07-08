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
from plearn import pyplearn
from Experiment import *
from ConfigParser import ConfigParser
from plearn.vmat.PMat import PMat
from plearn.utilities.moresh import *
from plearn.utilities.toolkit import vsystem
from plearn.utilities.ModeAndOptionParser import *

class XpMode( Mode ):
    pass

class mkdir( XpMode ):
    def __init__( self, targets, options ):
        dirname = targets[0]
        
        vsystem( 'mkdir %s'             % dirname )
        vsystem( 'mkdir %s/Experiments' % dirname )
        vsystem( 'mkdir %s/Reports'     % dirname )
        
        config = ConfigParser( )
        config.add_section( 'EXPERIMENTS' )
        config.set( 'EXPERIMENTS', 'expdir_root', 'Experiments' )
        config.set( 'EXPERIMENTS', 'report_root', 'Reports'     )

        config_path = '%s/.pyplearn' % dirname
        config_fp   = open( config_path, 'w' )
        config.write( config_fp )        
        config_fp.close()
        print '+++', config_path, 'created.'
        
        lname  = '%s/dispatch' % dirname
        import dispatch_template
        template_file = dispatch_template.__file__.replace( '.pyc', '.py' )
        vsystem( 'cp %s %s' % (template_file, lname) )
        vsystem( 'chmod u=rwx %s' % lname )
        print '+++', lname, 'created.'
        
#
#  Modes Parsing ExpKeys
#
class ExpKeyMode( XpMode ):
    def __init__( self, targets, options ):
        # MOVED IN Experiment.cache_experiments...
        # exproot = pyplearn.config.get_option( 'EXPERIMENTS', 'expdir_root' )
        # if exproot:
        #     os.chdir( exproot )
        self.routine( ExpKey( targets ), options, Experiment.match( targets ) )

class duplicates( ExpKeyMode ):
    def aliases( cls ):
        return ['dup']
    aliases = classmethod( aliases )

    def routine( self, expkey, options, experiments ):
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

class listdir( ExpKeyMode ):
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
    
    def routine( self, expkey, options, experiments ):
        if options.full or not expkey:
            print "\n","\n".join([ str(x) for x in experiments ])
        else:
            for x in experiments:
                print '\n%s' % x.toString( expkey )
            
        print "(%d experiments)" % len(experiments)

    def option_groups( cls, parser ):
        listdir_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under listdir mode only." )

        listdir_options.add_option( "--full",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Listdir's default behaviour is to restrict the printing of the metainfos to "
                                    "the key provided as target. Use this option to print all metainfos."                       
                                  )
        
        return [ listdir_options ]
    option_groups = classmethod( option_groups )

class group( ExpKeyMode ):
    def option_groups( cls, parser ):
        group_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under group mode only." )

        group_options.add_option( "--move",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Should the experiments directory be moved (instead of linked) in "
                                  "the created group directory."                       
                                  )
        
        group_options.add_option( "--name",
                                  default = None,
                                  help    = "The name that should be given the created group directory. The default "
                                  "name is built from the experiment key."                       
                                  )

        return [ group_options ]
    option_groups = classmethod( option_groups )
    
    def routine( self, expkey, options, experiments ):    
        reffunc = os.symlink
        if options.move:
            reffunc = lambda src,dest: os.system("mv %s %s"%(src,dest))        

        for exp in experiments:
            subkey = exp.getKey( expkey )
            if options.name is None:
                dirname = "_".join([ "%s=%s" % (lhs, str(rhs))
                                     for (lhs, rhs) in subkey.iteritems() ])
            else:
                dirname = options.name

            if not os.path.exists( dirname ):
                os.mkdir( dirname )

            pushd( dirname )
            if not os.path.exists( exp.path ):
                reffunc( os.path.join('..',exp.path), exp.path )
            popd( )

class report( ExpKeyMode ):
    def routine( self, expkey, options, experiments ):
        scores = { }

        xstring = lambda xp: xp.toString(expkey)
        if not expkey:
            xstring = lambda xp: xp.path

        for exp in experiments:
            global_stats = PMat( os.path.join(exp.path, 'global_stats.pmat') )
            scores[ exp.path ] = global_stats[ options.score ][0,0]
            global_stats.close()            

        if options.sort == 'best':
            sort_function = lambda xp1, xp2: cmp(scores[xp1.path], scores[xp2.path])
        elif options.sort == 'key':
            raise 'Implement keycmp(xp1, xp2, expkey)'
            sort_function = lambda xp1, xp2: cmp(xp1, xp2) or cmp(scores[xp1.path], scores[xp2.path])

        experiments.sort( sort_function )
        for exp in experiments:
            print '%s    %s = %f' % ( xstring(exp), options.score, scores[exp.path] )
            

    def option_groups( cls, parser ):
        report_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under report mode only." )

        report_options.add_option( "--score",
                                  default = 'SHARPERATIO[test.E[vsCash_strict_monthly_relative_return]]',
                                  help    = ""
                                  )

        report_options.add_option( "--sort",
                                   default = 'best',
                                   choices = [ 'best', 'key' ],
                                   help    = ""
                                   )        

        return [ report_options ]
    option_groups = classmethod( option_groups )

class running( ExpKeyMode ):
    def routine( self, expkey, options, experiments ):
        assert not expkey
        r = 0
        for exp in experiments:
            if exp.running():
                if r == 0:
                    print
                r += 1
                print exp

        print( "\n(%d experiment%s %s running)"
               % ( r, toolkit.plural(r),
                   toolkit.plural(r, 'is', 'are') )
               )
        
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
