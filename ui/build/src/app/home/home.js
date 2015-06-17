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

    $scope.$watch("sorted", $scope.updatePage);

    $scope.$watch("currentPage + numPerPage", $scope.updatePage);
    
    $http.get('/profile.json')
        .success(function(data, status, headers, config) { 
            console.log("Reading profile data ...");
            $scope.entries = data;
            $scope.updatePage();
        }).error(function(data, status, headers, config) { 
            console.log("Unable to retrieve profile data: status " + status);
    });
}]);

