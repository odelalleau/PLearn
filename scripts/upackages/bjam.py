# upackages/bjam.py
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
    return 'Boost jam is the build system used for the Boost Library'

def get_installed_versions():
    """Looks for relevant files in a number of standard locations
    to see if the package appears to be installed.
    Returns a list of strings containing the versions of this package that
    appear to be installed, or an empty list if none is installed."""
    bjampath = locate_bin('bjam')
    installed_versions = []
    if bjampath!='':
        output = invoke(bjampath + ' -v')
        installed_versions.append( string.split(output[0])[2][:-1] )
    return installed_versions

def get_installable_versions():
    """Returns a list of strings containing the versions that can be installed by the install call.""" 
    return ['3.1.10']

def get_dependencies(version):
    """Returns a list describing the upackages required to build and use the given version of this one.
    The returned list is made of pairs (package_name, minimum_required_version_string)"""
    return []

def install(version, prefixdir):
    """Downloads and builds the package in the current directory
    Then installs it in the given prefixdir.
    On unix a 'prefixdir' is a directory that has the same organisation
    as /usr/ and is typically given in configure commands when building from
    source to instruct make install to copy the files there.
    It has the following standard sub-directories: lib include bin share src
    The builddir will typically be prefixdir+"/src/"+pacakgename+"/"+version
    One may assume that all required dependencies are maet when this is called.
    """
    
    filebase = ''
    if version=='3.1.10':
        filebase = 'boost-jam-3.1.10-1'
    else:
        raise Error('Installation of bjam version '+version+' not supported.')

    if sys.platform=='linux2':
        download_from_sourceforge('boost',filebase+'-linuxx86.tgz')
        unpack(filebase+'-linuxx86.tgz')
        copy(filebase+'-linuxx86/bjam',prefixdir+'/bin')
    elif sys.platform=='darwin':
        download_from_sourceforge('boost',filebase+'-macosxppc.tgz')
        unpack(filebase+'-macosxppc.tgz')
        copy(filebase+'-macosxppc/bjam',prefixdir+'/bin')
    elif sys.platform=='win32':
        download_from_sourceforge('boost',filebase+'-ntx86.zip')
        unpack(filebase+'-ntx86.zip')
        copy(filebase+'-ntx86/bjam',prefixdir+'/bin')
    else:
        raise Error('Installation of bjam not (yet) supported on platform '+sys.platform)
    
