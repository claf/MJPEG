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


opcontrol --stop
opcontrol --reset
opcontrol --start

# KAAPI_CPUCOUNT=2
# cpu=$KAAPI_CPUCOUNT
# for fps in `seq 22 1 24`; do
#     for run in 1 2 3 4 5 6 7 8 9; do
# 	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/titi.mjpeg ~/toto.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
#     done
#     profile $cpu $fps
# done

# cpu=3
# KAAPI_CPUCOUNT=$cpu
# for fps in `seq 38 1 42`; do
#     for run in 1 2 3 4 5 6 7 8 9; do
# 	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg ~/titi.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
#     done
#     profile $cpu $fps
# done


# cpu=4
# KAAPI_CPUCOUNT=$cpu
# for fps in `seq 55 1 60`; do
#     for run in 1 2 3 4 5 6 7 8 9; do
# 	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg ~/titi.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
#     done
#     profile $cpu $fps
# done

cpu=5
KAAPI_CPUCOUNT=$cpu
for fps in `seq 70 1 75`; do
    for run in 1 2 3 4 5 6 7 8 9; do
	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg ~/titi.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
    done
    profile $cpu $fps
done

# cpu=6
# KAAPI_CPUCOUNT=$cpu
# for fps in `seq 80 1 95`; do
#     for run in 1 2 3 4 5 6 7 8 9; do
# 	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg ~/titi.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
#     done
#     profile $cpu $fps
# done

# cpu=8
# KAAPI_CPUCOUNT=$cpu
# for fps in `seq 100 1 105`; do
#     for run in 1 2 3 4 5 6 7 8 9; do
# 	KAAPI_CPUCOUNT=$cpu ./mjpeg -f $fps ~/toto.mjpeg ~/titi.mjpeg > doubletrace-"$cpu"cpu-"$fps"fps.$run
#     done
#     profile $cpu $fps
# done

opcontrol --stop

