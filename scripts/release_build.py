"""
This python script will create a release on github,
it will generate a version.h file in src folder with macro VERSION equal to
the version provided.
"""
import json
import os
import sys
import argparse
from argparse import RawTextHelpFormatter
from github_release import gh_release_create


### Command line argument parser

# Intro message
intro = "This is an automated release tool!\n\n"
intro += "Make sure that you did 'export GITHUB_TOKEN=<YOUR_TOKEN>' before using this tool, here is how to do it:\n"
intro += "https://help.github.com/en/github/authenticating-to-github/creating-a-personal-access-token-for-the-command-line"

# Parser setup
parser = argparse.ArgumentParser(description=intro, formatter_class=RawTextHelpFormatter)
parser.add_argument("--version", type=str,required=True, help="This is a version of release, something like v1.2.5")
parser.add_argument("--body", type=str,required=True, help="This is the visible text in release body")
parser.add_argument("--branch", type=str,required=True, help="This is a branch that the release will be made from")

# Display help message if no arguments are provided
if len(sys.argv)==1:
    parser.print_help(sys.stderr)
    sys.exit(1)
args = parser.parse_args()

# Remove dots and 'v' forversion.h file
actual_version = args.version.encode().decode()
version = args.version.replace(".","")
version = version.replace("v","")

# Lets first create content of our version.h and then create it
version_h = "#ifndef VERSION_H\n"
version_h += "#define VERSION_H\n\n"
version_h += "// This file was generated by scripts/release_build.py file.\n\n"
version_h += "#define VERSION " + version + "\n\n"
version_h += "#endif /* VERSION_H *\n"
version_h += "/*** end of file ***\n"

with open("../src/version.h", "w+") as f:
    f.write(version_h)

# Get the project name into a variable
project_name = os.path.basename(os.path.realpath("../"))
repo = "IRNAS/" + project_name

# Create a release, thats it, one command
gh_release_create(repo, actual_version, publish=True, target_commitish=args.branch, name=actual_version, body=args.body)
