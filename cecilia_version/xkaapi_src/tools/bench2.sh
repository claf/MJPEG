#!/bin/bash

module -s load xkaapi-bench


echo "Systeme configuration :"
echo -e "CPU count\t\t: $KAAPI_CPUCOUNT"
echo -e "CFLAGS\t\t\t: $CFLAGS"
echo -e "SDL_VIDEODRIVER\t\t: $SDL_VIDEODRIVER"
echo -e "KAAPI_DISPLAY_PERF\t: $KAAPI_DISPLAY_PERF"
ulimit -c unlimited



for run in 1 2 3 4 5 6 7 8 9; do
    # KAAPI_CPUCOUNT=2
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 42 1 50`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    KAAPI_CPUCOUNT=3
    cpu=$KAAPI_CPUCOUNT
    for i in `seq 76 1 80`; do
      ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    done
    # KAAPI_CPUCOUNT=4
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 110 1 120`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=5
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 140 1 150`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=6
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 165 1 175`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=7
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 165 1 180`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=8
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=9
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=11
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=13
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=15
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
    # KAAPI_CPUCOUNT=17
    # cpu=$KAAPI_CPUCOUNT
    # for i in `seq 198 2 220`; do
    #   ./mjpeg -f $i ~/toto.mjpeg > trace-"$cpu"cpu-"$i"fps.$run
    # done
done 

