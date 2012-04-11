#!/bin/bash 

#cpus=`seq 2 1 8`
cpus='8'

#bufSize=`seq 1 1 30`
bufSize='5'

runs=`seq 0 1 0`

video='/home/claferri/toto.mjpeg'

allfps[2]='45 46 47 48'
#allfps[3]='83 84 85 86'
allfps[3]=`seq 83 1 83`

allfps[4]='111 112 113 114'
allfps[5]='142 143 144 145'
allfps[6]='166 167 168'
allfps[7]='200 201 202'
allfps[8]='150'
#allfps[8]='200 201 202 203'

########
# OPTS #
########

oprof=0
likwid=0
sets=0

while getopts "hpold:-:" OPT
do
  # puis gestion des options courtes
  case $OPT in
    o) echo "Oprofile"
      if [ $likwid == '1' ]
      then
        echo "Can't use both Likwid and Oprofile, make your choice"
        exit 1
      fi
      likwid=0
      oprof=1 ;;
    l) echo "Likwid"
      if [ $oprof == '1' ]
      then
        echo "Can't use both Likwid and Oprofile, make your choice"
        exit 1
      fi
      oprof=0
      likwid=1;;
    d) echo "Output directory set to $OPTARG"
      dir=$OPTARG;;
    p) echo "Threads placement using KAAPI_CPUSET"
      sets=1;;
    h) echo "$(basename $0) [-o | -l] [-d output_dir] [-p]"
      echo -e "\t-o for oprofile"
      echo -e "\t-l for likwid"
      echo -e "\t-p for cpuset placement"
      exit 1;;
  esac
done

#########
# CONST #
#########

if [ -z $dir ]; then
  dir=`date '+%d-%m-%y-%H%M'`
fi

# Gestion des threads :
if [ $sets == '0' ]
then
  cpuset[2]="KAAPI_CPUCOUNT=2"
  cpuset[3]="KAAPI_CPUCOUNT=3"
  cpuset[4]="KAAPI_CPUCOUNT=4"
  cpuset[5]="KAAPI_CPUCOUNT=5"
  cpuset[6]="KAAPI_CPUCOUNT=6"
  cpuset[7]="KAAPI_CPUCOUNT=7"
  cpuset[8]="KAAPI_CPUCOUNT=8"
else
  cpuset[2]="0,4"
  cpuset[3]="0,4,8"
  cpuset[4]="0,4,8,12"
  cpuset[5]="0,4,8,12,1"
  cpuset[6]="0,4,8,12,1,5"
  cpuset[7]="0,4,8,12,1,5,9"
  cpuset[8]="0,4,8,12,1,5,9,13"
fi

########
# INIT #
########

ulimit -c unlimited

mkdir $dir

export KAAPI_DISPLAY_PERF=1
export SDL_VIDEODRIVER=dummy
export LD_LIBRARY_PATH=/home/claferri/opt/debug/lib/:/home/claferri/opt/lib/

if [ $oprof == '1' ]
then
  opcontrol --reset
  opcontrol --setup --event=CPU_CLK_UNHALTED:1000000:0:1:1 \
    --event=DATA_CACHE_MISSES:1000000 --no-vmlinux \
    --buffer-size=100000 --buffer-watershed=25000 --cpu-buffer-size=0 \
    --callgraph=10 --separate=library 
  opcontrol --shutdown
  opcontrol --start
  echo "Enable oprofile, disable likwid :"
fi

if [ $likwid == '1' ]
then
  echo "Enable likwid, disable oprofile :"
  likwid_cmd="/home/claferri/opt/bin/likwid-perfctr -g CACHE -c"
fi

#############
# FUNCTIONS #
#############

function profile {
if [ $oprof == '1' ]
then
  opcontrol --stop
  opcontrol --dump
  opcontrol --shutdown
  opreport -gdf | op2calltree
  mv op*mjpeg* $dir/out.$cpu.$fps.mjpeg && rm oprof.out.*
  if [ -f $dir/out.$cpu.$fps.mjpeg ]
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
file=$dir/trace-"$cpu"cpu-"$fps"fps-"$buffer"fb.$run
if [ $likwid == '1' ]
then
  if [ $sets == '1' ]
  then
    #TODO test
    $likwid_cmd 0-47 KAAPI_CPUSET=${cpuset[$cpu]} ./mjpeg -b $buffer -f $fps $video > $file
  else
    $likwid_cmd 0-47 KAAPI_CPUCOUNT=$cpu ./mjpeg -b $buffer -f $fps $video > $file
  fi
else
  export ${cpuset[$cpu]}
  ./mjpeg -b $buffer -f $fps $video > $file
fi
if [ -f core ]
then
  mv core $dir/core.$cpu.$fps.R$run.B$buffer.core
fi
}

#########
# BENCH #
#########

echo "Benchs starting"
for buffer in $bufSize; do
  echo -ne "\nBuffer Size : $buffer\n"
  for cpu in $cpus; do
    echo -n "CPU : $cpu ("
    for fps in ${allfps[$cpu]}; do
      echo -ne "$fps"
      for run in $runs; do
        echo -ne "."
        run
      done
      profile $cpu $fps
    done
    echo -n ")"
  done
done

########
# TERM #
########

if [ $oprof == '1' ]
then
  opcontrol --stop
fi
