#!/usr/bin/python
from buildimpl import *
import argparse

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("-b", "--build", action='store_true')
parser.add_argument("-c", "--clean", action='store_true')
parser.add_argument("-e", "--eclipse", action='store_true')
parser.add_argument("-g", "--generate", action='store_true')
parser.add_argument("-t", "--test", action='store_true')
parser.add_argument("-a", "--all", action='store_true')
parser.add_argument("-r", "--release", action='store_true')
parser.add_argument("-f", "--release_folder", default='release/')
args = parser.parse_args()

# cleaning
if args.clean or args.all:
    clean()

# building
if args.build or args.all or len(sys.argv) == 1:
    build()

# generating eclipse projects
if args.test or args.all:
    test()
    
# generating eclipse projects
if args.eclipse or args.all:
    eclipse()

# generating model
if args.generate:
    generate()
    
# generating model
if args.release:
    release(args.release_folder)
