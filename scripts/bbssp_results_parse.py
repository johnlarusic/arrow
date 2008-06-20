#!/usr/bin/env python
# encoding: utf-8
"""
arrow_bbssp_parse.py

Created by John LaRusic on 2008-05-07.
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
    print "problem_file, params, objective_value, total_time"
    
    # for each document given, spit out data in CSV format
    for doc in docs:
        xDoc = ET.parse(doc)
        root = xDoc.getroot()
        
        problem_file = root.attrib["problem_file"]
        params = root.attrib["params"]
        objective_value = root.find("objective_value").text
        total_time = root.find("total_time").text
        
        print "'%s', '%s', %s, %s" % (problem_file, params, objective_value, total_time)

if __name__ == "__main__":
    sys.exit(main())
