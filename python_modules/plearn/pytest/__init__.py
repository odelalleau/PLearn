
from tests                import *
from BasicStats           import BasicStats
from IntelligentDiff      import *
from ModeAndOptionParser  import *

def test_suite_dir(path=None):
    if path is None:
        return globalvars.branches.keys()

    # Otherwise, find to which branch the path belongs
    abspath = os.path.abspath(path)
    tsuites = globalvars.branches.keys()
    for branch in tsuites:
        if string.find(abspath, branch) != -1:
            return branch
    
    return None    
