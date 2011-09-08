#!/bin/bash

module -s load xkaapi-bench


#echo "Systeme configuration :"
#echo -e "CPU count\t\t: $KAAPI_CPUCOUNT"
#echo -e "CFLAGS\t\t\t: $CFLAGS"
#echo -e "SDL_VIDEODRIVER\t\t: $SDL_VIDEODRIVER"
#echo -e "KAAPI_DISPLAY_PERF\t: $KAAPI_DISPLAY_PERF"
#ulimit -c unlimited


for run in 1 2 3 4 5 6 7 8 9; do
  for cpu in $1; do
    KAAPI_CPUCOUNT=$cpu
    for i in `seq $2 1 $2`; do
      ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    done
  done
done 

