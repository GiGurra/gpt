#!/usr/bin/python
from buildutil import *

###########
# TARGETS #
###########

def clean():
    rmFolder("gpt-common/src_generated")
    rmFolder("gpt-displaystransmitter/target")
    check_call("sbt clean", shell=True)
    
def build():
    generate_model()
    build_cpp()
    check_call("sbt compile package publish-local assembly", shell=True)

def test():
    sbt_test(".")

def eclipse():
    check_call("sbt eclipse", shell=True)

def publish():
    print("publish(): Not yet implemented!")
    
def generate():
    generate_model()

def deploy(folder):
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
    
def build_cpp():
    mkFolder("gpt-displaystransmitter/target")
    mkFolder("gpt-displaystransmitter/target/hook")
    mkFolder("gpt-displaystransmitter/target/loader") 
    cmake("gpt-displaystransmitter/target/hook", "../../src/hook", "Release")
    cmake("gpt-displaystransmitter/target/loader", "../../src/loader", "Release")
    cppBuild("gpt-displaystransmitter/target/hook", "Release", "gpt_displaystransmitter_hook")
    cppBuild("gpt-displaystransmitter/target/loader", "Release", "gpt_displaystransmitter_loader")
