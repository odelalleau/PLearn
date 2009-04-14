import os, sys
from plearn.utilities import toolkit
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject, PyPLearnSingleton

#######  PyTestObject  ########################################################

class PyTestObject(PyPLearnSingleton):
    def _by_value(self):
        return True

#######  Exceptions  ##########################################################

def traceback(exception):
    import traceback
    return "Traceback (most recent call last):\n" + \
           ''.join(traceback.format_tb(sys.exc_info()[2])) + \
           "%s: %s"%(exception.__class__.__name__, exception)

class PyTestUsageError(Exception):
    pass

#######  Configs and Caches  ##################################################

def mail(subject, msg_content, to="dorionc@apstat.com"):
    # Opening the senmail process
    from popen2 import Popen3
    sendmail = Popen3('mail %s -s "%s"'%(to,subject), True)

    # "Body" of the mail
    sendmail.tochild.write("%s\n"%msg_content)

    # "Closing" the mail process
    sendmail.tochild.close()

def pytest_config_path():
    from plearn.utilities.ppath import plearn_configs
    return os.path.join(plearn_configs, 'pytest')


__pytest_define_msg = \
"""// This file is generated and managed automatically by PyTest.
// PLEASE DO NOT EDIT
"""
def pytest_defines():
    pytest_configs = pytest_config_path()
    if not os.path.isdir(pytest_configs):
        os.mkdir(pytest_configs)

    # Parse the define currently in pytest_defines.h
    pytest_defines = os.path.join(pytest_configs, 'defines.h')
    current_defines = []
    if os.path.isfile(pytest_defines):
        for line in file(pytest_defines, 'r'):
            if line.startswith('#define'):
                current_defines.append(line.strip())

    # Build the defines expected to be in pytest_defines.h
    pytest_state = os.getenv('PYTEST_STATE', 'Inactive')
    states = {'Active' : 1, 'Inactive' : 0}
    assert pytest_state in states.keys()
    expected_defines = [ "#define PYTEST_STATE %s" % pytest_state,
                         "#define PYTEST_ACTIVE %s" % states[pytest_state] ]

    # Compare current and expected defines
    unchanged = (len(current_defines) == len(expected_defines))
    it_current = iter(current_defines)
    it_expected = iter(expected_defines)
    while unchanged:  
        try:
            unchanged = (it_current.next() == it_expected.next())
        except StopIteration:
            break        

    # If the defines changed, modify the file
    if not unchanged:
        defines = open(pytest_defines, 'w')
        
        print >>defines, __pytest_define_msg
        for line in expected_defines:
            print >>defines, line

        defines.close()
        return "Modified"        

    return "Unchanged"

__cached_list_fname = "test_map.cfg"
def getCachedTestMap():
    cached_list_fpath = os.path.join(pytest_config_path(), __cached_list_fname)
    try:
        return eval( file(cached_list_fpath, 'r').read() )
    except IOError:
        return {}

def updateCachedTestMap(test_map):
    pytest_configs = pytest_config_path()
    if not os.path.isdir(pytest_configs):
        os.mkdir(pytest_configs)
        
    # Overwrite old file if any
    cached_list_fpath = os.path.join(pytest_configs, __cached_list_fname)
    cached_list_file = file(cached_list_fpath, 'w')
    
    name_to_dir = dict( [(name, test.directory()) for name, test in test_map.iteritems()] )
    print >>cached_list_file, name_to_dir
    cached_list_file.close()

#######  Exit Code  ###########################################################

#   0 : Tests were ran and all enabled tests passed (no skipped test)
#   1 : At least one test was skipped
#   2 : At least one test failed
#   4 : At least one program did not compile 
#  32 : PyTest usage error (wrong command line arguments, ...)
#  64 : PyTest internal error
__code_mappings = {
    ""                   : 0, # For convenience
    "PASSED"             : 0,
    "DISABLED"           : 0,
    "SKIPPED"            : 1,
    "FAILED"             : 2,
    "COMPILATION FAILED" : 4,
    "USAGE ERROR"        : 32,
    "INTERNAL ERROR"     : 64
    }

__exit_flags = set()
def exitPyTest(flag=""):
    updateExitCode(flag)    
    sys.exit( sum(__exit_flags) )

def updateExitCode(flag):
    __exit_flags.add(__code_mappings[flag])

#######  Module's Test  #######################################################

if __name__ == '__main__':
    import os
    from plearn.pytest import modes
    from plearn.utilities.ModeAndOptionParser import Mode

    def vsystem(cmd, display_cmd = ''):        
        if display_cmd == '':
            display_cmd = cmd
        print >>sys.stderr, '#\n#  %s\n#' % display_cmd
        sys.stderr.flush()
        stderr_output = toolkit.command_output(cmd, True, False)
        if stderr_output:
            print >>sys.stderr, stderr_output[0].rstrip('\n')
        print >>sys.stderr, ''
        sys.stderr.flush()
        
    if sys.platform == 'win32':
        which_pytest = toolkit.command_output('which pytest')[0].rstrip('\n')
        pytest_cmd = \
            toolkit.command_output('cygpath -w %s'%which_pytest)[0].rstrip('\n')
        pytest_cmd = 'python ' + pytest_cmd
    else:
        pytest_cmd = 'pytest'

    vsystem(pytest_cmd, 'pytest')
    vsystem('%s -h' % pytest_cmd, 'pytest -h')

    for mode in Mode.mode_list():
        mode_name = mode.__name__
        vsystem('%s %s -h' % (pytest_cmd, mode_name),'pytest %s -h' % mode_name)

