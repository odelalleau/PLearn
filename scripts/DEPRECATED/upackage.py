#!/usr/bin/env python2.3

# upackage
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
__cvs_id__ = "$Id$"


import sys
import os
import string
import urllib

upackagedir = os.environ.get('UPACKAGEDIR',os.path.join(os.environ.get(os.environ.get('HOME')),'.upackage'))

def check_setup():
    """Checks setup for upackage."""
    if not os.path.isdir(upackagedir):
        mkdir(upackagedir)

    configfile = os.path.join(upackagedir,'config.py')
    if not os.path.isfile(configfile):
        f = open(configfile,'w')
        f.write("""
upackage_sources = [  
'/home/pascal/mypackages',
'upackdb://www.upackages.org/upackdb',
'ftp://ftp.upackages.org/upackages'
]
""")
        f.close()
        print "Created "+configfile
        print "You may edit it to configure the behavior of upackage or to add sources of upackages."

    # make sure we have a upackages sub-directory
    mkdir(os.path.join(upackagedir,'upackages'))

    improper_environment = False
    if upackagedir not in string.split(os.environ.get('PYTHONPATH'),':'):        
        print 'You must add '+upackagedir+' to the PYTHONPATH environment variable.'
        improper_environment = True

    platform = 'intel-linux'
    prefixpath = os.path.join(os.path.join(upackagedir,'local'),platform)
    # make sure we have a local/platform sub-directory
    mkdir(prefixpath)

    libdir = os.path.join(prefixpath,'lib')
    if libdir not in string.split(os.environ.get('LD_LIBRARY_PATH'),':'):
        print 'You must add '+libdir+' to the LD_LIBRARY_PATH environment variable.'
        improper_environment = True

    if libdir not in string.split(os.environ.get('LIBRARY_PATH'),':'):
        print 'You must add '+libdir+' to the LIBRARY_PATH environment variable.'
        improper_environment = True

    includedir = os.path.join(prefixpath,'include')
    if includedir not in string.split(os.environ.get('CPATH'),':'):
        print 'You must add '+includedir+' to the CPATH environment variable.'
        improper_environment = True

    execpath = string.split(os.environ.get('PATH'),':')
    bindir = os.path.join(prefixpath,'bin')
    if bindir not in execpath:
        print 'You must add '+bindir+' to the PATH environment variable.'
        improper_environment = True

    if improper_environment:
        print """\n
Please edit your startup file (.cshrc or .profile)
and define your environment variables correctly.
Then start a new shell and rerun upackage."""
        sys.exit()

check_setup()

# include config.py
execfile(os.path.join(upackagedir,'config.py'))



def ask_upackdb_for_package_url(upackdburl,packagename):    
    raise Error('Could not contact upackage database '+upackdburl)
    raise Error('Package '+packagename+' is not known by database '+upackdburl)

def get_upack_version(upack_url):
    """Attempts to open the given upack and reads its first line,
    which should be like: upack_version = '2004.06.18'
    If successful the obtained version string is returned.
    Otherwise some exception is raised.
    """
    f = urllib.urlopen(url)    
    firstline = f.readline()    
    tokens = string.split(firstline, " =\t\"\'")
    if len(tokens!=2) or tokens[0]!='upack_version':
        raise Error('First line of upackage '+upack_url+" appears invalid. It should be ex: upack_version = '2004.03.29' instead of "+firstline)
    return tokens[1]

def get_package(packagename):
    packagepath = os.path.join(os.path.join(upackagedir,'upackages'),packagename+'.py')

    oldversion = ''
    try: oldversion = get_upack_version(packagepath)
    except: pass
    
    for source in upackage_sources:
        if string_begins_with(source, 'upackdb:'):
            url = ''
            try: url = ask_upackdb_for_package_url(source,packagename)
            except: pass

            if url!='': # The upackage db query returned a url where to fetch the .upack file from
                newversion = ''
                try: newversion = get_upack_version(url)
                except: pass
                
                if newversion!='': # found a .upack file of which we were able to read the version
                    if oldversion='' or compare_versions(newversion,oldversion)>0:
                        download(url, packagepath)
                    break

        elif string_begins_with(source, 'ftp:') or string_begins_with(source,'http:'):
            url = source+'/'+packagename+'.upack'
            newversion = ''
            try: newversion = get_upack_version(url)
            except: pass

            if newversion!='': # found a .upack file of which we were able to read the version
                if oldversion='' or compare_versions(newversion,oldversion)>0:
                    download(url, packagepath)
                break

    exec 'import upackages.'+packagename
    exec 'package = upackages.'+packagename
    return package

