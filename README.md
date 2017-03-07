# mptcp_with_machine_learning

How to run (in root directory of ns3):

configure compile condition
1. CXXFLAGS=" -std=c++11" ./waf --enable-examples configure

compile
2. ./waf

mock machine learning process and make it run in background
3. python machineLearning/server_example.py &

run mptcp test and communicate with machine learning process
4. ./waf --run "scratch/run/run --outputDir=./mptcp_output"

PS. Because of the modification of tcp part, some irrelevant tests of original 300+ test will fail if try to compile and run original test suit.

Env:
gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04.3)
ubuntu 14.04: kernel version 4.2.0-27-generic
Python 2.7.12 | Anaconda 4.2.0 (64-bit)
