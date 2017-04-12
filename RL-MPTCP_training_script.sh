#!/bin/bash
for i in {1..1000}
do
   ./waf --run "scratch/run/run"
  #  echo 'sleep 33 seconds from now'
  #  sleep 33
done
