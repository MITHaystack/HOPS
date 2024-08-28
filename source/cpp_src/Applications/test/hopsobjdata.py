import argparse
import json
import sys

#this is crude, but only uses standard lib
def sorting(item):
    if isinstance(item, dict):
        return sorted((key, sorting(values)) for key, values in item.items())
    if isinstance(item, list):
        return sorted(sorting(x) for x in item)
    else:
        return item

def main():

    parser = argparse.ArgumentParser(
         prog='hopsobjdata.py', \
         description='''primitive utility to compare hops file json summary dumps''' \
         )

    parser.add_argument('fileA', help='the first json file')
    parser.add_argument('fileB', help='the second json file')

    args = parser.parse_args()

    f1_name = args.fileA
    f2_name = args.fileB

    f1 = open(f1_name, "r")
    f2 = open(f2_name, "r")
    j1 = json.loads(f1.read())
    j2 = json.loads(f2.read())

    #brute force is the last resort of the incompetent.
    cnk = "class_name"
    n_equiv_obj = 0
    for key1, value1 in j1.items():
        if cnk in value1.keys():
            for key2, value2 in j2.items():
                if cnk in value1.keys():
                    if(value1[cnk] == value2[cnk]):
                        if sorting(value1 == value2):
                            n_equiv_obj += 1

    print("Number of objects in files which are equivalent = ", n_equiv_obj)

    if n_equiv_obj < 2:
        return 1 #failed to detect both weights and visibilities which share type/dims/size in new and reference files
    else:
        return 0


if __name__ == '__main__':          #entry point
    ret_val = main()
    sys.exit(ret_val)
