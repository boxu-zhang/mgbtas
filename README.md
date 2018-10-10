# MGBTAS - Machine Gun Ballistic Trajectory Analysis System
This is a system which used to analyze the I/O perfermance of a web server. Here, we imagine a running web server as a shooting game. In this game, each request/response is 'bullet'. Different from the real game, the network 'bullet' has more complex attributes than the real one only having a trajectory cared by normal analysis system. For example: a 'network bullet' has several aspects that this system might care, like: 'latency'(how many time a request is blocked before it actually start), 'execution time'(how many time a request costs to execute), 'thoughput'(how many requests the server handles in a period of time - 'round'), and etc.
Imagine 'a specific type' of the coming requests are fired from a machine gun. We are going to analyze the performance of the gun. We test it in some 'round's. Each 'round' has same length. After the first 'round' complete, we can know the basic information about this gun(this type of inputs). Because the inputs vary and we might want to know the difference between 'round's, we push all 'round's into a chianed list to make it a 'track'

## What is bullet
A bullet is a network request/response pair. It has several attribtues: 'recieved time', 'started time', 'finished time', 'payload length' and 'status'(describe whether the request/response is succeeded). These attributes might not be meaningful enough for this system, and we need to convert it to more meaningful form. Using following fomulas to calculate.
'''
latency     = 'started time' - 'recieved time'  # unit is microseconds(us)
execution   = 'finished time' - 'started time'  # unit is microseconds(us)
payload     = 'payload length' / 1024           # convert unit to kilobytes
'''

While some other aspect the system cares might not avaible for single bullet. We'll talk them later.

## What is trajectory
A trajectory is a curve describes the varies of a sequence of 'bullet's. However, a bullet has more than one attribute that the system cares, and a trajectory can only describe only one of them.

Besides, trajectory provides additional attribtues the system wants, listed below.
'''
total_inputs = count(bullet) # unit is times
total_time = (first(bullet)->'recieved time' - last(bullet)->'finished time') / (1000 * 1000) # unit is second(s)

total_latency   = sum(latency)
maximum_latency = max(latency)
minimum_latency = min(latency)
average_latency = average(latency)

total_execution = sum(execution)
maximum_execution = max(execution)
minimum_execution = min(execution)
average_execution = average(latency)

total_payload = sum(payload)
maximum_payload = max(payload)
minimum_payload = min(payload)
average_payload = average(payload)

throughput = total_inputs / total_time # unit is 'times per second'(t/s)

bandwidth = total_payload / total_time # unit is 'kilobytes per second'(kb/s)
'''

## What is round
A round is a period of time that the system counts 'bullet's and calculates 'trajectory's. It's a fixed time and can be adjust through predefined processor - 'MGBTAS_DEFAULT_ROUND' in unit of seconds(s). Default is 60s.

## What is track
A track saves some round information and interface with user to fetch or dump the analysis report. A track cannot grow infinitely, like 'round', the maximum count of 'round' that a track can has can be adjust through predefined processor - 'MGBTAS_MAX_ROUNDS'. Default is 60. So the maximum time a track can analyze is 60 * 60s = 3600s(1 hour).

## What is game
A game demonstrates a simplest way to manage tracks in host application and exhibits the usage of 'mgbtas' library. It is located at 'mgbtas_demo.h' & 'mgbtas_demo.cpp'.

## Appendix
Information that developer might need to know.

### Unit
'mgbtas' has predefined the internal data unit. For example, unit of 'payload_length' is 'kb = 1024 bytes', unit of time is microsecond 'us' and unit of 'bullet' is 'times'. However, these unit are not appropriate for display. So, it provide some interface for a nicer display string of both inputing parameters and outputing results. See API for detail.
