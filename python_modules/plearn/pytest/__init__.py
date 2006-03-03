__version_id__ = "$Id: __init__.py 3647 2005-06-23 15:49:51Z dorionc $"
                            
import os, sys, time

from plearn.utilities import toolkit
from plearn.utilities.ModeAndOptionParser import Mode, ModeAndOptionParser, OptionGroup

# PyTest Modules
import core, modes 

__all__ = [ "main" ]
    
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
    os.environ['PYTEST_STATE'] = 'Active'
 
    parser = ModeAndOptionParser( usage = "%prog mode [options] target*",
                                  version = "%prog " + pytest_version()   )
    
    import logging
    verb_levels = [ level
                    for level in logging._levelNames
                    if isinstance(level, type('')) ]

    parser.add_option( '-v', "--verbosity",
                       choices=verb_levels, default="INFO",
                       help=""
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
    # In Python2.4: logging.basicConfig(level=logging.DEBUG, ...)
    hdlr = logging.StreamHandler()
    hdlr.setFormatter( logging.Formatter("%(message)s") )
    logging.root.addHandler(hdlr)
    logging.root.setLevel(logging._levelNames[options.verbosity])
    
    if ( hasattr( options, 'mail' )
         and options.mail is not None ):
        raise NotImplementedError
    
    
    ############################################################
    # Launching the selected mode: The main part of the program
    ############################################################
    
    ## Program name and copyrights with version number
    newline = '\n  '
    logging.info( newline.join(
        [ "%sPyTest %s"%(newline,pytest_version()),
          "(c) 2004-2006, Christian Dorion",
          "This is free software distributed under a BSD type license.",
          "Report problems to dorionc@apstat.com\n" ]) )
    
    try:
        modes.current_mode( targets, options )    
    except core.PyTestError, e: 
        if options.traceback:
            raise
        else:
            logging.critical( "%s: %s." % (e.__class__.__name__,e) )
            core.updateExitCode(10)
    except KeyboardInterrupt, kex:
        if options.traceback:
            raise
        else:            
            logging.info("Interupted by user.")
            core.updateExitCode(-1)

    logging.debug("\nQuitting PyTest.")
    logging.info('')
    sys.exit( core.getExitCode() )
