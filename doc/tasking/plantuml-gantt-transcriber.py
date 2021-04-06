#!/usr/bin/python3

""" 
Author: violetp 
Contact: violetp@mit.edu
Transcriber to convert the output of the perl scripts to a plantuml text class input file.
The purpose is to automate the generation of the plantuml file to expedite the generation
of gantt charts. 
Note: The plantuml program still needs to be run on the resulting output of this script in
order to generate a class diagram.
For information on plantuml see: https://plantuml.com/
"""

"""
To Do:
    Change parser to get start dates, stop dates, domain name.
    Canonicalize the script to it uses argparse? source/python_src/Bindings/test/TestNumpymhops.py
    Use a glob to find all the input files or accept them as commandline arguments? (yikes) Ideally then make can call this script
"""

import textwrap
import re
import sys
from glob import glob
from datetime import date
from random import randint
import argparse

def print_starting_boiler_plate(p_file):
    p_file.write(textwrap.dedent("""
    @startuml

    projectscale monthly
    Project starts 2019-01-01
    """))

def parse_files():
    """ Equivalent to parsing in shell script using: head -n 1 file.txt | tr -d -c [0-9]

        Args:
            None

        Returns:
            list: A list of dictionaries containing values from each task or domain in all.wbs

    """
    type_t = [] # tasks
    domain = []
    start = []
    days = []
    done = []
    stop = []
    data = []
    f = open('../../../ambld-4.00-new/doc/tasking/fruit/all.wbs', 'r') 
    # check if line has keyword && ->
    for line in f:
        if "type ->" in line:
            type_t.append((re.split(' |\n', line)[3]))
        if "domain ->" in line:
            domain.append((re.split('-> |\n', line)[1]))
        if "start ->" in line:
            start.append((re.split(' |\n', line)[3]))
        if "days ->" in line:
            days.append((re.split(' |\n', line)[3]))
        if "done ->" in line:
            done.append((re.split(' |\n', line)[3]))
        if "stop ->" in line:
            stop.append((re.split(' |\n', line)[3]))
    f.close()
    # Loop through the lists for len(type_t) and create a dictionary for each element with the same index
    for element in range(len(type_t)):
        data.append(dict(type_t = type_t[element], domain = domain[element], start = start[element], 
            days = days[element], done = done[element], stop = stop[element]))
    return data

def get_domains(data):
    """ Parse list and produce a new list of domains without duplicates

        Args:
            data (list): A list of dictionaries containing values from each task or domain in all.wbs

        Returns:
            list: A subset of data containing only the domains

    """
    domains = []
    for dictionary in data:
        if dictionary['domain'] not in domains:
            domains.append(dictionary['domain'])
    return domains

def calculate_percent_complete(data, domain):
    """ For a given domain return what percent complete it is by getting the average of all the tasks and the entry in data for domain itself
        If the percent complete is 0.0, then return a random percent from 1 to 10

        Args:
            data (list): A list of n-tuples containing values from each task or domain in all.wb
            domain (string): 

        Returns:
            str: A string representation of the percent complete of the domain

    """
    percents = []
    for dictionary in data:
        if domain in dictionary['domain']:
            percents.append(dictionary['done'])

    percent = ((sum([float(p) for p in percents])/len(percents))*100) # Change percent to different name or change percent in list comprehension to different name

    # If the percent complete is less than or equal to zero, get a percentage at random
    if int(percent) <= 0:
        result = str(randint(1,10))
        return str(result)

    return str(percent)

def find_min_and_max_date(data, domain):
    """ Find the start and stop date for a given domain 

        Args:
            data (list): a list of dictionaries
            domain (list): a list of strings
        
        Returns:
            dict: {'start': min date, 'stop': max date] The format of the dates is "%Y-%m-%d"

    """
    domain_dates = []
    for dictionary in data:
        if domain in dictionary['domain']:
            if "depends" not in dictionary['start']:
                domain_dates.append(dictionary['start'])
                if "depends" not in dictionary['stop']: # NOTE This statement might have a logical error and may need to be shifted 4 spaces to the left 
                    domain_dates.append(dictionary['stop'])

    # If there are no dates return default dates
    if len(domain_dates) == 0:
        return {'start': "2020-11-01", 'stop': "2022-04-01"}

    # If the dates are the same generate an end date that is 60 days from the provided date
    elif len(domain_dates) == 1 or domain_dates[0] in domain_dates[1]:
        start_date = date.fromisoformat(domain_dates[0])
        project_end_date = date.fromisoformat("2022-04-01")
        timedelta = project_end_date - start_date
        end_date = (start_date + timedelta).isoformat()
        return {'start': domain_dates[0], 'stop': end_date}

    # Otherwise return the earliest and latest dates
    return {'start': min(domain_dates), 'stop': max(domain_dates)}

if __name__ == "__main__":

    # Support argparse as part of the make build process for HOPS 4 
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-v", "--verbose", action="count", default=0,
            help="increase output verbosity")
    args = parser.parse_args()
    if args.verbose == 1:
        # Be verbose
        pass
    if args.verbose >= 2:
        # Be more verbose
        pass

    # Get contentse of file
    data = parse_files()

    # Get domains
    domains = get_domains(data)

    # Open new plantuml file
    plantuml_file = open("plantuml-code-gantt-chart.uml", 'w')

    # Write to the plantuml file
    print_starting_boiler_plate(plantuml_file)

    for domain in domains:
        dates = find_min_and_max_date(data, domain)
        percent_complete = calculate_percent_complete(data, domain)
        plantuml_file.write(textwrap.dedent("""
        [{}] starts {} and ends {} and is {}% completed
        """.format(domain, dates['start'], dates['stop'], percent_complete[0])))

        #[domain] begins on YYYY-MM-DD and ends YYYY-MM-DD is ##% complete

    plantuml_file.write("@enduml")

    # Close plantuml file
    plantuml_file.close()

