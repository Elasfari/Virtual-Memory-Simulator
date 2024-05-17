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

`VMsimulator plist ptrace 4 FIFO +`

Which is the compiled program `VMsimulator` simulating the processes in `ptrace` with a page size of 2, and a FIFO page replacement algorithm with pre-paging.
