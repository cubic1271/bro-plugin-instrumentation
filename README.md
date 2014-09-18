
# Bro Instrumentation Plugin #

## Overview ##

The bro instrumentation plugin is intended to offer some insight into what exactly bro is doing and how, exactly, it's spending its time.  To that effect, the plugin offers three different modes of operation:

* Occasional collection - this mode pauses either every X packets or Y seconds of network time to write statistics regarding the number of allocations, the number of filesystem reads / writes, and the total number of cycles spent processing packets (via RDTSC).
* Per-function statistics - this mode queries counters at the beginning and end of each function call, and uses the collected statistics to record information about how much overhead is associated with each specific function
* Call-graph - this mode generates a call-graph that can be parsed by graphviz (preferably 'dot')

## Building ##

* build the syshook shared libraries (via 'make preload')
* make and install the plugin (via 'make install')

It is recommended that (and, on OS/X, required that) the syshook libraries are linked into the build process when the bro binary is constructed instead of being loaded via LD_PRELOAD.  To do this, begin by ensuring that there is *no existing build/ directory in the bro source tree*.  Next, set the following environment variable:

```bash
LDFLAGS='-lsyshook-io -lsyshook-malloc -L/path/to/syshook/libraries'
```

Now, configure and build bro:

```bash
./configure --prefix=/usr/bro-instrumented
make clean && make -j6
sudo make install
```

Assuming all goes according to plan, CMake will grab the LDFLAGS items and append them to the build string for the linking phase of all libraries / binaries built during the build process.

After bro has been linked, use 'otool' (on OS/X) or 'ldd' (on Linux / FreeBSD) to ensure that the syshook-io and syshook-malloc libraries appear in the list of dependencies.

*NOTE*: Linking against one / both of these shared libraries will _always_ incur overhead (even when the instrumentation plugin is not loaded), as these libraries intercept *all* calls to malloc / read / write and increment counters when they are called.  In the case of malloc, this consists of an increment to a thread-local variable (the __thread storage class).  In the case of file operations, this consists of an increment to one or more C++11 std::atomic counter(s).  In both cases, there is additional overhead involved with additionally invoking the method referenced by RTLD_NEXT.

## Using ##

This plugin can collect information in three different forms:

* Dump a collection of statistics after observing X packets / after Y seconds have elapsed.  This is enabled via the Instrumentation::(Set)?Collection* set of methods.
* Track statistics per-function / per-event.  This is enabled via the Instrumentation::(Set)?Function* set of methods.
* Build call-graphs (meant to be parsed by the graphviz 'dot' utility) to visualize what the code is doing.  This is enabled via the Instrumentation::(Set)?ChainData* set of methods.

An example of writing statistics to a file called '/tmp/collection.out' for every 10000 observed packets:

```
event bro_init() {
	# Write a new statistics line once for every 10000 packets we see ...
    Instrumentation::SetCollectionCount(10000);
    # ... to the file '/tmp/collection.out'
    Instrumentation::SetCollectionTarget("/tmp/collection.out");
}

event bro_done() {
	# Make sure everything has been written to the collection file since we're getting ready to exit
    Instrumentation::CollectionFlush();
}
```

## Notes ##

At the moment, function data is tracked via a map.  This is not efficient.  A better implementation might e.g. use a vector and index directly by the function IDs.  This may be coming in a future version of the code.

Dot output generally tends to be extremely long and flat.  One way to make the output a little better is to use the 'unflatten' utility
to pre-process the graphs intended for dot, e.g.

```bash
unflatten -c 5 chains.out | dot -Tpng /tmp/callgraph.png
```

There's code in here to support lower-level statistics (cache, others) via Intel PCM, but there are cross-platform considerations that make this trickier than it would be otherwise.  As such, supporting these is a TODO item.
