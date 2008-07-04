#!/usr/bin/env python
# encoding: utf-8
"""
sample_abtsp_experiments.py

Created by John LaRusic on 2008-06-19.
"""
# PARAMETERS
program_dir     = "/Users/johnlarusic/Dev/arrow"
programs        = ["bbssp", "bap", "bscssp", "2mb", "dcbpb"]
problem_files   = "/Users/johnlarusic/Dev/problems/atsp/ftv3*.atsp"
output_dir      = "/Users/johnlarusic/Dev/results/abtsp/08-08-03"
trials          = 2

# START SCRIPT
import sys
import os
import glob
import math
import os.path
import ConfigParser

# Read configuration scripts
data = ConfigParser.ConfigParser()
data.read("data.ini")

# Get a list of problems
problems = glob.glob(problem_files)

# Output confirmation of experiment
print "EXPERIMENT DETAILS:"
print " - Output Directory: '%s'" % output_dir
print " - # of Trials: %d" % trials
print " - Problems:"
for problem in problems:
    print "     - '%s'" % problem
question = raw_input('Run experiment? Type \'yes\' to confirm: ')
if question != "yes":
    sys.exit("Cancelled")

# These keep track of the amount of work done so far
total = len(problems) * trials * len(programs)
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
        print " - problem: %s" % problem_name
    
        for program in programs:
            print "   - program: %s" % program
            for trial in range(1, trials + 1):
                file_trial = "%s/%s.%s.%02d" % \
                    (output_dir, problem_name, program, trial)
                xml_file = "%s.xml" % file_trial
                stdout_file = "%s.txt" % file_trial

                print "     - trial %d of %d, %d of %d total or %.1f percent\"" % \
                    (trial, trials, total_done, total, total_per)
                
                command = "%s/%s -i %s -x %s > %s" % \
                    (program_dir, program, problem, xml_file, stdout_file)
                os.system(command)
            
                total_per += total_each
                total_done += 1
    except:
        sys.stderr.write("Missing data for %s\n" % problem_name)

        

