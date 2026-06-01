#construct name for local site-packages directory, e.g. lib/python3.12/site-packages
#consumed by the top-level CMakeLists.txt to set PYTHON_SITE_PREFIX / PYTHON_MODULE_INSTALL_DIR

import os
import sys
version_str = "{0}.{1}".format(sys.version_info.major, sys.version_info.minor)
print(os.path.join("lib", "python" + version_str, "site-packages"))
