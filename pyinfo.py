#construct name for local site-packages directory
import os
import sys
version_str = str(sys.version_info.major) + "." + str(sys.version_info.minor)
pyinstall_path = os.path.join("lib", "python" + version_str, "site-packages")

#determine if distutils or setuptools/pip is what we are going to use 
setup_option="distutils.core" #default for python <3.10
need_pip="false"

try:
   from setuptools import setup
   setup_option="setuptools"
   need_pip="true"
except ImportError:
   #setuptools not installed on this system
   from distutils.core import setup
   setup_option="distutils.core"
   need_pip="false"

pip_installed="false"
try:
    import pip 
    pip_installed="true"
except ImportError:
    pip_installed="false"

info_str = version_str + ";" + pyinstall_path + ";" + setup_option + ";" + need_pip + ";" + pip_installed
#info_str = version_str + ";" + pyinstall_path + ";" + "distutils.core" + ";" + "false" + ";" + pip_installed

print(info_str)
