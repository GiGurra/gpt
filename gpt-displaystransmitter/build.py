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
	if os.path.exists("target"): shutil.rmtree("target")		

# building
if args.build or args.all or len(sys.argv) == 1:
	print("Building displaystransmitter not yet implemented!")

# generating eclipse projects
if args.eclipse or args.all:
	print("Generating eclipse project for displaystransmitter not yet implemented!")
