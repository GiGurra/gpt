#!/usr/bin/python
import sys
import argparse
import subprocess
import shutil
import os.path
from subprocess import check_call

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("-b", "--build", action='store_true')
parser.add_argument("-c", "--clean", action='store_true')
parser.add_argument("-e", "--eclipse", action='store_true')
parser.add_argument("-a", "--all", action='store_true')
args = parser.parse_args()

# cleaning
if args.clean or args.all:
	if os.path.exists("gpt-common/src_generated"): shutil.rmtree("gpt-common/src_generated")
	check_call("sbt clean", shell=True)
	check_call("python build.py -c", cwd="gpt-displaystransmitter", shell=True)

# building
if args.build or args.all or len(sys.argv) == 1:
	check_call("mgen models/project.xml", cwd="gpt-common", shell=True)
	check_call("sbt compile package publish-local assembly", shell=True)
	check_call("python build.py -b", cwd="gpt-displaystransmitter", shell=True)

# generating eclipse projects
if args.eclipse or args.all:
	check_call("sbt eclipse", shell=True)
