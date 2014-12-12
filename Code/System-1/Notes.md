I began to ask this question about the scheduling system on StackExchange, and partway through the writeup, I basically realised I was discussing the differences between cooperative multitasking and pre-emptive multitasking.

Since a single thread failing will tank the whole process, I've decided on a cooperative approach. 

###Approaches for handling complex scheduling requirements in a system involving many separate processes?

I'm working on designing a scheduling system that has to manage some complex, partially synchronous control state across a system that has a great many independent processes (it's a radio-telescope data-acquisition system, actually).

Basically, it requires a number of modules be cycled through a number of distinct steps on a repeating basis. 

 - Acquiring data (~30 minutes):
    - There are multiple separate processes that handle stream processing and write data out to file
    - Runs in a chunked basis, in ~30 second spans, which are challenging to interrupt.
 - Hardware calibration (short)
 - Antenna calibrarion (also short)
 - Other assorted housekeeping (pointing, etc...).

The critical thing is that I have to interrupt the main acquisition to perform calibration, and I'm trying to determine what the best approach to use for the system topology is.

 - Bottom-up scheduling:
    - Processes manage their own internal state, and yield flow back to a simple global loop periodically.
    - Effectively cooperative multitasking
 - Top-down scheduling:
    - State is managed in a global 