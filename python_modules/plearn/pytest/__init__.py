__cvs_id__ = "$Id: __init__.py,v 1.16 2005/03/11 02:49:05 dorionc Exp $"
                            
### The versionning tools are now properly enabled.
import os, sys, time
import modes
from   plearn.utilities.verbosity    import *
from   ModeAndOptionParser           import ModeAndOptionParser, OptionGroup
import plearn.utilities.toolkit      as     toolkit

__all__ = [ "main"
            ]
    
########################################################################
## Functions are listed by alphabetical order

def dynamic_version_header( pytest_version ):
    # Dynamic version header
    c = lambda s: toolkit.centered_square( s, 70 ) 

    vprint( "\n%s\n%s\n%s\n"
            % ( c("PyTest -- version " + pytest_version()),
                c("(c) 2004 Christian Dorion"),
                c("Report problems to dorionc@apstat.com") ),
            1
            )

def mail():
    raise DeprecationWarning
    # Opening the senmail process
    sendmail = Popen3("sendmail -t", True)

    # "Header" of the mail
    sendmail.tochild.write("From: PyTest -compilePLL -mail\n")
    sendmail.tochild.write("Subject: PyTest -- List of files that did not compile\n")
    sendmail.tochild.write("To: " + modes.options.mail + "\n")

    # Changing the mode of the savelog_file from write to read
    savelog_file.close()
    savelog_file = open(modes.options.savelog, "r")

    # "Body" of the mail
    sendmail.tochild.write(savelog_file.read() + "\n")
    sendmail.tochild.write(".\n")

    # "Closing" the mail mode
    sendmail.tochild.close()
    savelog_file.close()
    modes.options.savelog = None # that way the file isn't closed twice
    #                      # ( see end of pytest() )

###################################################################################
## MAIN PROGRAM
def main( pytest_version ):
    os.environ['PyTest'] = 'Running'
    
    parser = ModeAndOptionParser( usage = "%prog mode [options] target*",
                                  version = "%prog " + pytest_version()   )
    
    parser.add_option( '-v', "--verbosity",
                       choices=["0", "1", "2", "3"], default="1",
                       help="Selects the level of verbosity among [0, 1, 2, 3], "
                       "1 being the default value. Level 0 is very quiet, while level 3 "
                       "is mainly intended for debug."
                       )
    
    parser.add_option("--mail", default=None,
                      help='Not supported yet.')
    
    parser.add_option( '--traceback',
                       action="store_true",
                       help="This flag triggers routines to report the traceback of "
                       "PyTestUsageError. By default, only the class's name and meesage "
                       "are reported.",
                       default = False )
    
    modes.add_supported_modes( parser )
    
    modes.options, modes.targets = parser.parse_args()
    modes.current_mode           = parser.selected_mode
    
    ############################################################
    ################## Some preprocessing ######################
    
    ## Managing the verbosity option.
    if hasattr(modes.options, 'verbosity'):        
        set_vprint( VerbosityPrint( verbosity        = modes.options.verbosity,
                                    default_priority = 0
                                    ) )
    
    if ( hasattr(modes.options, 'mail')
         and modes.options.mail is not None ):
        ## vprint.keep_output()
        raise NotImplementedError
    
    
    ############################################################
    # Launching the selected mode: The main part of the program
    ############################################################
    
    ## Program name and copyrights with version number
    dynamic_version_header(  pytest_version  )
    
##     try:
    modes.current_mode.start()    
            
##     except KeyboardInterrupt, kex:
##         print "Interupted by user."
##         sys.exit()
    
    vprint("\nQuitting PyTest.\n", 1)
    
    ## kept = vprint.close()    
    ## if kept is not None:
    ##     for k in kept:
    ##         print k
    ##     raise NotImplementedError('mail not implemented yet')
   
    
