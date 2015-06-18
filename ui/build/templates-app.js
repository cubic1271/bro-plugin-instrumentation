angular.module('templates-app', ['about/about.tpl.html', 'home/home.tpl.html']);

angular.module("about/about.tpl.html", []).run(["$templateCache", function($templateCache) {
  $templateCache.put("about/about.tpl.html",
    "<div class=\"row\">\n" +
    "    <h4>Overview</h4>\n" +
    "    <p>This page is a client-side HTML / JS application designed to offer a basic user interface for browsing the results of a single execution of Bro.  Results are split into three parts, corresponding \n" +
    "    to the three different types of profiling output generated by the plugin.</p>\n" +
    "    <h5>Runtime</h5>\n" +
    "    <p>The first type of output is 'runtime' output, or statistics gathered every X seconds.  This type of output is designed to offer insight into how bro is performing overall (e.g. tracking memory \n" +
    "    allocation, disk writes, etc).  Currently, the following metrics are reported:</p>\n" +
    "    <ul>\n" +
    "        <li>Cycles - The total number of CPU cycles spent processing packets in bro.</li>\n" +
    "        <li>Packets - The total number of packets bro has seen thus far.</li>\n" +
    "        <li>Bytes (Written|Read) - The total number of bytes bro has written to / read from disk.  Caveat: this relies on hooks that intercept calls to 'read' and 'write' ... as such, data written or read that didn't use these calls won't be reflected in these numbers.</li>\n" +
    "        <li>malloc count / malloc size / free count - The number of times bro has called 'malloc' and 'free'.  This is a metric to report on the total number of allocations bro has made thus far.  malloc size records how much <strong>total</strong> memory bro has requested: it does <strong>not</strong> reflect the amount of memory in use by the process.<li>\n" +
    "        <li>cycles / packet - On average, the number of cycles spent processing each individual packet (calculated since the last time statistics were gathered)</li>\n" +
    "        <li>packets / second - On average, the number of packets observed each second (calculated since the last time statistics were gathered)</li>\n" +
    "        <li>load - An estimate of how close the system is to capacity, reflected as a number between 0 and (approximately) 1.  This is a <strong>very</strong> rough guess.  It is calculated by first dividing \n" +
    "        the average number of cycles taken to process each packet by the number of cycles the CPU has available to it each second: this estimates an upper bound on the number of packets bro can \n" +
    "        process in a second, which we'll call 'capacity'.  If we divide the number of packets we observed during the last time period by the 'capacity' number we computed above, it produces a \n" +
    "        number between 0 and 1.  This offers a <strong>very rough</strong> estimate of bro's load during the past X seconds or Y packets (depending on how the profiler is configured).  For example, a value \n" +
    "        of 0.6 here would indicate that Bro was processing 60% of what we calculated its theoretical maximum (its capacity) to be.  This number is by no means perfect, and is here more as food for thought\n" +
    "        than anything else ...</li>\n" +
    "    </ul>\n" +
    "    <p>\n" +
    "    The X axis represents the time (in seconds) since bro received its first packet.\n" +
    "    </p>\n" +
    "    <p>\n" +
    "    It's possible to interact with the graph: clicking on the names of different fields will toggle whether or not they are displayed on the graph, and moving the cursor over a point on the graph will\n" +
    "    display a table showing the exact values for that data point.\n" +
    "    <p>\n" +
    "    Additionally, an option is offered to normalize the data (by clicking the blue button above the graph).  This transforms the data such that <strong>all</strong> values fit between '0' and '1'.  This\n" +
    "    allows all the different metrics to share a common scale, and offers a way to search for patterns in the recorded data (e.g. \"my memory allocation is increasing with the number of packets squared ...\n" +
    "    my server will probably catch fire soon\").\n" +
    "    </p>\n" +
    "    <h5>Function Profile</h5>\n" +
    "    <p>\n" +
    "    This section offers a list of how many times each individual bro function was called, how much CPU time was spent in the function, and how many times the function allocated memory while it was running.\n" +
    "    CPU numbers are aggregate: these values record the total time spent in this function <strong>or</strong> any of the functions it calls in turn.  In practice, this means that it's quite likely that the\n" +
    "    total CPU seconds recorded across all bro script functions can add up to a number that is higher than the total number of CPU seconds bro actually spent running the program.\n" +
    "    </p>\n" +
    "    <p>\n" +
    "    Results may be sorted in three ways:\n" +
    "    </p>\n" +
    "        <ul>\n" +
    "            <li>cycles</li> - sort by the total number of cycles consumed by the function (CPU seconds)\n" +
    "            <li>malloc</li> - sort by the total number of allocations made by the function\n" +
    "            <li>count</li> - sort by the total number of times the function was called\n" +
    "        </ul>\n" +
    "    <h5>Call Graph</h5>\n" +
    "    <p>\n" +
    "    The call graph is an overview of which functions called which other functions while bro was executing.  This is usually an <strong>incredibly</strong> large picture for nontrivial applications, since\n" +
    "    bro normally calls a large number of functions (e.g. to initialize logging streams and the like).\n" +
    "    </p>\n" +
    "    <h4>Using this UI</h4>\n" +
    "    <p>\n" +
    "    The easiest way to get going with this UI at the moment is as follows:\n" +
    "    </p>\n" +
    "    <ul>\n" +
    "        <li>Clone the https://github.com/cubic1271/bro-plugin-instrumentation repository</li>\n" +
    "        <li>Execute bro with instrumentation enabled (e.g. with the provided instrument.bro script in the root of the repository)</li>\n" +
    "        <li>Copy the resulting bro-profile-collection file into the 'ui/bin' directory as 'collection.json'</li>\n" +
    "        <li>Copy the resulting bro-profile-function file into the 'ui/bin' directory as 'profile.json'</li>\n" +
    "        <li>Execute 'unflatten' and 'dot' as described in the bro-plugin-instrumentation README.md, and copy the resulting callgraph image to the 'bin' directory as 'callgraph.png'</li>\n" +
    "        <li>Change directory to 'ui/bin' and execute 'python -m SimpleHTTPServer'</li>\n" +
    "        <li>Open http://localhost:8000 in a browser of choice</li>\n" +
    "    </ul>\n" +
    "    <h4>Notess</h4>\n" +
    "    <p>Skeleton generated by <a href='https://github.com/ngbp/ngbp'>ngBoilerplate</a></p>\n" +
    "</div>\n" +
    "\n" +
    "");
}]);

