#! /usr/bin/env python3
"""Script to create the Windows installer for BLAST command line applications"""
# $Id: make_win.py 646276 2022-03-04 20:25:09Z merezhuk $
#
# Author: Christiam camacho

from __future__ import print_function
import os, sys, os.path
import shutil
from optparse import OptionParser
SCRIPT_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
sys.path.append(os.path.join(SCRIPT_DIR, ".."))
from blast_utils import safe_exec, update_blast_version

VERBOSE = False
    
# NSIS Configuration file
NSIS_CONFIG = os.path.join(SCRIPT_DIR, "ncbi-blast.nsi")

def extract_installer():
    """Extract name of the installer file from NSIS configuration file"""
    from fileinput import FileInput

    retval = "unknown"
    for line in FileInput(NSIS_CONFIG):
        if line.find("OutFile") != -1:
            retval = line.split()[1]
            return retval.strip('"')

def show_dir_tree( start_path ):
    dir_list = []    
    dir_list = os.listdir( start_path )
    for fn in dir_list:
        if os.path.isfile( os.path.join( start_path,fn )):
            print(os.path.join( start_path,fn ) )
        else:
            show_dir_tree( os.path.join( start_path,fn ) )
    return

def main():
    """ Creates NSIS installer for BLAST command line binaries """
    global VERBOSE #IGNORE:W0603
    parser = OptionParser("%prog <blast_version> <installation directory>")
    parser.add_option("-v", "--verbose", action="store_true", default=False,
                      help="Show verbose output", dest="VERBOSE")
    options, args = parser.parse_args()
    if len(args) != 2:
        parser.error("Incorrect number of arguments")
        return 1
    
    blast_version, installdir = args
    VERBOSE = options.VERBOSE
    
    apps = [ "blastn.exe", 
             "blastp.exe",
             "blastx.exe",
             "tblastx.exe",
             "tblastn.exe",
             "rpsblast.exe",
             "rpstblastn.exe",
             "psiblast.exe",
             "blastdbcmd.exe",
             "makeblastdb.exe",
             "makembindex.exe",
             "makeprofiledb.exe",
             "blastdb_aliastool.exe",
             "segmasker.exe",
             "dustmasker.exe",
             "windowmasker.exe",
             "convert2blastmask.exe",
             "blastdbcheck.exe",
             "blast_formatter.exe",
             "deltablast.exe",
             "legacy_blast.pl",
             "update_blastdb.pl",
             "cleanup-blastdb-volumes.py",
             "get_species_taxids.sh",
             "blastn_vdb.exe",
             "tblastn_vdb.exe",
             "blast_formatter_vdb.exe",
	     "nghttp2.dll",
	     "ncbi-vdb-md.dll"
             ]
    
    cwd = os.getcwd()
    # DEBUG: print current tree content
    print("#DEBUG:START: DIRLIST");
    show_dir_tree( cwd )
    print("#DEBUG:END: DIRLIST");

    for app in apps:
        app = os.path.join(installdir, "bin", app)
        if VERBOSE: 
            print("Copying", app, "to", cwd)
        shutil.copy(app, cwd)
   
    # TODO: remove next copy, nghttp2.dll expected to be in a same place as exe files
    ##dll = os.path.join('\\\\', 'snowman', 'win-coremake', 'Lib', 'ThirdParty', 'nghttp2', 'vs2017.64', '1.33.0', 'bin', 'ReleaseDLL', 'nghttp2.dll')
    ##shutil.copy(dll, cwd)

    # pick up library after DLL: ncbi-vdb-md.dll in Manifest
    # expected to be in CWD + c++\compilers\vs2019\static\bin\ReleaseDLL\ncbi-vdb-md.dll
    #ncbi_vdb_nd_dll = os.path.join(cwd, "c++","compilers","vs2019","static","bin","ReleaseDLL","ncbi-vdb-md.dll")
    #if VERBOSE: 
    #   print("Copying", ncbi_vdb_nd_dll, "to", cwd)
    #shutil.copy(ncbi_vdb_nd_dll, cwd)
    
    update_blast_version(NSIS_CONFIG, blast_version)
    # Copy necessary files to the current working directory
    shutil.copy(NSIS_CONFIG, cwd)
    license_file = os.path.join(SCRIPT_DIR, "..", "..", "LICENSE")
    shutil.copy(license_file, cwd)
    privacy_file = os.path.join(SCRIPT_DIR, "..", "..", "BLAST_PRIVACY")
    shutil.copy(privacy_file, cwd)

    f = open("README.txt", "w")
    f.write("The user manual is available in http://www.ncbi.nlm.nih.gov/books/NBK279690\n")
    f.write("Release notes are available in http://www.ncbi.nlm.nih.gov/books/NBK131777\n")
    f.close()

    for aux_file in ("EnvVarUpdate.nsh", "ncbilogo.ico"):
        src = os.path.join(SCRIPT_DIR, aux_file)
        if VERBOSE:
            print("Copying", src, "to", cwd)
        shutil.copy(src, cwd)
        
    # makensis is in the path of the script courtesy of the release framework
    # use 3.08 version
    cmd = os.path.join('\\\\','snowman','win-coremake','App','ThirdParty','NSIS_3.08','makensis.exe') + ' ' + os.path.basename(NSIS_CONFIG)
    safe_exec(cmd)

    installer_dir = os.path.join(installdir, "installer")
    if not os.path.exists(installer_dir):
        os.makedirs(installer_dir)

    installer = extract_installer()
    shutil.copy(installer, installer_dir)

if __name__ == "__main__":
    sys.exit(main())

