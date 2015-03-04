
# Bro Instrumentation Plugin #

## Overview ##

The bro instrumentation plugin is intended to offer some insight into what exactly bro is doing and how, exactly, it's spending its time.  To that effect, the plugin offers three different modes of operation:

* Occasional collection - this mode pauses either every X packets or Y seconds of network time to write statistics regarding the number of allocations, the number of filesystem reads / writes, and the total number of cycles spent processing packets (via RDTSC).
* Per-function statistics - this mode queries counters at the beginning and end of each function call, and uses the collected statistics to record information about how much overhead is associated with each specific function
* Call-graph - this mode generates a call-graph that can be parsed by graphviz (preferably 'dot')
* REST export of live data - a few methods allow for export of arbitrary data to a REST interface from either the plugin or bro script-land

## Building ##

* build the syshook shared libraries (via 'make preload')
* make and install the plugin (via 'make install')

### LD_PRELOAD ###

Once these two steps are completed, this plugin may be used by setting LD_PRELOAD to the following (on Linux / FreeBSD):

```bash
LD_PRELOAD=/path/to/syshook-malloc.so:/path/to/syshook-io.so
```

*However*, there are a few caveats:

* When run as root, applications ignore LD_PRELOAD (for security reasons).
* OS/X does not support an equivalent of LD_PRELOAD in a sane fashion (DYLD_FLAT_NAMESPACE fixes this, but can yield strange behavior and / or crashes).

### Linking the plugin to bro ###

Begin by ensuring that there is *no existing build/ directory in the bro source tree*.  Next, set the following environment variable:

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
* Enable live REST data output.  This is a slightly more complex topic ... please see the relevant section below.

As a convenience, there are three wrapper scripts included with this plugin: instrumentation/instrumentation/chains.bro, instrumentation/instrumentation/function.bro, and instrumentation/instrumentation/collection.bro.  Including these three files will make working with the interface quite a bit easier: see those files for relevant comments.  As a quick start, though:

An example of writing statistics to a file called '/tmp/collection.out' for every 10000 observed packets:

```
@load instrumentation/instrumentation/collection.bro

redef Instrumentation::collection_log = "/tmp/collection.out";
redef Instrumentation::collection_freq = 10000;

```

An example of writing function profiling, chain profiling, and collection information (once every 100 packets) to the default log paths:

```
@load instrumentation/instrumentation/collection.bro
@load instrumentation/instrumentation/function.bro
@load instrumentation/instrumentation/chains.bro

redef Instrumentation::collection_freq = 100;
redef Instrumentation::function_profile_enable = T;
redef Instrumentation::chain_profile_enable = T;
```

## REST Export ##

One of the unfortunate things about using logs for data is that it makes it relatively difficult to keep things collated and do things like e.g. reading data in real time from all the individual nodes of a cluster.  To work around this, this plugin supports the export of arbitrary data in a REST fashion.  To accomplish this, the plugin embeds an HTTP server (civetweb, a more liberally licensed fork of the mongoose embedded server) into the plugin.  A call to the Instrumentation::ExportStart() method will launch this HTTP server: for example:

```
event bro_init() {
    Instrumentation::ExportStart(4268);  # Launch the HTTP server on port 4268
}
```

Once this server has been launched, data can be displayed on this page as follows:

```
event bro_update_page() {
    Instrumentation::ExportAdd("some.data.to", "display");
    Instrumentation::ExportAdd("some.data.we", "like");
    Instrumentation::ExportAdd("another.blob", "with stuff in it");
}
```

This will make a REST page available at http://localhost:4268/export with the following content:

```
{
    "some": {
        "data": {
            "to": "display",
            "we": "like"
        }
    },
    "another": {
        "blob": "with stuff in it"
    }
}
```

There is, however, a slight catch.  Data is only actually *updated* on http://localhost:4268/export when the following method is called:

```
Instrumentation::ExportUpdate();
```

The reason for this involves a bit of a trade-off.  Since the HTTP server runs in a (number of) threads, there are concerns about thread-safety when passing values from the main bro thread into the HTTP thread servicing the request.  Thus, updates are cached to minimize the overhead involved with acquiring a mutex, copying necessary data, and releasing that mutex.  This makes individual ExportAdd() calls quite cheap, but requires a bit of thought with regard to how ExportUpdate() methods are handled.  One relatively simple way to handle things here is:

```
event update_rest_info() {
    if(bro_is_terminating()) {
        return;
    }
    Instrumentation::ExportUpdate();

    # Refresh data on the REST page 4 times each second
    schedule 250msecs { update_rest_info() };
}

event bro_init() {
    Instrumentation::ExportStart(4268);
   
    # Wait 5 seconds for our first update just because we can 
    schedule 5secs { update_rest_info() };
}
```

Also, note that a few basic pieces of information are included on the default /export page.  These keys cannot be used in user applications.

## Notes ##

Function profiling really, really needs to be further optimized.  I think a lot of the overhead has to do with the copying involved.  This is an important thing to fix.

Dot output generally tends to be extremely long and flat.  One way to make the output a little better is to use the 'unflatten' utility
to pre-process the graphs intended for dot, e.g.

```bash
unflatten -c 5 chains.out | dot -Tpng /tmp/callgraph.png
```

Currently, cycle counts are only supported on x86 platforms (due to rdtsc)

The REST interface stuff is pretty new, so the JSON formation is only lightly tested.  I may switch to a third-party library to handle this, since anything but really simple structures quickly become messy.

