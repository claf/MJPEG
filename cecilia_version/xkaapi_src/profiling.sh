#!/bin/bash

DATE=`date '+%H:%M-%d-%m-%y'`
sudo opcontrol --stop
sudo opcontrol --reset
sudo opcontrol --start
#./value.sh > tmp_profiling_resume_$DATE
time ./mjpeg -f 2000 ~/toto.mjpeg #~/toto.mjpeg ~/toto.mjpeg >> tmp_profiling_resume_$DATE
sudo opcontrol --stop

opreport -cl --demangle=smart /home/claferri/dev/MJPEG/cecilia_version/xkaapi_src/mjpeg > tmp_profiling_mjpeg_$DATE

opreport --symbols --debug-info ~/opt/debug/lib/libkaapi.so.0.0.0 > tmp_profiling_libkaapi_$DATE
