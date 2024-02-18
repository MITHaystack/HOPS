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
         prog='compjson.py', \
         description='''primitive utility to compare two json files''' \
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


    # this comparison relies on the module jsondiff
    # from jsondiff import diff
    #
    # f1 = open("./tmp.json", "r")
    # f2 = open("./tmp2.json", "r")
    # j1 = json.loads(f1.read())
    # j2 = json.loads(f2.read())
    # obj = diff(j1,j2)
    # len(obnj)

    result = False
    if sorting(j1) == sorting(j2):
         result = True

    if result:
        return 0 #ok (files are equal)
    else:
        return 1 #not equivalent



if __name__ == '__main__':          #entry point
    ret_val = main()
    sys.exit(ret_val)
