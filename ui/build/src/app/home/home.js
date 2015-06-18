angular.module( 'broProfiler.home', [
  'ui.router',
  'plusOne'
])
.config(['$stateProvider', function config( $stateProvider ) {
  $stateProvider.state( 'home', {
    url: '/home',
    views: {
      "main": {
        controller: 'HomeCtrl',
        templateUrl: 'home/home.tpl.html'
      }
    },
    data:{ pageTitle: 'Home' }
  });
}])
.controller( 'HomeCtrl', ['$scope', '$http', function HomeController( $scope, $http ) {
    $scope.entries = [];
    $scope.pagedEntries = [];
    $scope.numPerPage = 20;
    $scope.maxSize = 100;
    $scope.currentPage = 1;
    $scope.sorted = "cycles";
    $scope.sort_func = {"cycles": function(a, b) { return b.perf.cycles - a.perf.cycles; },
                        "malloc": function(a, b) { return b.memory.malloc_count - a.memory.malloc_count; },
                        "count" : function(a, b) { return b.count - a.count; } };
    $scope.normalizeData = false;

    // http://stackoverflow.com/questions/11832914/round-to-at-most-2-decimal-places-in-javascript
    $scope.round = function(value, exp) {
        if (typeof exp === 'undefined' || +exp === 0) {
            return Math.round(value);
        }

        value = +value;
        exp  = +exp;

        if (isNaN(value) || !(typeof exp === 'number' && exp % 1 === 0)) {
            return NaN;
        }
        // Shift
        value = value.toString().split('e');
        value = Math.round(+(value[0] + 'e' + (value[1] ? (+value[1] + exp) : exp)));

        // Shift back
        value = value.toString().split('e');
        return +(value[0] + 'e' + (value[1] ? (+value[1] - exp) : -exp));
    };

    $scope.updatePage = function() {
        $scope.entries.sort($scope.sort_func[$scope.sorted]);
        $scope.maxSize = Math.ceil($scope.entries.length / $scope.numPerPage);
        var begin = (($scope.currentPage - 1) * $scope.numPerPage); 
        var end = begin + $scope.numPerPage;
        $scope.pagedEntries = $scope.entries.slice(begin, end);
    };
    
    $scope.updateSort = function(target) {
        console.log("Sort selected: " + target);
        $scope.sorted = target;
        $scope.updatePage();
    };

    $scope.rebuildChart = function() {
        console.log("Rebuilding chart (normalized = " + $scope.normalizeData + ")");
        var data = $scope.collection;
        if(typeof(data) === "undefined") {
            console.log("No data available.  Aborting build ...");
            return;
        }

        var graphElements = [ ['network-time'], ['cpu cycles'], ['bytes written'], ['bytes read'], ['malloc count'], ['malloc bytes'], ['free count'], ['packets'], ['cycles / packet'], ['packets / second'], ['load (approx.)'] ];
        var startCycles = 0;
        var startTime = 0;
        var i = 0;
        var j = 0;

        for(i = 0; i < data.length; ++i) {
            if(startTime < 0.0001) {
                startTime = parseFloat(data[i]['network-time']);
                graphElements[0].push(0);
            }
            else {
                graphElements[0].push(data[i]['network-time'] - startTime);
            }
            if(startCycles === 0) {
                startCycles = parseInt(data[i]["perf"]["cycles"], 10);
                graphElements[1].push(0);
            }
            else {
                graphElements[1].push(parseInt(data[i]["perf"]["cycles"], 10) - startCycles);
            }
            graphElements[2].push(parseInt(data[i]["io"]["fwrite_sz"], 10) + parseInt(data[i]["io"]["write_sz"], 10));
            graphElements[3].push(parseInt(data[i]["io"]["fread_sz"], 10) + parseInt(data[i]["io"]["read_sz"], 10));
            graphElements[4].push(data[i]["memory"]["malloc_count"]);
            graphElements[5].push(data[i]["memory"]["malloc_sz"]);
            graphElements[6].push(data[i]["memory"]["free_count"]);
            graphElements[7].push(data[i]["packets"]);
            // Ignore the first five packets to try to smooth things out a bit ...
            if(data[i]["packets"] > 5 && i > 0) {
                var load_capacity = 0;
                var load_measurement = 0;
                var cycles_per_packet = 0;
                if(data[i]["packets"] - data[i - 1]["packets"] > 0) {
                    cycles_per_packet = data[i]["perf"]["cycles"] - data[i - 1]["perf"]["cycles"];
                    cycles_per_packet /= data[i]["packets"] - data[i - 1]["packets"];
                }
                graphElements[8].push(cycles_per_packet);
                var packets_per_second = 0;
                if( ( (data[i]["network-time"] - startTime) - (data[i - 1]["network-time"] - startTime) ) > 0 ) {
                    packets_per_second = data[i]["packets"] - data[i - 1]["packets"];
                    packets_per_second /= ( (data[i]["network-time"] - startTime) - (data[i - 1]["network-time"] - startTime) );
                }
                graphElements[9].push(packets_per_second);
                if(cycles_per_packet > 0 && packets_per_second > 0) {
                    // capacity is the number of cycles per second divided by the number of cycles we spend on each individual packet.
                    load_capacity = data[i]["perf"]["frequency"] / cycles_per_packet;
                    // estimated load is then the current measured PPS / the total capacity
                    load_measurement = packets_per_second / load_capacity;
                }
                graphElements[10].push(load_measurement);
            }
            else {
                graphElements[8].push(0);
                graphElements[9].push(0);
                graphElements[10].push(0);
            }
        }

        if($scope.normalizeData) {
            var graphMax = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
            for(i = 1; i <= data.length; ++i) {
                for(j = 1; j < 11; ++j) {
                    if(graphElements[j][i] > graphMax[j]) {
                        graphMax[j] = graphElements[j][i];
                    }
                }
            }
            for(i = 1; i <= data.length; ++i) {
                for(j = 1; j < 11; ++j) {
                    graphElements[j][i] = graphElements[j][i] / graphMax[j];
                }
            }
        }

        var chart = c3.generate({ 
            data: {
                x: 'network-time',
                columns: graphElements
            }
        });
    };

    $scope.$watch("normalizeData", $scope.rebuildChart);

    $scope.$watch("sorted", $scope.updatePage);

    $scope.$watch("currentPage + numPerPage", $scope.updatePage);
    
    $http.get('profile.json')
        .success(function(data, status, headers, config) { 
            console.log("Reading profile data ...");
            $scope.entries = data;
            $scope.updatePage();
        }).error(function(data, status, headers, config) { 
            console.log("Unable to retrieve profile data: status " + status);
    });

    $http.get('collection.json')
        .success(function(data, status, headers, config) {
            console.log("Reading collection data ...");
            $scope.collection = data;
            $scope.rebuildChart();
            $scope.updatePage();
        }).error(function(data, status, headers, config) {
            console.log("Unable to retrieve collection data: status " + status);
        });
}]);