def install_single_package(packagename, version, prefixdir):
    upackages.make_usr_structure(prefixdir)
    package = get_package(packagename)
    srcdir = prefixdir+'/src/'+packagename+'/'+version
    upackages.chdir(srcdir)
    package.install(version, prefixdir)

def install(packagename, version='', prefixdir=''):
    package = get_package(packagename)

    installed_versions = package.get_installed_versions()
    installable_versions = package.get_installable_versions()

    if version=='': # Let the user choose a version 
        if installed_versions!=[]:
            print 'VERSIONS OF',packagename,'THAT APPEAR TO BE CURRENTLY INSTALLED ON YOUR SYSTEM:'
            print '  '+string.join(installed_versions,'\n  ')
        print 'VERSIONS OF',packagename,'THAT ARE INSTALLABLE AS UPACKAGES:'
        print '  '+string.join(installable_versions,'\n  ')
        version = string.strip(raw_input('VERSION TO INSTALL (leave blank to exit)? '))
        print '\n\n'
        if version=='':
            sys.exit()

    if version not in installable_versions:
        print 'Version',version,'is not in the list of installable package versions.'
        sys.exit()
    # if version in installed_versions:
    #    print packagename, version, 'appears to be already installed.'
    #    sys.exit()

    if prefixdir=='': # Let the user choose a prefix
        prefixes = get_environment_prefixpath()
        print """\n
        CHOOSE THE PREFIX WHERE TO BUILD AND INSTALL THE PACKAGE
        (make sure you have write permission for that directory, switch user and rerun if needed)
        The following prefixes are listed in UPACKAGE_PREFIX_PATH:"""
        for i in range(len(prefixes)):
            print i+1,':',prefixes[i]
        i = input('Which one to use? ')
        print '\n\n'
        prefixdir=prefixes[i-1]

    dependencies = package.get_dependencies(version)
    for (depname, minimum_version) in dependencies:
        deppack = get_package(depname)
        installed_versions = deppack.get_installed_versions()
        found_ver = ''
        for ver in installed_versions:
            if upackages.version_equal_or_greater(ver, minimum_version):
                found_ver = ver
                break
        if found_ver!='':
            print 'Found dependency ', depname, found_ver, 'OK.'
        else:
            print 'MISSING DEPENDENCY: ', packagename, 'requires', depname, 'version >=',minimum_version
            print '  Versions of '+depname+' installable with upackage are:'
            print '    '+string.join(deppack.get_installable_versions(),'    \n')
            print '  Please enter the version you wish upackage to install '
            print '  (alternatively press return when you are finished installing it with your O.S. packaging system).'
            iver = string.strip(raw_input('  Version of '+depname+' to install? '))
            print '\n\n'
            if iver!='':
                install(depname, iver, prefixdir)

    install_single_package(packagename, version, prefixdir)
    print 'FINISHED INSTALLING', packagename, version


def environment_help_and_exit():
    print """
    Please edit your shell startup file
    (.cshrc or .profile or equivalent) and
    properly define your environment variables there.
    
    The following environment variables must be
    defined for upackage to work properly:

    UPACKAGE_PREFIX_PATH should contain a colon-separated list of
    base prefix directories in which you can install extra packages
    for yourself or the group of people you work with.
    c-shell ex: setenv UPACKAGE_PREFIX_PATH $HOME/usr:$GROUPHOME/usr 
    Where $GROUPHOME is some common place where you or somebody else 
    in your group can install stuff so that everybody in the group can use it.
    Note that if you work on multiple architectures/OS, you will probably
    want to set-up architecture-dependent paths
    for ex: setenv UPACKAGE_PREFIX_PATH $HOME/usr/$ARCH:$GROUPHOME/usr/$ARCH
    Where $ARCH is defined to depend on the os/architecture.

    PATH should contain (among other things) all the prefixes in
    UPACKAGE_PREFIX_PATH with /bin appended to them.

    LD_LIBRARY_PATH should contain a colon-separated list
    of directories in which dynamic libraries may be searched-for.
    (/lib and /usr/lib may be omitted from the list, but the list will
    typically contain /usr/local/lib). In particular, it should contain
    all the prefixes in UPACKAGE_PREFIX_PATH with /lib appended to them.

    LIBRARY_PATH is used to give gcc a list of directories where to locate libraries.
    It will typically be the same as LD_LIBRARY_PATH

    CPATH is used to give gcc a list of directories where to locate includes.
    It should contain all the prefixes in UPACKAGE_PREFIX_PATH with
    /include appended to them.
    """
    sys.exit()

