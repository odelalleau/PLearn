# upackages
#
# Copyright (C) 2004 ApSTAT Technologies Inc.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Pascal Vincent

import os
import os.path
import urllib
import string
import shutil

packages = [ f[:-3] for f in os.listdir(__path__[0]) if len(f)>3 and f[-3:]=='.py' and f[0]!='_' ]
# __all__ = packages

patchdir = os.path.join(__path__[0],'patches')

class Error(Exception):
    """Base class for exceptions in this module"""
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def ask_choice(question, possible_answers):
    """Asks the user the question and lets him choose among a list of
    possible_answers.  The (zero-baased) index of the chosen answer is
    returned."""
    print question
    i = 1
    for a in possible_answers:
        print '[',i,'] :',a
        i = i+1
    choice = -1
    while choice<=0 or choice>len(possible_answers):
        choice = input('? ')
    return choice-1

def make_usr_structure(prefixdir):
    for name in ['src', 'lib', 'doc', 'include', 'bin', 'share' ]: 
        d = os.path.join(prefixdir,name)
        if not os.path.isdir(d): os.makedirs(d)

def invoke(command):
    (child_stdin, child_stdout_and_stderr) = os.popen4(command)
    output = child_stdout_and_stderr.readlines()
    return output

def system(command):
    print "$ EXECUTING " + command
    os.system(command)

def copy(src,dest):
    print "$ COPY ", src, dest
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
        raise Error('Cannot unpack '+filename+', unsupported extension.')

def patch(patchfile):
    """Calls patch <patchfile"""
    system('patch <'+patchfile)

def download_hook(nblocks, blocksize, totalsize):
    totalnblocks = int(totalsize/blocksize)
    if nblocks%10==0:
        print 'Transferred', nblocks*blocksize, '/', totalsize, ' bytes (', int(nblocks*100/totalnblocks),'% )'
    
def download(url, filename=''):
    """Downloads the url as the given filename"""
    if filename=='':
        filename = string.split(url,'/')[-1]
    print "DOWNLOADING", filename, "FROM", url
    f = urllib.urlretrieve(url, filename, download_hook)

def choose_location_and_download(filename, url_list):
    """Downloads a file from a list of possible alternative urls.
    Asking the user which one he prefers."""
    url = url_list[0]
    if len(url_list)>1:
        choice = ask_choice('Choose location from where to download file '+filename, url_list)
        url = urk_list[choice]
    download(url,filename)

def download_from_sourceforge(project, filename):
    """Downloads the sourceforge file specified.
    This will offer a choice of sourceforge mirrors."""

    global sourceforge_mirror
    if 'sourceforge_mirror' not in globals():
        mirror_list = [
            ('ovh','(FR)'),
            ('voxel','(US)'),
            ('aleron','(US)'),
            ('puzzle','(CH)'),
            ('heanet','(IE)'),
            ('optusnet','(AU)'),
            ('umn','(US)'),
            ('unc','(US)'),
            ('belnet','(BE)') ]

        print 'Choose sourceforge mirror to download from:'
        for i in range(len(mirror_list)):
            mirror, location = mirror_list[i]
            print ' ',i+1,':',mirror,location
        i = input('Which one to use? ')
        sourceforge_mirror = mirror_list[i-1][0]

    url = 'http://'+sourceforge_mirror+'.dl.sourceforge.net/sourceforge/'+project+'/'+filename
    download(url)

def chdir(dirpath):
    """This will cd into the specified directory,
    and attempt to create all necessary directories on the way if needed.
    An exception is raised if it fails.
    """
    print "$ CD "+dirpath
    if not os.path.isdir(dirpath):
        os.makedirs(dirpath)
    os.chdir(dirpath)    
    
def locate_lib(libfilename):
    """Looks for libfilename in the following standard directories:
    /lib /usr/lib $LD_LIBRARY_PATH $LIBRARY_PATH
    If found, returns the full path of the library file.
    If not found, returns the empty string."""

    libdirs = ['/lib','/usr/lib'] + string.split(os.getenv('LD_LIBRARY_PATH',''),':') + string.split(os.getenv('LIBRARY_PATH',''),':')
    libdirs = [ d for d in libdirs if d!='' ] 
    for dirpath in libdirs:
        filepath = os.path.join(dirpath,libfilename)
        if(os.path.isfile(filepath)):
            return filepath
    return ''                                                     

def locate_include(includename):
    """Looks for that include in the following standard directories:
    /usr/inclue /usr/local/include $CPATH $C_INCLUDE_PATH $CPLUS_INCLUDE_PATH
    (The includename must be relative to one of these directories)
    If found, returns the path of the directory containing the file.
    If not found, returns the empty string."""

    includedirs = ['/usr/include','/usr/local/include'] + string.split(os.getenv('CPATH',''),':') + string.split(os.getenv('C_INCLUDE_PATH',''),':') + string.split(os.getenv('CPLUS_INCLUDE_PATH',''),':')
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

def compare_versions(a, b):
    """Compares 2 version strings a and b (of the form 1.2 or 1.2.3).
    Returns 0 if they are equal
           -1 if a<b
           +1 if a>b   """
    if a==b:
        return 0
    else:
        avec = [ int(n) for n in string.split(a,'.') ]
        bvec = [ int(n) for n in string.split(b,'.') ]
        if avec>bvec:
            return 1
        elif avec<bvec:
            return -1
        else:
            raise Error('The two version strings '+a+' and '+b+' cannot be compared.')

def remove(path):
    """removes a file, symlink, or an entire directory recursively"""
    if os.path.isdir(path):
        for name in os.listdir(path):
            remove(name)
        os.rmdir(path)
    else:
        os.remove(path)

def rename(src,dst):
    """Renames file or directory src into dst
    This currently simply calls os.rename"""
    os.rename(src,dst)

def symlink(src, dst, rename_dst=True):
    """Create a symbolic link pointing to src named dst.
    The link is created only if src exists.
    If rename_dst is True and dst exists, dst is renamed into dst.old.<num> where <num> is the first number available.
    If rename_dst is False and dst exists, dst is first removed.
    """
    if os.path.exists(src):
        if os.path.exists(dst):
            if not rename_dst:
                remove(dst)
            else:
                num = 1
                newname = dst+'.old.'+str(num)
                while os.path.exists(newname):
                    num = num+1
                    newname = dst+'.old.'+str(num)
                rename(dst,newname)
        os.symlink(src,dst)
        print "CREATED SYMBOLIC LINK: ",dst,'-->',src
    
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
