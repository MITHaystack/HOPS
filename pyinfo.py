#determine if distutils or setuptools is what we are going to use 
setup_option=""
import sys
if sys.version_info.major == 3 and sys.version_info.minor >= 10:
   try:
       from setuptools import setup
       setup_option="setuptools"
   except ImportError:
       #setuptools not installed on this system
       from distutils.core import setup
       setup_option="distutils.core"
else:
    setup_option="distutils.core"

print(setup_option)
