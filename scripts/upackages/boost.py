# upackages/boost.py
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

from upackages import *
import sys

def description():
    return 'The Boost C++ library.'

def get_installed_versions():
    """Looks for relevant files in a number of standard locations
    to see if the package appears to be installed.
    Returns a list of strings containing the versions of this package that
    appear to be installed, or an empty list if none is installed."""

    versions = []
    if(locate_lib('libboost_regex-gcc-mt-1_31.so.1.31.0')!=''):
        versions.append('1.31.0')

    return versions

def get_installable_versions():
    """Returns a list of strings containing the versions that can be installed by the install call.""" 
    return ['1.31.0']

def get_dependencies(version):
    """Returns a list describing the upackages required to build and use the given version of this one.
    The returned list is made of pairs (package_name, minimum_required_version)"""
    return [ ('bjam','3.1.10') ]

def install(version, prefixdir):
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
        # download_from_sourceforge('boost','boost_1_31_0.tar.gz')
        # unpack('boost_1_31_0.tar.gz')
        # chdir('boost_1_31_0')
        # system('bjam -sTOOLS=gcc -sPYTHON_ROOT='+python_root+' -sPYTHON_VERSION='+python_version+' --prefix='+prefixdir+' install')

        # Make symbolic links in lib/
        chdir(os.path.join(prefixdir,'lib'))
        optext = '-gcc-mt-1_31'
        for libname in [ 'date_time', 'prg_exec_monitor', 'python', 'regex', 'signals', 'test_exec_monitor', 'thread', 'unit_test_framework']:
            symlink('libboost_'+libname+optext+'.a','libboost_'+libname+'.a',False)
            symlink('libboost_'+libname+optext+'.so','libboost_'+libname+'.so',False)
            
        # Make symbolic link include/boost -> include/boost-1_31/boost
        chdir(os.path.join(prefixdir,'include'))
        symlink('boost-1_31/boost','boost')
              
    else:
        raise Error('Installation of boost version '+version+' not supported.')
        
    #download('http://unc.dl.sourceforge.net/sourceforge/boost/boost-build-2.0-m9.1.tar.bz2')
    #unpack('boost-build-2.0-m9.1.tar.bz2')

    # patch(patchdir+'/boost_patch0')
    
    # configure('--prefix='+prefixdir)
    # patch('Makefile',Makefile_patch)
    # create

