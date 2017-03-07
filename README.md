# Weighted Round Robin

## Introduction

Round Robin (RR) arbitration is a common scheme used to fairly distribute work
across a number of agents. A number of agents request access to some shared
resource and are in turn granted access in a circular manner.

For the majority of day-to-day arbitration, fixed priority arbitration schemes
are sufficient. They are limited however by the fact that all requestors have
fixed priority and therefore response with equal weighting to all
requestors. For some use cases, it is desireable to weight certain requestors
over others. There are a variety of ways that this can be achieved.

* __Duplicated request lines__ High priority agents can be allocated more than
  one request line into the arbiter. By consequence, a single requesting agent
  may be granted more frequently than others requestors. This scheme, although
  simple, is inherently static in nature, and there is no way to dynamically
  alter weights of agents in situ (at least trivially).
* __Request Counters__ Alternatively, request can be masked against a
  counter. For each grant, the request counter is decremented. When the count
  reaches zero, the agent is no longer eligible for arbitration. Such a process
  continues until all counters have been exhausted. This scheme, although
  dynamic, does not ensure fairness between agents, as grants are not equally
  distributed over time and requestor may experience long latency between
  requests.
* __Dynamically Weighted Arbiters [PDF][1]__ Finally, requestors can be
  attributed binary priorites. Arbitration procedes by walking through indexes
  in the priority range, and performing standard RR arbitration between
  requestors in a particular round. This is the approach in this implementation
  and is presented formally in the reference.

## System Requirements
* cmake >= 3.2
* systemc >= 2.3.1
* verilator >= 3.9
* clang >= 3.9

## Build Steps
~~~~
git clone https://github.com/stephenry/weighted_round_robin.git
cd linked_list_queue
git submodule update --init --recursive
cmake ../
make
~~~

## Parameterizations

* __N__ The number of requestors.

* __PRIORITY_W__ The width (in bits) of the priority assigned to each requestor.

## Memory Requirements

All internal state is retained in FLOPS are as such there are no dependencies
upon external RAM macros.

## Micro-architecture

Priorities are uploaded using the PRIO interface. There are no
micro-architectural limitations upon when priorities may be modified however the
effect that this has on arbitration over the short-term is undefined.

The micro-architecture of the arbitration mechanism is discussed in detail
in [PDF][1] and therefore shall not be repeated here.

## Error handling

There are no error conditions.

## Performance

Requests are update on a cycle-by-cycle basis. Grants are retained until
explicitly acknowledged by the external agent.

## Verification Methodology

The verification environment is self-checking and verifies the following
properties:

* No grant is made to a requestor that is not asserted.

* No grant is made to a requestor that as nil priority (disabled).

For a given input distribution, grant count is tabulated and graphed. The
expected number of grants over the period of interest can be computed statically
and is checked against the measured result.

## References

[1]: http://ieeexplore.ieee.org/document/105173/?tp=&arnumber=105173&url=http:%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D105173
