angular.module('templates-app', ['about/about.tpl.html', 'home/home.tpl.html']);

angular.module("about/about.tpl.html", []).run(["$templateCache", function($templateCache) {
  $templateCache.put("about/about.tpl.html",
    "<div class=\"row\">\n" +
    "	<div>Skeleton generated by <a href='https://github.com/ngbp/ngbp'>ngBoilerplate</a></div>\n" +
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
