#construct name for local site-packages directory
import os
import sys
version_str = str(sys.version_info.major) + "." + str(sys.version_info.minor)
print( os.path.join("lib", "python" + version_str, "site-packages") )
