#!/bin/bash


if [ -z $1 ]
then
  opcontrol --reset
  opcontrol --setup --event=CPU_CLK_UNHALTED:1000000:0:1:1 --no-vmlinux --buffer-size=65536 --buffer-watershed=0 --cpu-buffer-size=0 --callgraph=10 --separate=library
  opcontrol --shutdown
  opcontrol --start
fi

allfps=`seq 182 -2 80`
runs=`seq 1 1 3`

cpuset[2]="0,4"
cpuset[3]="0,4,8"
cpuset[4]="0,4,8,12"
cpuset[5]="0,4,8,12,1"
cpuset[6]="0,4,8,12,1,5"
cpuset[7]="0,4,8,12,1,5,9"
cpuset[8]="0,4,8,12,1,5,9,13"

dir=`date '+%d-%m-%y-%H%M'`
mkdir $dir

export export KAAPI_DISPLAY_PERF=1
export LD_LIBRARY_PATH=/home/claferri/opt/debug/lib/:/home/claferri/opt/lib/
export SDL_VIDEODRIVER=dummy

if [ -z $1 ]
then
  echo "Enable oprofile, disable likwid :"
  #temp
  #likwid="/home/claferri/opt/bin/likwid-perfctr -g CACHE -c"
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
  opcontrol --dump
  opcontrol --shutdown
  opreport -gdf | op2calltree > /dev/null
  mv op*mjpeg* $dir/out.$1.$2.mjpeg && rm oprof.out.*
  if [ -f $dir/out.$1.$2.mjpeg ]
  then
    echo "ok"
  else
    echo "pas de mjpeg"
    exit 0
  fi
  opcontrol --reset
  opcontrol --start
fi
}

function run {
if [ $oprof == 0 ]
then
  $likwid $KAAPI_CPUSET ./mjpeg -f $fps /home/claferri/toto.mjpeg > $dir/trace-"$cpu"cpu-"$fps"fps.$run
else
  ./mjpeg -f $fps /home/claferri/toto.mjpeg > $dir/trace-"$cpu"cpu-"$fps"fps.$run
  #./mjpeg -f $fps toto.mjpeg > trace-"$cpu"cpu-"$fps"fps.$run
fi
if [ -f core ]
then
  mv core $dir/core.$cpu.$fps.$run.core
fi
}

#module -s load xkaapi-bench
ulimit -c unlimited



for fps in $allfps;do
  for cpu in 8 7 6 5 4 3;do
    for run in $runs; do
      export KAAPI_CPUSET=${cpuset[$cpu]}
      echo -e "$fps FPS\t$cpu CPU\trun $run"
      run
    done
    profile $cpu $fps
  done
done
if [ $oprof == 1 ]
then
  opcontrol --stop
fi
