"""integration test for mk4io python/c-library interface"""
#core imports
import datetime
import argparse
import sys
import os
import logging

#HOPS imports
import hops_test as ht
import mk4io
import vpal.processing

################################################################################

def main():

    parser = argparse.ArgumentParser(
        prog='test_mk4io.py', \
        description='''integration test for mk4io''' \
        )

    parser.add_argument('data_directory', help='path to the test data directory')
    args = parser.parse_args()
    exp_dir = os.path.abspath(args.data_directory)
    if os.path.isdir(exp_dir) is False:
        return 1

    #we just want to test that we are able to open mk4-type files via the c-library
    ret_status = 0;

    #try loading a single corel file in the example dir
    type1_file_list = ht.recursive_find_corel_files(exp_dir)
    if len(type1_file_list) > 0:
        #try to load a corel file
        try:
            corel_obj = mk4io.mk4corel(type1_file_list[0])
        except AttributeError as e1:
            print("A mk4io library exception was thrown:")
            print(str(e1))
            ret_status = 1;
    else:
        #no files found (something went wrong with un-tarring test data)
        ret_status = 2

    return ret_status


if __name__ == '__main__':          # official entry point
    return_value = main()
    sys.exit(return_value)
