#!/usr/bin/env python
""" 
Author: violetp 
Contact: violetp@mit.edu
Transcriber to convert the output of the perl script to a plantuml text class input file.
The purpose is to automate the generation of the plantuml file to expedite the generation
of class diagrams. 
Note: The plantuml program still needs to be run on the resulting output of this script in
order to generate a class diagram.
For information on plantuml see: https://plantuml.com/
"""
import textwrap
import re
import sys
from glob import glob

def print_starting_boiler_plate(p_file):
    """Print the plantuml header and class boiler plate to the file."""
    p_file.write(textwrap.dedent("""
    @startuml
    ' Use MIT colors
    skinparam class {
    \t\tArrowColor #A31F34
    \t\tBordercolor #8a8b8c
    \t\tBackgroundColor #fffff0
    }

    ' Create classes
    class 0001 <<Domain>> {

    }
    class 0010 <<Thing>> {

    }

    """))
    
def print_associations_boiler_plate(p_file):
    """Print the domain and thing associations."""
    plantuml_file.write(textwrap.dedent("""
    ' Show associations 
    0001 <|-- 0010
    0010 <|-- 0100
    """))

def print_hide_members_boiler_plate(p_file):
    plantuml_file.write(textwrap.dedent("""
    ' Hide class diagram circles
    hide members
    hide <<Domain>> circle
    hide <<Thing>> circle
    """))

def parse_files(files):
    """ Equivalent to parsing in shell script using: head -n 1 file.txt | tr -d -c [0-9] """
    f_num = []
    d = []
    for file in files:
        f = open(file, 'r') 
        fst = f.readline()
        snd = f.readline()
        f_num.append((re.split('_([0-9]*)\.', fst)[1]))
        d.append((re.split('@|\n', snd)[1]))
        f.close()
    return list(zip(f_num, d))

if __name__ == "__main__":
    # Files
    all_files = glob('alltasks/*_0[1-9]*.txt')

    # Get contentse of file
    data = parse_files(all_files)

    # Open new plantuml file
    plantuml_file = open("plantuml-code.uml", 'w')

    # Print the plantuml header and classes 
    print_starting_boiler_plate(plantuml_file)

    # Print the classes
    for element in data:
        plantuml_file.write(textwrap.dedent("class {} <<@{}>> ".format(element[0], element[1])))
        plantuml_file.write("{\n\n}\n")
    
    # Print the associations
    print_associations_boiler_plate(plantuml_file)
    for element in data:
        plantuml_file.write(textwrap.dedent("0010 <|-- {}\n".format(element[0])))

    # Print the 
    print_hide_members_boiler_plate(plantuml_file)
    for element in data:
        plantuml_file.write(textwrap.dedent("hide <<@{}>> circle\n".format(element[1])))

    # Boiler plate to finish off the plantuml file 
    plantuml_file.write("@enduml")

    # Close plantuml file
    plantuml_file.close()

