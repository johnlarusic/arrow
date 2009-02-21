#!/usr/bin/env python
# encoding: utf-8
"""
sample_cbtsp_experiments.py

Created by John LaRusic on 2008-06-19.
"""
# PARAMETERS
program         = "/Users/johnlarusic/Dev/arrow/cbtsp"
problem_files   = "/Users/johnlarusic/Dev/problems/tsplib/*.tsp"
multipliers     = [1.00, 1.05, 1.10, 1.20]

# START SCRIPT
import sys
import os
import glob
import math
import os.path
import ConfigParser

# Read configuration scripts
data = ConfigParser.ConfigParser()
data.read("cbtsp_data.ini")

# Get a list of problems
problems = glob.glob(problem_files)

# Output confirmation of experiment
print "PROBLEM" ,
for multiplier in multipliers:
    print "\t%.2f" % multiplier ,
print ""

# For each problem calculate length times multiplier for each multiplier
for problem in problems:
    problem_base = os.path.basename(problem)
    problem_name =  problem_base[0:problem_base.rfind('.')]
    
    try:
        tsp_obj = int(data.get("tsp", problem_name))
    
        print "%s" % problem_name ,
        for multiplier in multipliers:
            L_param = int(math.floor(1.0 * tsp_obj * multiplier + 0.5))
            
            print "\t%d" % L_param,
        print ""
    except:
        sys.stderr.write("Missing data for %s\n" % problem_name)

