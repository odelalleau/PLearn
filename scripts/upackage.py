#!/usr/bin/env python

import sys
import os
import string
import upackages

def get_package(packagename):
    exec 'import upackages.'+packagename
    exec 'package = upackages.'+packagename
    return package

def install_single_package(packagename, version, prefixdir):    
    upackages.make_usr_structure(prefixdir)
    package = get_package(packagename)
    package.install(version, prefixdir+'/src/'+packagename+'/'+version, prefixdir)

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
    if version in installed_versions:
        print packagename, version, 'appears to be already installed.'
        sys.exit()

    if prefixdir=='': # Let the user choose a prefix
        prefixes = get_environment_prefixpath()
        print """\n
        CHOOSE THE PREFIX WHERE TO BUILD AND INSTALL THE PACKAGE
        (make sure you have write permission for that directory)
        The following prefixes are listed in UPACKAGE_PREFIX_PATH:"""
        for i in range(len(prefixes)):
            print i+1,':',prefixes[i]
        i = input('Which one to use? ')
        print '\n\n'
        prefixdir=prefixes[i-1]

    dependencies = package.get_dependencies(version)
    for (depname, desired_versions) in dependencies:
        deppack = get_package(depname)
        installed_versions = deppack.get_installed_versions()
        found_ver = ''
        for ver in desired_versions:
            if ver in installed_versions:
                found_ver = ver
                break
        if found_ver!='':
            print 'Found dependency ', depname, found_ver, 'OK.'
        else:
            print 'MISSING DEPENDENCY: ', packagename, 'requires', depname, '(required version:',string.join(desired_versions,' or '),')'
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
    """Checks if relevant environment variables are consistently defined"""

    upackage_prefix_path = os.getenv('UPACKAGE_PREFIX_PATH')
    if not upackage_prefix_path:
        print 'Environment variable UPACKAGE_PREFIX_PATH not defined!!!\n'
        environment_help_and_exit()
    upackage_prefix_path = [ string.rstrip(d,'/') for d in string.split(upackage_prefix_path,':') if d!='' ]

    ld_library_path = os.getenv('LD_LIBRARY_PATH')
    if not ld_library_path:
        print 'Environment variable LD_LIBRARY_PATH not defined!!!\n'
        environment_help_and_exit()
    ld_library_path = [ string.rstrip(d,'/') for d in string.split(ld_library_path,':') if d!='' ]
    for d in upackage_prefix_path:
        if os.path.join(d,'lib') not in ld_library_path:
            print os.path.join(d,'lib'), 'not in LD_LIBRARY_PATH' 
            environment_help_and_exit()

    library_path = os.getenv('LIBRARY_PATH')
    if not library_path:
        print 'Environment variable LIBRARY_PATH not defined!!!\n'
        environment_help_and_exit()
    library_path = [ string.rstrip(d,'/') for d in string.split(library_path,':') if d!='' ]
    for d in upackage_prefix_path:
        if os.path.join(d,'lib') not in library_path:
            print os.path.join(d,'lib'), 'not in LIBRARY_PATH' 
            environment_help_and_exit()

    cpath = os.getenv('CPATH')
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
        for dep in dependencies:
            print '  ',dep[0],' version',dep[1]
        

run()
