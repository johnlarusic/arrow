#!/usr/bin/env python
# encoding: utf-8
"""
nozzle-vanes.py

Created by John LaRusic on 2009-03-28.
Copyright (c) 2009. All rights reserved.
"""

import sys
import getopt
import random
import math

help_message = '''
Creates an asymmetric TSPLIB nozzle vane assembly problem.
'''


class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def main(argv=None):
    name = ""
    nodes = 0
    seed = 0
    
    if argv is None:
        argv = sys.argv
    try:        
        try:
            opts, args = getopt.getopt(argv[1:], "hn:s:r:", ["help", "name=", "size=", "seed="])
        except getopt.error, msg:
            raise Usage(msg)
    
        # option processing
        for option, value in opts:
            if option == "-v":
                verbose = True
            if option in ("-h", "--help"):
                raise Usage(help_message)
            if option in ("-n", "--name"):
                name = value
            if option in ("-s", "--nodes"):
                try:
                    nodes = int(value)
                except:
                    raise Usage("must specify integer for number of nodes")
            if option in ("-r", "--seed"):
                try:
                    seed = int(value)
                except:
                    raise Usage("must specify integer for seed")
                    
        if nodes <= 0:
            raise Usage("number of nodes must be positive")
        if seed < 0:
            raise Usage("seed must be positive")
            
    except Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2
    
    if name == "":
        name = "vane%d.%d" % (nodes, seed)
    
    min_rand = -1.0
    max_rand =  1.0
    mult = 10000
    infty = mult * 16
    
    random.seed(seed)
    
    d = 0
    A = range(nodes)
    B = range(nodes)
    
    for i in range(nodes):
        A[i] = random.uniform(min_rand, max_rand)
        B[i] = random.uniform(min_rand, max_rand)
        d = d + A[i] + B[i]
        # print "%d\t%5.4f\t%5.4f" % (i, A[i], B[i])
    d = d / nodes
    # print "d = %5.4f" % d
    
    print "NAME: %s" % name
    print "TYPE: ATSP"
    print "COMMENT: seed %d" % seed
    print "DIMENSION: %d" % nodes
    print "EDGE_WEIGHT_TYPE: EXPLICIT"
    print "EDGE_WEIGHT_FORMAT: FULL_MATRIX"
    print "EDGE_WEIGHT_SECTION"
    
    for i in range(nodes):
        for j in range(nodes):
            if i == j:
                print "%d\t" % infty ,
            else:
                print "%d\t" % (math.floor(mult * (d - (A[i] + B[j]))**2 + 0.5)) ,
        print
    print "EOF"
    
if __name__ == "__main__":
    sys.exit(main())
