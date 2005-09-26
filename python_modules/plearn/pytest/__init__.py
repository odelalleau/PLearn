__version_id__ = "$Id: __init__.py 3647 2005-06-23 15:49:51Z dorionc $"
                            
import os, sys, time
import modes

from PyTestCore                           import *
from plearn.utilities                     import toolkit
from plearn.utilities.verbosity           import *
from plearn.utilities.ModeAndOptionParser import Mode, ModeAndOptionParser, OptionGroup

__all__ = [ "main"
            ]
    
########################################################################
## Functions are listed by alphabetical order

def mail():
    raise DeprecationWarning
    # Opening the senmail process
    sendmail = Popen3("sendmail -t", True)

    # "Header" of the mail
    sendmail.tochild.write("From: PyTest -compilePLL -mail\n")
    sendmail.tochild.write("Subject: PyTest -- List of files that did not compile\n")
    sendmail.tochild.write("To: " + options.mail + "\n")

    # Changing the mode of the savelog_file from write to read
    savelog_file.close()
    savelog_file = open(options.savelog, "r")

    # "Body" of the mail
    sendmail.tochild.write(savelog_file.read() + "\n")
    sendmail.tochild.write(".\n")

    # "Closing" the mail mode
    sendmail.tochild.close()
    savelog_file.close()
    options.savelog = None # that way the file isn't closed twice
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
                       "PyTestError or KeyboardInterrupt. By default, "
                       "only the test's name and message are reported.",
                       default = False )
    
    options, targets   = parser.parse_args()
    modes.current_mode = parser.selected_mode
    
    ############################################################
    ################## Some preprocessing ######################
    
    ## Managing the verbosity option.
    if hasattr( options, 'verbosity' ):        
        set_vprint( VerbosityPrint( verbosity        = options.verbosity,
                                    default_priority = 0
                                    ) )
    
    if ( hasattr( options, 'mail' )
         and options.mail is not None ):
        ## vprint.keep_output()
        raise NotImplementedError
    
    
    ############################################################
    # Launching the selected mode: The main part of the program
    ############################################################
    
    ## Program name and copyrights with version number
    newline = '\n  '
    vprint(newline.join([ "%sPyTest %s"%(newline,pytest_version()),
                          "(c) 2004, 2005, Christian Dorion",
                          "This is free software distributed under a BSD type license.",
                          "Report problems to dorionc@apstat.com\n" ]), 1)

    try:
        modes.current_mode( targets, options )    
    except PyTestError, e: 
        if options.traceback:
            raise
        else:
            vprint( "%s: %s." % (e.__class__.__name__,e) )
    except KeyboardInterrupt, kex:
        if options.traceback:
            raise
        else:
            "Interupted by user."
            sys.exit()
    
    vprint("\nQuitting PyTest.\n", 1)
    
    ## kept = vprint.close()    
    ## if kept is not None:
    ##     for k in kept:
    ##         print k
    ##     raise NotImplementedError('mail not implemented yet')
   
