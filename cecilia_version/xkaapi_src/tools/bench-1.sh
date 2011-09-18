#!/bin/bash

export SDL_VIDEODRIVER=dummy
export KAAPI_DISPLAY_PERF=1
export LD_LIBRARY_PATH=/home/claferri/opt/debug/lib/:/home/claferri/opt/lib/

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


cpu=2
for fps in `seq 33 1 33`; do
    for run in 1 2 3 4 5;do
	KAAPI_CPUSET=0~0,1~1 ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
	mv trace.trace paje-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

cpu=3

for fps in `seq 42 1 42`; do
    for run in 1 2 3 4 5;do
	KAAPI_CPUSET=0~0,1~1,2~2 ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
	mv trace.trace paje-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

cpu=4

for fps in `seq 44 1 44`; do
    for run in 1 2 3 4 5;do
	KAAPI_CPUSET=0~0,1~1,2~2,3~3 ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
	mv trace.trace paje-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

cpu=6

for fps in `seq 46 1 46`; do
    for run in 1 2 3 4 5;do
	KAAPI_CPUSET=0~0,1~1,2~2,3~3,4~4,5~5 ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
	mv trace.trace paje-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

#cpu=8
#
#for fps in `seq 48 1 48`; do
#    for run in 1 2 3 4 5;do
#	KAAPI_CPUSET=0~0,1~1,2~2,3~3,4~4,5~5,6~6,7~7 ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#	mv trace.trace paje-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done

opcontrol --stop

