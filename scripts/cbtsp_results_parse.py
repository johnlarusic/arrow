#!/usr/bin/env python
# encoding: utf-8
"""
arrow_cbtsp_parse.py

Created by John LaRusic on 2008-06-29.
"""

import sys
import getopt
import glob
from xml.etree import ElementTree as ET


help_message = "usage: parse.py file1.xml file2.xml ... fileN.xml"


class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def main(argv=None):
    docs = []
    
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "h", ["help"])
        except getopt.error, msg:
            raise Usage(msg)
    
        # option processing
        for option, value in opts:
            if option in ("-h", "--help"):
                raise Usage(help_message)
                
        # argument processing
        for arg in args:
            files = glob.glob(arg)
            docs.extend(files)
    
    except Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2

    # print out header row
    print "problem_file,params,max_length,objective_value,tour_length," + \
          "optimal,lower_bound,lower_bound_time,binary_search_steps," + \
          "linkern_attempts,linkern_avg_time,exact_tsp_attempts," + \
          "exact_tsp_time,btsp_total_time,total_time"
    
    # for each document given, spit out data in CSV format
    for doc in docs:
        try:
            xDoc = ET.parse(doc)
            root = xDoc.getroot()
        
            problem_file        = root.attrib["problem_file"]
            params              = root.attrib["command_args"]
            max_length          = root.find("max_length").text
            objective_value     = root.find("objective_value").text
            tour_length         = root.find("tour_length").text
            optimal             = root.find("optimal").text
            lower_bound         = root.find("lower_bound").text
            lower_bound_time    = root.find("lower_bound_time").text
            binary_search_steps = root.find("binary_search_steps").text
            linkern_attempts    = root.find("linkern_attempts").text
            linkern_avg_time    = root.find("linkern_avg_time").text
            exact_tsp_attempts  = root.find("exact_tsp_attempts").text
            exact_tsp_avg_time  = root.find("exact_tsp_avg_time").text
            btsp_total_time     = root.find("btsp_total_time").text
            total_time          = root.find("total_time").text
        
            print "'" + problem_file + "','" + params + "'," + max_length + "," \
                + objective_value + "," + tour_length + "," + optimal + "," \
                + lower_bound + "," + lower_bound_time + "," \
                + binary_search_steps + "," + linkern_attempts + "," \
                + linkern_avg_time + "," + exact_tsp_attempts + "," \
                + exact_tsp_avg_time + "," + btsp_total_time + "," + total_time
        except:
             sys.stderr.write("ERROR OPENING %s\n" % doc)
if __name__ == "__main__":
    sys.exit(main())
