#!/usr/bin/env python
# encoding: utf-8
"""
sample_cbtsp_experiments.py

Created by John LaRusic on 2008-06-19.
"""
# PARAMETERS
program         = "/Users/johnlarusic/Dev/arrow/bin/cbap"
problem_files   = "/Users/johnlarusic/Dev/problems/tsplib/t*.tsp"
output_dir      = "/Users/johnlarusic/Dev/results/cbtsp/09-02-23"
trials          = 1
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
print "EXPERIMENT DETAILS:"
print " - Output Directory: '%s'" % output_dir
print " - # of Trials: %d" % trials
print " - Multipliers:"
for multiplier in multipliers:
    print "     - %.2f" % multiplier
print " - Problems:"
for problem in problems:
    print "     - '%s'" % problem
question = raw_input('Run experiment? Type \'yes\' to confirm: ')
if question != "yes":
    sys.exit("Cancelled")

# These keep track of the amount of work done so far
total = len(problems) * len(multipliers) * trials
total_done = 1;
total_per = 0.0;
total_each = 1.0 / total * 100

# Create directory for output if it does not yet exist
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# For each problem, multiplier, and trial, run one experiment
for problem in problems:
    problem_base = os.path.basename(problem)
    problem_name =  problem_base[0:problem_base.rfind('.')]
    
    try:
        tsp_obj = int(data.get("tsp", problem_name))
    
        print " - problem: %s" % problem_name
        for multiplier in multipliers:
            L_param = int(math.floor(1.0 * tsp_obj * multiplier + 0.5))
            
            print "   - multiplier: %.2f" % multiplier
            for trial in range(1, trials + 1):
                file_trial = "%s/m%.2f.%s.%02d" % \
                    (output_dir, multiplier, problem_name, trial)
                xml_file = "%s.xml" % file_trial
                stdout_file = "%s.txt" % file_trial
    
                print "     - trial %d of %d, %d of %d total or %.1f percent\"" % \
                    (trial, trials, total_done, total, total_per)
                    
                command = "%s -i %s -L %d -x %s > %s" % \
                    (program, problem, L_param, xml_file, stdout_file)
                os.system(command)
                
                total_per += total_each
                total_done += 1
    except:
        sys.stderr.write("Missing data for %s\n" % problem_name)

        

