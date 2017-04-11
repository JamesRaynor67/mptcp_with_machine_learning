#!/bin/bash
for i in {1..5}
do
   ./waf --run "scratch/run/run"
   echo 'sleep 31 seconds from now'
   sleep 31
done
