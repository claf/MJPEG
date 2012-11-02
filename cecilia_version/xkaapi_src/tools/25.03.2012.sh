#!/bin/bash 

unset KAAPI_CPUCOUNT
unset KAAPI_CPUSET

module load xkaapi-debug

cpus='1 2 4 8'
nb_frame=200
runs='1 2 3 4 5'
video='bb.mjpeg'
sleep_noprio=300
sleep_prio=50

# NUMA placement :
cpuset[1]="0"
cpuset[2]="0,4"
cpuset[3]="0,4,8"
cpuset[4]="0,4,8,12"
cpuset[5]="0,4,8,12,1"
cpuset[6]="0,4,8,12,1,5"
cpuset[7]="0,4,8,12,1,5,9"
cpuset[8]="0,4,8,12,1,5,9,13"




echo "Benchs starting"
  for cpu in $cpus; do
    echo -n "CPU : $cpu ("
    for fps in ${allfpsS[$cpu]}; do
      echo -ne "$fps"
      for run in $runs; do
        echo -ne "."
        echo "using" ${cpuset[$cpu]}
        ./mjpeg_nobuf_noprio -f $fps -s $sleep_noprio -b 1 -n $nb_frame $video > good_file
        rename 's/\.trace$/.$run.trace/' *.trace
        ./mjpeg_noprio -f $fps -s $sleep_noprio -b $nb_frame -n $nb_frame $video > good_file2
        rename 's/\.trace$/.$run.trace/' *.trace
        ./mjpeg_prio -f $fps -s $sleep_prio -b $nb_frame -n $nb_frame $video > good_file3
        rename 's/\.trace$/.$run.trace/' *.trace
      done
    done
    echo -ne ")\n"
done
