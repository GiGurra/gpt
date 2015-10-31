#!/usr/bin/python
import argparse
import shutil
import os.path
import distutils.dir_util
import sys
import subprocess
import fnmatch
import platform
from subprocess import check_call

def mkFolder(path):
    if not os.path.exists(path):
        os.makedirs(path)
        
def rmFolder(path):
    if os.path.exists(path):
        shutil.rmtree(path)
        
def rmFile(path):
    if os.path.exists(path):
        os.remove(path)
        
def copyTree(src, dst):
    distutils.dir_util.copy_tree(src, dst)

def findDirs(path, matching):
    out = []
    for root, dirnames, filenames in os.walk(path):
        for dirName in fnmatch.filter(dirnames, matching):
            out.append(os.path.join(root, dirName))
    return out
      
def findFiles(path, matching):
    out = []
    for root, dirnames, filenames in os.walk(path):
        for fileName in fnmatch.filter(filenames, matching):
            out.append(os.path.join(root, fileName))
    return out
     
def findFilesExt(path, exts):
    out = []
    for root, dirnames, filenames in os.walk(path):
        for fileName in filenames:
            for ext in exts:
                if fileName.endswith(ext):
                    out.append(os.path.join(root, fileName))
    return out

def rmFolders(path, matching):
    for folder in findDirs(path, matching):
        rmFolder(folder)

def rmFiles(path, matching):
    for folder in findFiles(path, matching):
        rmFile(folder)
        
def findLocalFiles(path, matching):
    out = []
    for name in fnmatch.filter(os.listdir(path), matching):
        out.append(name)
    return out

def sbt(path, targets):
    check_call("sbt " + targets, cwd=path, shell=True)  
    
def sbt_test(path):
    sbt(path, "test")

def sbt_clean(path):
    sbt(path, "clean")

def sbt_jasmine(path):
    sbt(path, "jasmine")
