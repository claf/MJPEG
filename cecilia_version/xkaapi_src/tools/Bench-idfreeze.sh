#!/bin/bash

runs='1'

export export KAAPI_DISPLAY_PERF=1
export LD_LIBRARY_PATH=/home/claferri/opt/debug/lib/:/home/claferri/opt/lib/
export SDL_VIDEODRIVER=dummy

if [ -z $1 ]
then
  echo "Enable oprofile, disable likwid :"
  likwid=""
  oprof=1
else
  echo "Enable likwid, disable oprofile :"
  likwid="/home/claferri/opt/bin/likwid-perfctr -g CACHE -c"
  oprof=0
fi

function profile {
if [ $oprof == 1 ]
then
  opcontrol --stop
  opreport -gdf | op2calltree
  mv op*mjpeg* out.$1.$2.mjpeg
  rm oprof.out.*
  opcontrol --reset
  opcontrol --start
else
  echo "$1 runs terminated"
fi
}

function run {
if [ $oprof == 0 ]
then
  $likwid $KAAPI_CPUSET ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
else
  $likwid ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
fi
}

#module -s load xkaapi-bench
ulimit -c unlimited


if [ -z $1 ]
then
  opcontrol --reset
  opcontrol --start
else
  echo "Start :"
fi

#cpu=8
#export KAAPI_CPUSET=0,4,8,12,1,5,9,13
#for fps in `seq 200 1 200`; do
#    for run in $runs; do # 2 3 4 5 6 7 8 9; do
#	$likwid ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done
#
#cpu=6
#for fps in `seq 142 1 142`; do
#    for run in $runs; do
#	$likwid ./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
#    done
#    profile $cpu $fps
#done


cpu=4
export KAAPI_CPUSET=0,4,8,12
for fps in `seq 111 1 111`; do
    for run in $runs; do
      run
    done
    profile $cpu $fps
done

cpu=3
export KAAPI_CPUSET=0,4,8
for fps in `seq 76 1 76`; do
    for run in $runs; do
      run
    done
    profile $cpu $fps
done

cpu=2
export KAAPI_CPUSET=0,4
for fps in `seq 43 1 43`; do
    for run in $runs; do
      run
    done
    profile $cpu $fps
done

cpu=5
export KAAPI_CPUSET=0,4,8,12,16
for fps in `seq 111 1 111`; do
    for run in $runs; do
      run
    done
    profile $cpu $fps
done

opcontrol --stop
