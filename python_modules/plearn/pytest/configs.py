import os

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

