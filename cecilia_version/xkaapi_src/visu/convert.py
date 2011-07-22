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
   print "2 decoded! S Decoded \"0 1 0\""
   print "2 resized! S Resized \"0 0 1\""
   print "2 printed! S Printed \"1 1 0\""
   print "2 dropped! S Dropped \"1 0 0\""

def translate (f):
   for line in f:
      sline = line.replace("[","").replace("]","").split()
      frame_str = sline[0]
      start_str = sline[1]
      end_str = sline[2]
      op_str = sline[3]
      # CreateContainer
      if frame_str not in created_containers:
        print "4", start_str, frame_str, "F", "0", frame_str
        created_containers[frame_str] = 1
      # PushState and PopState
      print "9", start_str, "S", frame_str, op_str
      print "10", end_str, "S", frame_str

#starts here
# the script starts here
if len(sys.argv) == 1:
   usage ()
   sys.exit()
else:
   f = open (sys.argv[1],'r')
   created_containers = dict()
   header()
   hierarchy ()
   translate (f)
