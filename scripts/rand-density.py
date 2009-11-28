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
Creates symmetric TSPLIB problems with random densities.
'''


class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def main(argv=None):
    name = ""
    nodes = 0
    min_cost = 0
    max_cost = 3000
    seed = 0
    
    if argv is None:
        argv = sys.argv
    try:        
        try:
            opts, args = getopt.getopt(argv[1:], "hn:s:r:l:u:", ["help", "name=", "size=", "seed=", "min=", "max="])
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
            if option in ("-l", "--min"):
                try:
                    min_cost = int(value)
                except:
                    raise Usage("must specify integer for minimum cost")
            if option in ("-u", "--max"):
                try:
                    max_cost = int(value)
                except:
                    raise Usage("must specify integer for maximum cost")
            if option in ("-r", "--seed"):
                try:
                    seed = int(value)
                except:
                    raise Usage("must specify integer for seed")
                    
        if nodes <= 0:
            raise Usage("number of nodes must be positive")
        if min_cost < 0:
            raise Usage("min cost must be positive")
        if max_cost < min_cost:
            raise Usage("max cost must be larger than min cost")
        if seed < 0:
            raise Usage("seed must be positive")
            
    except Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2
    
    if name == "":
        name = "den%d.%d.%d" % (nodes, seed, max_cost)
    
    random.seed(seed)
    
    print "NAME: %s" % name
    print "TYPE: TSP"
    print "COMMENT: seed %d, min %d, max %d" % (seed, min_cost, max_cost)
    print "DIMENSION: %d" % nodes
    print "EDGE_WEIGHT_TYPE: EXPLICIT"
    print "EDGE_WEIGHT_FORMAT: LOWER_DIAG_ROW"
    print "EDGE_WEIGHT_SECTION"
    
    for i in range(nodes):
        for j in range(i + 1):
            if i == j:
                print "0\t" ,
            else:
                print "%d\t" % random.randint(min_cost, max_cost),
        print
    print "EOF"
    
if __name__ == "__main__":
    sys.exit(main())
