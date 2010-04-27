#!/usr/bin/env python
# encoding: utf-8
"""
sym_tour_to_asym.py

Created by John LaRusic on 2010-04-05.
Copyright (c) 2010 __MyCompanyName__. All rights reserved.
"""

import sys
import getopt


help_message = '''
-i [tour_file]
'''


class Usage(Exception):
	def __init__(self, msg):
		self.msg = msg


def main(argv=None):
	input_file = ""
	
	if argv is None:
		argv = sys.argv
	try:
		try:
			opts, args = getopt.getopt(argv[1:], "hi:", ["help", "input="])
		except getopt.error, msg:
			raise Usage(msg)
	
		# option processing
		for option, value in opts:
			if option == "-v":
				verbose = True
			if option in ("-h", "--help"):
				raise Usage(help_message)
			if option in ("-i", "--input"):
				input_file = value
				
		if input_file == "":
			raise Usage(help_message)
	
	except Usage, err:
		print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
		print >> sys.stderr, "\t for help use --help"
		return 2
	
	problem_size = -1
	old_tour = []
	segments = []
	
	for line in open(input_file, 'r'):
		#try:
			segments = line.split(" ")
			
			if problem_size == -1:
				problem_size = int(segments[0]) / 2
			else:
				for segment in segments:
					try:
						node = int(segment)
			 			old_tour.append(node)
					except:
						pass
					
		#except:
		#	message = "Error reading line '%s'" % line
		#	print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + message
		#	return 2
		
	forward_dir = (old_tour[1] == problem_size)
	new_tour = []
	
	for i in range(0, problem_size):
		if forward_dir:
			node = old_tour[i * 2]
		else:
			node = old_tour[(problem_size - i - 1) * 2]
		new_tour.append(node)
	
	if forward_dir == False:
		new_tour.insert(0, 0)
		new_tour.pop()
		
	print "NAME : %s" % input_file
	print "TYPE : TOUR"
	print "DIMENSION : %d" % problem_size
	print "TOUR_SECTION"
	
	for node in new_tour:
		print node
	print -1

if __name__ == "__main__":
	sys.exit(main())
