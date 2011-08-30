#!/usr/bin/env python
# encoding: utf-8

# packages and configuration
import sys
import random
import math

def usage ():
   print "convert.py tracefile"
   print "\tinput file need to be formated like that :"
   print "\t#nb_frame start_time end_time operation"
   print "\toperation may be : decoded!, resized!, printed! and dropped!"

def header ():
   f = file("header.txt")
   print f.read()

def hierarchy ():
   print "0 F 0 Frame"
   print "1 S F State"
   print "2 decode S Decoded \"0 1 0\""
   print "2 resize S Resized \"0 0 1\""
   print "2 print S Printed \"1 1 0\""
   print "2 drop S Dropped \"1 0 0\""
   print "2 fetch S Fetch \"0 1 1\""
   print "84 decode F Decoded \"0 1 0\""
   print "84 resize F Resized \"0 0 1\""
   print "84 print F Printed \"1 1 0\""
   print "84 drop F Dropped \"1 0 0\""
   print "84 fetch F Fetch \"0 1 1\""
   print "84 next F Next \"1 1 1\""

def translate (f):
   global drop
   for line in f:
      sline = line.replace("[","").replace("]","").split()
      frame_str = sline[0]
      if not frame_str.startswith("Frame"):
        if frame_str.startswith("#drop"):
          drop = int(sline[3])
          if drop==-1:
            drop=0
        continue
      start_str = sline[1]
      if len(sline) == 3:
        op_str = sline[2]
        if frame_str not in created_containers:
          print "4", start_str, frame_str, "F", "0", frame_str
          created_containers[frame_str] = 1
        print "20", start_str, op_str, frame_str, "o"
      else:
        end_str = sline[2]
        op_str = sline[3]
        # CreateContainer
        if frame_str not in created_containers:
          print "4", start_str, frame_str, "F", "0", frame_str
          created_containers[frame_str] = 1
        # PushState and PopState
        if op_str == "print" or op_str == "next":
          print "20", start_str, op_str, frame_str, "o"
        else:
          print "9", start_str, "S", frame_str, op_str
          print "10", end_str, "S", frame_str

#starts here
# the script starts here
if len(sys.argv) == 1:
   usage ()
   sys.exit()
else:
   f = open (sys.argv[1],'r')
   drop = 5 * 2
   created_containers = dict()
   header()
   hierarchy ()
   translate (f)
   sys.exit (drop)
