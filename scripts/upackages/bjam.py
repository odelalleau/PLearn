from upackages import *

def requirements():
    """Returns a list of package_names required by this one."""
    return []

def is_installed():
    """Looks for relevant files in a number of standard locations
    to see if the package appears to be installed."""
    return locate_bin('bjam')!=''

def install(builddir, prefixdir):
    """Downloads and builds the package in the given builddir
    Then installs it in the given prefixdir.
    On unix a 'prefixdir' is a directory that has the same organisation
    as /usr/ and is typically given in configure commands when building from
    source to instruct make install to copy the files there.
    It has the following standard sub-directories: lib include bin share src
    The builddir will typically be prefixdir+"src"+pacakgename
    """
    
    chdir(builddir)
    download('http://voxel.dl.sourceforge.net/sourceforge/boost/boost-jam-3.1.10-1-linuxx86.tgz')
    unpack('boost-jam-3.1.10-1-linuxx86.tgz')
    copy('boost-jam-3.1.10-1-linuxx86/bjam',prefixdir+'/bin')

