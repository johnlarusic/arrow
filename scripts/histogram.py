#!/usr/bin/env python
# encoding: utf-8
"""
histogram.py

Created by John LaRusic on 2008-05-09.
Copyright (c) 2008. All rights reserved.
"""

import sys
import getopt
import math
from matplotlib import rcParams
from matplotlib.ticker import FuncFormatter
from pylab import *

# Help Message
help_message = '''usage: -i [intervals] -t [title]'''

# Usage Exception
class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

# Main Method        
def main(argv=None):
    # Default parameters
    intervals = 0
    graph_title = None
    
    # Read parameters
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hi:t:", 
                ["help", "intervals=", "title="])
        except getopt.error, msg:
            raise Usage(msg)
    
        # option processing
        for option, value in opts:
            if option in ("-h", "--help"):
                raise Usage(help_message)
            if option in ("-i", "--intervals"):
                try:
                    intervals = int(value)
                except:
                    raise Usage("Interval must be an integer")
            if option in ("-t", "--title"):
                graph_title = value
    
    except Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2

    # Read data from stdin
    data = []
    for line in sys.stdin.readlines():
        length = int(line.rstrip('\n'))
        data.append(length)
        
    # Define formatter for y-axis
    def tick_percent(y, pos):
        return "%1.2f" % (y / len(data))
    formatter = FuncFormatter(tick_percent)
    ax = subplot(111)
    ax.yaxis.set_major_formatter(formatter)
        
    # Setup histogram
    ylabel("Percent (%s Total)" % len(data))
    xlabel("Edge Length")
    
    if graph_title is not None:
        title(graph_title)
    if intervals == 0:
        intervals = math.floor(math.sqrt(math.sqrt(len(data))))
        if intervals < 5: intervals = 5
    
    hist(data, intervals, normed=0)
    show()


if __name__ == "__main__":
    sys.exit(main())