def get_environment_prefixpath():
    """Checks if relevant environment variables are consistently defined.
    And returns the content of environment variable UPACKAGE_PREFIX_PATH
    as a list of strings (the paths of the prefix directories).
    The variables that are checked are: UPACKAGE_PREFIX_PATH,
    LD_LIBRARY_PATH, LIBRARY_PATH, CPATH
    If one of those is missing or inconsistent with UPACKAGE_PREFIX_PATH,
    then the function calls environment_help_and_exit()
    """

    upackage_prefix_path = os.environ.get('UPACKAGE_PREFIX_PATH')
    if not upackage_prefix_path:
        print 'Environment variable UPACKAGE_PREFIX_PATH not defined!!!\n'
        environment_help_and_exit()
    upackage_prefix_path = [ string.rstrip(d,'/') for d in string.split(upackage_prefix_path,':') if d!='' ]

    ld_library_path = os.environ.get('LD_LIBRARY_PATH')
    if not ld_library_path:
        print 'Environment variable LD_LIBRARY_PATH not defined!!!\n'
        environment_help_and_exit()
    ld_library_path = [ string.rstrip(d,'/') for d in string.split(ld_library_path,':') if d!='' ]
    for d in upackage_prefix_path:
        if os.path.join(d,'lib') not in ld_library_path:
            print os.path.join(d,'lib'), 'not in LD_LIBRARY_PATH' 
            environment_help_and_exit()

    library_path = os.environ.get('LIBRARY_PATH')
    if not library_path:
        print 'Environment variable LIBRARY_PATH not defined!!!\n'
        environment_help_and_exit()
    library_path = [ string.rstrip(d,'/') for d in string.split(library_path,':') if d!='' ]
    for d in upackage_prefix_path:
        if os.path.join(d,'lib') not in library_path:
            print os.path.join(d,'lib'), 'not in LIBRARY_PATH' 
            environment_help_and_exit()

    cpath = os.environ.get('CPATH')
    if not cpath:
        print 'Environment variable CPATH not defined!!!\n'
        environment_help_and_exit()
    cpath = [ string.rstrip(d,'/') for d in string.split(cpath,':') if d!='' ]
    for d in upackage_prefix_path:
        if os.path.join(d,'include') not in cpath:
            print os.path.join(d,'include'), 'not in CPATH' 
            environment_help_and_exit()

    return upackage_prefix_path
    

def help_and_exit():
    print 'Usage:', sys.argv[0], ' force_install <packagename> <version> <prefixdir>'
    print '   or:', sys.argv[0], ' install <packagename> [<version>] [<prefixdir>]'
    print '   or:', sys.argv[0], ' list'
    print '   or:', sys.argv[0], ' info <packagename>'    
    print '   or:', sys.argv[0], ' dependencies <packagename> <packageversion>'    
    sys.exit()

        
def run():
    args = sys.argv[1:]

    if len(args)==0:
        help_and_exit()

    command = args[0]

    if command=='list':
        print 'Available packages:'
        print string.join(upackages.packages,'\n')

    elif command=='force_install':
        packagename = args[1]
        version = args[2]
        prefixdir = args[3]
        install_single_package(packagename, version, prefixdir)

    elif command=='install':
        packagename = args[1]
        version = ''
        try: version = args[2]
        except: pass
        prefixdir=''
        try: prefixdir = args[3]
        except: pass
        install(packagename, version, prefixdir)

    elif command=='info':
        packagename = args[1]
        package = get_package(packagename)
        print 'UPACKAGE',packagename
        print package.description()
        print 'Versions installable with upackage:', package.get_installable_versions() 
        print 'Currently installed versions found on your system:', package.get_installed_versions()

    elif command=='dependencies':
        packagename = args[1]
        version = args[2]
        package = get_package(packagename)
        dependencies = package.get_dependencies(version)
        print 'UPACKAGE',packagename, version, ' installation depends on:'
        for depname, depver in dependencies:
            print '  ',depname,' version >=',depver
        
try:
    run()
except KeyboardInterrupt:
    print '\nABORTED due to KeyboardInterrupt.'
