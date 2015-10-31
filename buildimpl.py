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

def publish():
    print("publish(): Not yet implemented!")
    
def generate():
    generate_model()

def release():
    releaseRoot = "release"
    components = releaseRoot + "/components"
    dependencies = releaseRoot + "/dependencies"
    rmFile("release.zip")
    rmFolder(releaseRoot)
    mkFolder(releaseRoot)
    mkFolder(components)
    mkFolder(dependencies)
    for file in findFiles(".", '*assembly*.jar*'):
        trgFilePath = components + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    for file in findFiles(".", '*.json*'):
        trgFilePath = components + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    for file in findFiles("gpt-startupscripts", '*.bat*'):
        trgFilePath = components + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    for file in findFiles("dependencies", '*'):
        trgFilePath = dependencies + "/" + os.path.basename(file)
        shutil.copyfile(file, trgFilePath)
    rmFile(components + "/gpt-common-assembly-SNAPSHOT.jar")
    rmFile(components + "/gpt-assembly-SNAPSHOT.jar")
    shutil.copyfile("README.txt", "release/README.txt")
    shutil.copyfile("LICENSE", "release/LICENSE.txt")
    shutil.make_archive("release", format="zip", root_dir="release")    

    
###########
# HELPERS #
###########

mgen_cmd = "NEEDS_TO_BE_SET"
pluginPaths = "NEEDS_TO_BE_SET"

def generate_model():
    check_call("mgen models/project.xml", cwd="gpt-common", shell=True)
    
def build_jvm():
    check_call("sbt compile package publish-local assembly", shell=True)

def clean_gen():
    rmFolder("gpt-common/src_generated")

def clean_jvm():
    check_call("sbt clean", shell=True)
    
def clean_release():
    rmFolder("release")
    