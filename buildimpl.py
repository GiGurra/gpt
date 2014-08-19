#!/usr/bin/python
from buildutil import *

###########
# TARGETS #
###########

def clean():
    rmFolder("gpt-common/src_generated")
    check_call("sbt clean", shell=True)
    check_call("python build.py -c", cwd="gpt-displaystransmitter", shell=True)
    
def build():
    generate_model()
    check_call("sbt compile package publish-local assembly", shell=True)
    check_call("python build.py -b", cwd="gpt-displaystransmitter", shell=True)

def test():
    sbt_test(".")

def eclipse():
    check_call("sbt eclipse", shell=True)

def publish():
    print("publish(): Not yet implemented!")
    
def generate():
    generate_model()

def deploy(folder):
    rmFolder(folder)
    mkFolder(folder)
    for jarFile in findFiles(".", '*assembly*.jar*'):
        trgFilePath = folder + "/" + os.path.basename(jarFile)
        shutil.copyfile(jarFile, trgFilePath)

    
###########
# HELPERS #
###########

mgen_cmd = "NEEDS_TO_BE_SET"
pluginPaths = "NEEDS_TO_BE_SET"
default_cpp_build_cfg = "NEEDS_TO_BE_SET"

def generate_model():
    check_call("mgen models/project.xml", cwd="gpt-common", shell=True)
    