angular.module("home/home.tpl.html", []).run(["$templateCache", function($templateCache) {
  $templateCache.put("home/home.tpl.html",
    "<div>\n" +
    "    <div class=\"row\"><h4>Runtime Data</h4><div id='chart'></div></div>\n" +
    "    <div class=\"row\"> &nbsp; </div>\n" +
    "    <div class=\"row\">\n" +
    "        <button type=\"button\" class=\"btn btn-primary\" ng-model=\"normalizeData\" btn-checkbox btn-checkbox-true=\"true\" btn-checkbox-false=\"false\">\n" +
    "            Normalize Data \n" +
    "        </button>\n" +
    "        <span style='text-decoration:italic; font-size:0.8em;'>This will normalize all values, possibly making it easier to identify patterns in data ...</span>\n" +
    "    </div>\n" +
    "    <div class=\"row\"><hr /></div>\n" +
    "    <div class=\"row\"><h4>Profiling Results</h4>Sort by: <span ng-repeat=\"(key, value) in sort_func\"><a ng-click=\"updateSort(key)\" class='btn btn-sm btn-info'>{{key}}</a> </span></div>\n" +
    "\n" +
    "    <div class=\"row\">\n" +
    "    <pagination page=\"currentPage\" total-items=\"entries.length\" items-per-page=\"numPerPage\" boundary-links=\"true\"></pagination>\n" +
    "    </div>\n" +
    "    <div class=\"row\">\n" +
    "        <ul class=\"list-group\">\n" +
    "            <li class=\"list-group-item\" ng-repeat=\"entry in pagedEntries\">\n" +
    "                <h4 class=\"list-group-item-heading\">{{entry.name}} @ {{entry.location}}</h4>\n" +
    "                <p class=\"list-group-item-text\">Called {{entry.count}} times and used {{entry.perf.cycles}} cycles (~{{round(entry.perf.cycles / entry.perf.frequency, 4)}} cpu seconds)</p>\n" +
    "                <p class=\"list-group-item-text\">Allocated {{entry.memory.malloc_count}} times.</p>\n" +
    "            </li>\n" +
    "        </ul>\n" +
    "    </div>\n" +
    "\n" +
    "    <div class=\"row\">\n" +
    "        <h4>Call Graph</h4>\n" +
    "    </div>\n" +
    "    <div class=\"row\">\n" +
    "        Click <a href='callgraph.png' target='_blank'>here</a> to load a call graph of this execution.  WARNING: Call graphs tend to be huge, and can actually exceed the size supported by the browser in some cases ...\n" +
    "    </div>\n" +
    "</div>\n" +
    "\n" +
    "");
}]);
