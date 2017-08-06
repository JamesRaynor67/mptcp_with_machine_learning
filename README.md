# mptcp_with_machine_learning

How to run (in root directory of ns3):

1.configure compile condition
```
CXXFLAGS=" -std=c++11" ./waf --enable-examples configure
```
2.compile
```
./waf
```
3.1. To research into NS3 and force using one specific scheduler, run script 
```
bash -x RL-MPTCP_run_sets.sh
```
or
```
bash -x RL-MPTCP_run_Expxx-xx.sh
```
3.2. To train RL model and test on trainned model, run script
```
bash -x RL-MPTCP_training.sh
```
or
3.3 To train RL model with various settings for a long time (over night, for example), configure and run script
```
bash -x RL-MPTCP_scripts_for_night.sh
```

(Some paths is hard coded, generally, you can do grep -rn "/home/hong" to find them out and replace them)

Env:
gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04.3)
ubuntu 14.04: kernel version 4.2.0-27-generic
Python 2.7.12 | Anaconda 4.2.0 (64-bit)
Tensorflow 0.12.0

Details:
---
1. The MpTCP applications (on-off/file transfer) in scratch/run/mp-onoff-application.cc only use multiple IP addresses of server.
    * Example: let a server A own IP: 192.168.0.1 and 192.168.0.2, a client B own IP: 192.168.0.3 and 192.168.0.4. Then, the whole process is like this:
    1. Server A establishes the master subflow of MpTCP to B with socket (192.168.0.1, xxxx, 192.168.0.3, yyyy)
    2. After master subflow is fully established (Sever A gets first DSS from Client B), server A starts to check a new subflow is available or not. If available, A sends MP_JOIN through address 192.168.0.2:zzzz back to B's 192.168.0.3:yyyy to request a new subflow.
    3. Then, B does a simple one way hand shake with A's 192.168.0.2:zzzz through address 192.168.0.3:yyyy to establish socket (192.168.0.2, zzzz, 192.168.0.3, yyyy). Security is promised by token/nonce check.

For detailed logic: MpOnOffApplication::ConnectionFullyEstablished()

And this specific simulation can be regarded as IPhone use multiple subflows to connect to Siri's server.

>               Host A                                 Host B
>      ------------------------                      ----------
>      Address A1    Address A2                      Address B1
>      ----------    ----------                      ----------
>      |             |                                |
>      |            SYN + MP_CAPABLE(Key-A)           |
>      |--------------------------------------------->|
>      |<---------------------------------------------|
>      |          SYN/ACK + MP_CAPABLE(Key-B)         |
>      |             |                                |
>      |        ACK + MP_CAPABLE(Key-A, Key-B)        |
>      |--------------------------------------------->|
>      |             |                                |
>      |             |   SYN + MP_JOIN(Token-B, R-A)  |
>      |             |------------------------------->|
>      |             |<-------------------------------|
>      |             | SYN/ACK + MP_JOIN(HMAC-B, R-B) |
>      |             |                                |
>      |             |     ACK + MP_JOIN(HMAC-A)      |
>      |             |------------------------------->|
>      |             |<-------------------------------|
>      |             |             ACK                |
>
>      HMAC-A = HMAC(Key=(Key-A+Key-B), Msg=(R-A+R-B))
>      HMAC-B = HMAC(Key=(Key-B+Key-A), Msg=(R-B+R-A))

2. On-off application put data into txBuffer at configured rate during on-period. If txBuffer is full, application just fails to put data into txBuffer, which means these data even didn't become packets.

3. The large amount of bash scripts in root dir is for automatic configuration of experiments for long time.
If all parameters of experiemnts can be passed as parameters, we don't need this ugly implementation.
However, this is the easiest way to configure various expriemtents for a long time.

4. This is part of the graduation thesis of Hong in UTokyo, in 2017.
