#!/usr/bin/python
from buildutil import *

###########
# TARGETS #
###########

def clean():
    clean_gen()
    clean_jvm()
    
def build():
    generate_model()
    build_jvm()

def test():
    sbt_test(".")

def generate():
    generate_model()

def release():
    release = "release/gpt"
    rmFile("release.zip")
    rmFolder(release)
    mkFolder(release)
    for file in findFiles(".", '*assembly*.jar*'):
        trgFilePath = release + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    for file in findFiles(".", '*.json*'):
        trgFilePath = release + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    for file in findFiles("gpt-startupscripts", '*.bat*'):
        trgFilePath = release + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    rmFile(release + "/gpt-common-assembly-SNAPSHOT.jar")
    rmFile(release + "/gpt-assembly-SNAPSHOT.jar")
    shutil.copyfile("README.txt", release + "/README.txt")
    shutil.copyfile("LICENSE", release + "/LICENSE.txt")
    shutil.make_archive("release", format="zip", root_dir="release")    

    
###########
# HELPERS #
###########

mgen_cmd = "NEEDS_TO_BE_SET"
pluginPaths = "NEEDS_TO_BE_SET"

def generate_model():
    check_call("mgen models/project.xml", cwd="gpt-common", shell=True)
    
def build_jvm():
    check_call("sbt compile package assembly", shell=True)

def clean_gen():
    rmFolder("gpt-common/src_generated")

def clean_jvm():
    check_call("sbt clean", shell=True)
    
def clean_release():
    rmFolder("release")
    