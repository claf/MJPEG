#!/bin/bash

function profile {
    opcontrol --stop
    opreport -gdf | op2calltree
    mv op*mjpeg* out.$1.$2.mjpeg
    rm oprof.out.*
    opcontrol --reset
    opcontrol --start
}

#module -s load xkaapi-bench
ulimit -c unlimited


opcontrol --reset
opcontrol --start

KAAPI_CPUCOUNT=2
cpu=$KAAPI_CPUCOUNT
for fps in `seq 30 5 45`; do
    for run in 1 2 ;do #3 4 5 6 7 8 9; do
	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

cpu=3
KAAPI_CPUCOUNT=$cpu
for fps in `seq 40 5 70`; do
    for run in 1 2 ;do #3 4 5 6 7 8 9; do
	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

#cpu=4
#KAAPI_CPUCOUNT=$cpu
#for fps in `seq 111 1 113`; do
#    for run in 1 2 3 4 5 6 7 8 9; do
#	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done
#
#cpu=6
#KAAPI_CPUCOUNT=$cpu
#for fps in `seq 140 1 170`; do
#    for run in 1 2 3 4 5 6 7 8 9; do
#	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done
#
#cpu=8
#KAAPI_CPUCOUNT=$cpu
#for fps in `seq 199 1 201`; do
#    for run in 1 2 3 4 5 6 7 8 9; do
#	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done

opcontrol --stop

