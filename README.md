
## Bro Instrumentation Plugin ##

### Overview ###

The bro instrumentation plugin is composed of a few parts:

* A few syshook libraries that may either be loaded via LD_PRELOAD (on Linux / *BSD) or compiled into the project via the appropriate flags (on OS/X).
* A bro plugin that periodically grabs statistics from the syshook libraries (along with other statistics it keeps internally) and writes them to a CSV file.

Currently, only Intel CPUs supported by Intel's PCM library will report instruction counts / cache statistics.  This is because OS/X doesn't play nicely with PAPI.

### Building ###

* build the syshook shared libraries (via 'make preload')
* OS/X: set up Intel PCM (in aux/intel-pcm-2.6: consult the HOWTO there for details)
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

### Using ###

The plugin can collect information in three different forms:

* Dump a collection of statistics after observing X packets / after Y seconds have elapsed.  This is enabled via the Instrumentation::(Set)?Collection* set of methods.
* Track statistics per-function / per-event.  This is enabled via the Instrumentation::(Set)?Function* set of methods.
* Build call-graphs (meant to be parsed by the graphviz 'dot' utility) to visualize what the code is doing.  This is enabled via the Instrumentation::(Set)?CallGraph* set of methods.
