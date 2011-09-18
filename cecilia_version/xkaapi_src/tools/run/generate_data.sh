#!/bin/bash

if [ -z $1 ];
then
  echo "Usage ./$0 output_dir"
  exit 1
else
  dir=$1
fi

cpus="2 3 4 5 6 7 8"


for cpu in $cpus; do
  files[$cpu]=$dir/data$cpu.cpu.data
  echo "Generating data for $cpu threads"
  cd $dir
  grep "sched idle" *$cpu*cpu* | perl -pe 's/trace-\dcpu-(\d+)fps.*\s:\s(\d+)/$1 $2/' > ../${files[$cpu]}.tmp
  sort -g ../${files[$cpu]}.tmp > ../${files[$cpu]} && rm ../${files[$cpu]}.tmp
  #cg "sched idle" *$cpu*cpu* | cut -d "-" -f 3 | awk '{print $1 " " $7}' | cut -c 1-2,8-25 > ../${files[$cpu]}
  cd ../
done

gnuplot << EOF
set terminal postscript eps color
#set terminal png
set output 'sched_idle_tot.eps'
set ylabel "secondes"
set xlabel "framerate"
set style data linespoints

plot '${files[2]}', '${files[3]}', '${files[4]}', '${files[5]}', \
  '${files[6]}','${files[7]}','${files[8]}';

#plot 'data.data' using 1:4, 6.3*(x-1)-3, 'data.data' using 1:2

#plot 'data2cpu.data', './data3cpu.data', './data4cpu.data', './data5cpu.data',
#'./data6cpu.data', './data7cpu.data', './data.data' using 2:4

EOF

gnuplot << EOF
set terminal postscript eps color
#set terminal png
set output 'sched_idle.eps'
set ylabel "secondes"
set xlabel "framerate"
set style data linespoints

plot '${files[2]}', '${files[3]}' using 1:(\$2/2), '${files[4]}' using 1:(\$2/3), '${files[5]}' using 1:(\$2/4), \
  '${files[6]}' using 1:(\$2/5),'${files[7]}' using 1:(\$2/6),'${files[8]}' using 1:(\$2/7);

#plot 'data.data' using 1:4, 6.3*(x-1)-3, 'data.data' using 1:2

#plot 'data2cpu.data', './data3cpu.data', './data4cpu.data', './data5cpu.data',
#'./data6cpu.data', './data7cpu.data', './data.data' using 2:4

EOF
