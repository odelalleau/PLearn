
raise DeprecationWarning("Currently in tests.py")

__all__ = [ "complete_test_name", "relative_path" ]

def complete_test_name(directory):
    return os.path.join( globalvars.pure_branches[test_suite_dir(directory)],
                         relative_path(directory) )

def relative_path(path):
    tsdir = test_suite_dir(path)
    if string.find(path, tsdir) == -1:
        raise FormatError(path + " must begin with %s" % tsdir)

    rel_path = path[len(tsdir):]
    if len(rel_path) != 0 and rel_path[0] == '/':
        rel_path = rel_path[1:]
    return  rel_path

