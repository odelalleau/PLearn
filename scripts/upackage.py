#!/usr/bin/env python

import sys
import string
import upackages

def help_and_exit():
    print 'Usage:', sys.argv[0], ' install <packagename> <prefixdir>'
    print 'Available packages are:'
    print string.join(upackages.packages,'\n')
    sys.exit()

def get_package(packagename):
    exec 'import upackages.'+packagename
    exec 'package = upackages.'+packagename
    return package

def append_unique(l,elem):
    if elem not in l: l.append(elem)

def append_all_requirements(packagename, requirements):
    if packagename not in requirements:
        requirements.append(packagename)
    for pname in get_package(packagename).requirements():
        append_all_requirements(pname, requirements)

def get_all_requirements(packagename):
    requirements = []
    append_all_requirements(packagename, requirements)
    return requirements

def install_single_package(packagename, prefixdir):    
    upackages.make_usr_structure(prefixdir)
    package = get_package(packagename)
    package.install(prefixdir+'/src/'+packagename, prefixdir)

def install(packagename, prefixdir):
    requirements = get_all_requirements(packagename)
    uninstalled_requirements = [ r for r in requirements if not get_package(r).is_installed() ]
    if uninstalled_requirements==[]:
        print "Everything already installed"
    else:
        print "The following packages need to be installed:"
        print string.join(uninstalled_requirements,'\n')
        print "Should I proceed ?"
        for name in uninstalled_requirements:
            install_single_package(name,prefixdir)
        
def run():
    args = sys.argv[1:]

    if len(args)!=3:
        help_and_exit()

    command = args[0]
    if command=='install':
        packagename = args[1]
        prefixdir = args[2]
        install(packagename,prefixdir)

run()
