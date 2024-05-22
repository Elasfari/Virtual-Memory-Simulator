# Virtual Memory Simulator Project
## introduction

### Virtual Memory Simulator
This virtual memory simulator models the behavior of different page replacement algorithms. It helps in understanding how an operating system manages memory and the impact of various algorithms on system performance.

## Page Replacement Algorithms
### FIFO (First-In, First-Out)
The FIFO page replacement algorithm removes the oldest page in memory when a new page needs to be loaded.
### LRU (Least Recently Used)
The LRU algorithm replaces the page that has not been used for the longest period of time.
### CLOCK (Second Chance)
The CLOCK algorithm gives each page a second chance by using a circular list and a reference bit to decide which page to replace.
### Optimal
The Optimal page replacement algorithm replaces the page that will not be used for the longest time in the future, achieving the lowest possible page-fault rate.
## Description

The goal of this project is to simulate a virtual memory management system. The compiled program accepts the following
parameters at the command prompt in the order specified:

1. plist (the name of the process list file)
2. ptrace (the name of the process execution trace file)
3. page size (# of memory locations for each page)
4. FIFO, LRU, or Clock (type of page replacement algorithm)
  - FIFO: first-in first-out
  - LRU: least recently used
  - Clock: clock algorithm
  - Optimal: optimal algorithm
5. A `+` or `-` (a flag to toggle a pre-paging feature)
  - `+`: turn pre-paging on
  - `-`: leave default demand paging

Two files, `plist` and `ptrace` are provided this repo, and can be used for testing. An example command looks like this:

`./VMsimulator plist ptrace 4 FIFO +`

Which is the compiled program `VMsimulator` simulating the processes in `ptrace` with a page size of 4, and a FIFO page replacement algorithm with pre-paging.
