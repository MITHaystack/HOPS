"""regression test: verify Python ctypes c_block size matches the compiled C struct c_block size.

This catches the class of bug where fields are added to control.h but not to
c_block._fields_ in ffcontrol.py, causing silent buffer overflows / segfaults
when the C library writes into a Python-allocated c_block.
"""
import ctypes
import sys

import ffcontrol


def main():
    ffcontrol_lib = ffcontrol.load_ffcontrol_library()
    if ffcontrol_lib is None:
        print("ERROR: could not load ffcontrolpy shared library")
        return 1

    ffcontrol_lib.get_cblock_size.restype = ctypes.c_size_t
    c_size = ffcontrol_lib.get_cblock_size()
    py_size = ctypes.sizeof(ffcontrol.c_block)

    if c_size != py_size:
        print(
            "FAIL: c_block size mismatch, "
            f"C struct is {c_size} bytes, Python ctypes struct is {py_size} bytes. "
            "Check that c_block._fields_ in ffcontrol.py matches struct c_block in control.h."
        )
        return 1

    print(f"PASS: c_block sizes match ({c_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
