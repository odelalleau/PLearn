import os
import os.path
import urllib
import string
import shutil

packages = [ f[:-3] for f in os.listdir(__path__[0]) if len(f)>3 and f[-3:]=='.py' and f[0]!='_' ]
# __all__ = packages

patchdir = os.path.join(__path__[0],'patches')

def make_usr_structure(prefixdir):
    for name in ['src', 'lib', 'doc', 'include', 'bin', 'share' ]: 
        d = os.path.join(prefixdir,name)
        if not os.path.isdir(d): os.makedirs(d)

def invoke(command):
    (child_stdin, child_stdout_and_stderr) = os.popen4(command)
    output = child_stdout_and_stderr.readlines()
    return output

def system(command):
    print ">>> EXECUTING " + command
    os.system(command)

def copy(src,dest):
    print ">>> COPY ", src, dest
    shutil.copy(src,dest)

def ends_with(s, termination):
    n = len(termination)
    return len(s)>=n and s[-n:]==termination

def unpack(filename):
    """Recognizes and unpacks archieves with the following extensions: .tar.gz .tgz .tar.bz2 .tar .gz .bz2 .zip"""
    if ends_with(filename,'.tar.gz') or ends_with(filename,'.tgz'):
        system('tar -xzf '+filename)
    elif ends_with(filename, '.tar.bz2'):
        basename, ext = os.path.splitext(filename)
        system('bunzip2 '+filename)
        system('tar -xf '+basename)
    elif ends_with(filename,'.tar'):
        system('tar -xf '+filename)
    elif ends_with(filename,'.gz'):
        system('gunzip '+filename)
    elif ends_with(filename, '.zip'):
        system('unzip '+filename)
    else:
        raise NameError

def patch(patchfile):
    """Calls patch <patchfile"""
    system('patch <'+patchfile)

def download_hook(nblocks, blocksize, totalsize):
    totalnblocks = int(totalsize/blocksize)
    if nblocks%10==0:
        print 'Transferred', nblocks*blocksize, '/', totalsize, ' bytes (', int(nblocks*100/totalnblocks),'% )'
    
def download(url, filename=''):
    """Downloads the given url as the given filename"""
    if filename=='':
        filename = string.split(url,'/')[-1]
    print ">>> DOWNLOADING", url, "AS", filename
    f = urllib.urlretrieve(url, filename, download_hook)

def chdir(dirpath):
    """This will cd into the specified directory,
    and attempt to create all necessary directories on the way if needed.
    An exception is raised if it fails.
    """
    print ">>> CD "+dirpath
    if not os.path.isdir(dirpath):
        os.makedirs(dirpath)
    os.chdir(dirpath)    
    
def locate_lib(libname):
    """Looks for that library in the following standard directories:
    /lib /usr/lib $LD_LIBRARY_PATH $LIBRARY_PATH
    If found, returns the full path of the library file.
    If not found, returns the empty string."""

    libdirs = ['/lib','/usr/lib'] + string.split(os.getenv('LD_LIBRARY_PATH',''),':') + string.split(os.getenv('LIBRARY_PATH',''),':')
    libdirs = [ d for d in libdirs if d!='' ] 
    for dirpath in libdirs:
        filepath = os.path.join(dirpath,'lib'+libname+'.so')
        if(os.path.isfile(filepath)):
            return filepath
        filepath = os.path.join(dirpath,'lib'+libname+'.a')
        if(os.path.isfile(filepath)):
            return filepath
    return ''                                                     

def locate_include(includename):
    """Looks for that include in the following standard directories:
    /usr/inclue /usr/local/include $C_INCLUDE_PATH $CPLUS_INCLUDE_PATH
    (The includename must be relative to one of these directories)
    If found, returns the path of the directory containing the file.
    If not found, returns the empty string."""

    includedirs = ['/usr/include','/usr/local/include'] + string.split(os.getenv('C_INCLUDE_PATH',''),':') + string.split(os.getenv('CPLUS_INCLUDE_PATH',''),':')
    includedirs = [ d for d in includedirs if d!='' ] 
    for dirpath in includedirs:
        filepath = os.path.join(dirpath,includename)
        if os.path.isfile(filepath):
            return filepath
    return ''

def locate_bin(binname):
    """Looks for file binname in the following standard directories:
    /bin /usr/bin /usr/local/bin $PATH
    If found, returns the full path of the the file.
    If not found, returns the empty string."""

    bindirs = ['/bin','/usr/bin','/usr/local/bin'] + string.split(os.getenv('PATH',''),':')
    bindirs = [ d for d in bindirs if d!='' ] 
    for dirpath in bindirs:
        filepath = os.path.join(dirpath,binname)
        if os.path.isfile(filepath):
            return filepath
    return ''

def get_package(packagename):
    exec 'import upackages.'+packagename
    exec 'package = upackages.'+packagename
    return package


# def requires(packagename, required_version_list):
#     """Checks if the specified package is installed with a version that is in the given version_list.
#     If not, proposes installing it (the first version of the list).
#     The call returns the first version of the list that appears to be installed."""
#     package = get_package(packagename)
#     installed_versions = package.get_installed_versions()
#     ret_version = ''
#     for ver in required_version_list:
#         if ver in installed_versions:
#             ret_version = ver
#             break
#     if ret_version=='':
#         print 'Package '+packagename+' is required in one of the following versions: ', string.join(required_version_list,' ')
#         if installed_versions==[]:
#             print 'But it appears not to be installed.'
#         else:
#             print 'But it appears to be installed with the following versions: ', string.join(installed_versions,' ')

#         print """If you have administrative rights on your system, and a
#         corresponding package exists in your O.S. distributions packaging
#         format, you may want to install it on all relevant machines using
#         your O.S. distribution's packaging system...
#         If you do not have administrative rights or no such ready-made
#         package exists for your distribution.
