#!/usr/bin/env python
# encoding: utf-8
"""
cbtsp_experiments.py

Created by John LaRusic on 2008-06-12.
"""

import sys
import getopt
import glob
import math
import os.path
import ConfigParser


help_message = '''
[-t #trials] [-i output_dir] file1 file2...
'''

problems = []
trials = 10
output_dir = "."
multiplier = 1.00


class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def parse_args(argv):
    global multiplier
    global trials
    global output_dir
    
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "ht:o:m:", 
                                       ["help", "trials=", "multiplier=", "output-dir="])
        except getopt.error, msg:
            raise Usage(msg)
    
        # option processing
        for option, value in opts:
            if option in ("-h", "--help"):
                raise Usage(help_message)
            if option in ("-t", "--trials"):
                trials = int(value)
            if option in ("-o", "--output-dir"):
                output_dir = value
            if option in ("-m", "--multiplier"):
                multiplier = eval(value)
        
        # argument processing
        for arg in args:
            files = glob.glob(arg)
            problems.extend(files)
    
    except Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2

def get_name(tsplib_file):
    base = os.path.basename(tsplib_file)
    return base[0:base.rfind('.')]


def main(argv=None):
    parse_args(argv)
    
    print "#!/bin/bash"
    print "# Multiplier: %.5f\n" % multiplier
    
    data = ConfigParser.ConfigParser()
    data.read("data.ini")
    
    settings = ConfigParser.ConfigParser()
    settings.read("settings.ini")
    program = settings.get("cbtsp", "program")
    
    total = len(problems) * trials
    total_done = 1;
    total_per = 0.0;
    total_each = 1.0 / total * 100
    
    for problem in problems:
        problem_name = get_name(problem)
        
        try:
            tsp_obj = int(data.get("tsp", problem_name))
            btsp_obj = int(data.get("btsp", problem_name))
            L_param = int(math.floor(1.0 * tsp_obj * multiplier + 0.5))
            
            print "# %s (TSP Objective: %.0f, BTSP Objective: %d)" % \
                (problem_name, tsp_obj, btsp_obj)
            for i in range(1, trials + 1):
                file_trial = "%s/%s.%02d" % (output_dir, problem_name, i)
                xml_file = "%s.xml" % file_trial
                stdout_file = "%s.txt" % file_trial
                
                print "echo \"%s (%d of %d for problem, %d of %d total or %.1f percent)\"" % \
                    (problem_name, i, trials, total_done, total, total_per)
                print "%s -i %s -L %d -l %d -x %s > %s" % \
                    (program, problem, L_param, btsp_obj, xml_file, stdout_file)
                total_per += total_each
                total_done += 1
            print ""
            
        except:
            sys.stderr.write("Missing data for %s\n" % problem_name)
            sys.exit(2)

if __name__ == "__main__":
    sys.exit(main())
