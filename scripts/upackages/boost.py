from upackages import *

def description():
    return 'The Boost C++ library.'

def get_installed_versions():
    """Looks for relevant files in a number of standard locations
    to see if the package appears to be installed.
    Returns a list of strings containing the versions of this package that
    appear to be installed, or an empty list if none is installed."""

    versions = []
    if(locate_lib('boost_prg_exec_monitor-gcc-mt-1_31')!=''):
        versions.append('1.31')

    return versions

def get_installable_versions():
    """Returns a list of strings containing the versions that can be installed by the install call.""" 
    return ['1.31.0']

def get_dependencies(version):
    """Returns a list describing the upackages required to build and use the given version of this one.
    The returned list is made of pairs (package_name, valid_versions)
    where valid_versions is itself a list containing all the version strings
    that can be used to build this package."""
    return [ ('bjam',['3.1.3','3.1.4','3.1.7','3.1.9','3.1.10']) ]

def install(version, builddir, prefixdir):
    """Downloads and builds the package in the current directory
    Then installs it in the given prefixdir.
    On unix a 'prefixdir' is a directory that has the same organisation
    as /usr/ and is typically given in configure commands when building from
    source to instruct make install to copy the files there.
    It has the following standard sub-directories: lib include bin share src
    The builddir will typically be prefixdir+"src"+pacakgename
    One may assume that all required dependencies are maet when this is called.
    """

    if sys.platform!='linux2':
        raise Error('Installation of boost not (yet) supported on platform '+sys.platform)
        
    # locate python and version
    python_root = ''
    python_version = ''
    if os.path.isdir('/usr/lib/python2.3'):
        python_root = '/usr'
        python_version = '2.3'
    elif os.path.isdir('/usr/local/lib/python2.3'):
        python_root = '/usr/local'
        python_version = '2.3'
    if os.path.isdir('/usr/lib/python2.2'):
        python_root = '/usr'
        python_version = '2.2'
    elif os.path.isdir('/usr/local/lib/python2.2'):
        python_root = '/usr/local'
        python_version = '2.2'
    
        if python_root=='':
            raise NameError
        
    if(version=='1.31.0'):
        download_from_sourceforge('boost','boost_1_31_0.tar.gz')
        unpack('boost_1_31_0.tar.gz')
        chdir('boost_1_31_0')
        system('bjam -sTOOLS=gcc -sPYTHON_ROOT='+python_root+' -sPYTHON_VERSION='+python_version+' --prefix='+prefixdir)
    else:
        raise Error('Installation of boost version '+version+' not supported.')
        
    #download('http://unc.dl.sourceforge.net/sourceforge/boost/boost-build-2.0-m9.1.tar.bz2')
    #unpack('boost-build-2.0-m9.1.tar.bz2')

    # patch(patchdir+'/boost_patch0')
    
    # configure('--prefix='+prefixdir)
    # patch('Makefile',Makefile_patch)
    # create

