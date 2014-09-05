
## Bro Instrumentation Plugin ##

### Overview ###

The bro instrumentation plugin is composed of a few parts:

* A few syshook libraries that may either be loaded via LD_PRELOAD (on Linux / *BSD) or compiled into the project via the appropriate flags (on OS/X).
* A bro plugin that periodically grabs statistics from the syshook libraries (along with other statistics it keeps internally) and writes them to a CSV file.

### Building ###

_NOTE_ OS/X requires rebuilding / relinking bro before the syshook libraries will work as intended.  TODO: document how to do this.

* build the syshook shared libraries (via 'make preload').
* make and install the plugin (via 'make install').

### Using ###

_TODO_

