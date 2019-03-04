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
3.run script
```
bash RL-MPTCP_debug_NS3.sh
```
P.S.
```
bash RL-MPTCP_run.sh
```
runs multiple python program for statistic and analyzing. But lots of path for
file storage in them is hard coded and only fits for my machine, my bad :(

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

P.S.有问题可以中文交流，对大家都方便。

P.P.S.这个ns3的mptcp模拟还是不完全的，我记忆中问题最大的一处是数据包选择链路A发送后是无法撤销然后选择链路B的。所以会导致当一条链路中断长时间时，整个传输被卡住。我尝试解决但是最后还是放弃了（因为我重点放在了机器学习方面）。你们如果解决了请告诉我，谢谢！
