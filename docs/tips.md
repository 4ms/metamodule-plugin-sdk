# Tips


## Allocations

The MetaModule has hard real-time requirements in order to acheive low-latency audio. 

There is an audio "thread" which has the highest priority of any task. The audio thread
calls your module's `process()` function for each sample frame. Therefore any code in the `process()` 
function must be optimized for efficient execution.
 
This means you cannot make any memory allocations in the `process()` function (or any other
function that's called by `process()`. This includes:

- Do not create, destroy, or re-size a `std::string`. Writing characters to an already allocated
string is OK, as long as it does not change length.

- Do not `push_back` or `emplace_back` or otherwise change the length of a
  std::vector (or any std container such as list, deque, etc... You can
  read/write data in a vector, but don't change the capacity of the vector. If
  you do need a dynamically-sized vector in the audio thread, one useful
  technique is to use `reserve()` when you construct your module. Then you can safely
  call `push_back` in the audio thread as long as you are 100% certain you will
  not push more items than you reserved.


I've seen a few examples of VCV Rack plugins that allocate memory in the audio thread. The most common
example is allocating channels when a cable is patched into a jack. Often, the number of polyphonic
channels is read from the port and then a vector of some data structure is allocated to accomodate that 
many poly channels. 
This is not allowed in the MetaModule. I suspect it's OK in VCV Rack because typically Rack is run
on modern computers which can handle spikes in CPU usage coming from allocations. But the MetaModule
will behave erratically, sometimes doing OK and sometimes failing when you patch a cable.

In order to address this pattern in several VCV Rack modules, the MetaModule runs `process` once
for each module when it first loads the patch. Sometimes modules will allocate only if they detect
their data structures have not been set up.

If you are designing a new module, I would put forth a suggestion to set up all data structures
in the module constructor. That includes anything that requires a heap allocation. Use `reserve()`
if you still need dynamic re-sizing.

Then, in the `process()` function, make sure there is no situation in which a memory allocation 
can happen.


## Block sizes

The minimum block size the MetaModule supports is 16 frames. So if you are optimizing your module
to process in blocks, this would be the ideal size to use. 

Using a larger size would mean the CPU usage would spike, which can be upsetting and confusing to users.
