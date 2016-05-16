'use strict';

var cmApp = angular.module('analyzeApp', []);

cmApp.controller('analyzeController', ['$scope',
	function($scope) {
		$scope.mode = 'data';
		var STARTING_EQUITY = 5000;
		var STARTING_YEAR = 2011;

		$scope.data = DATA;
		$scope.sortType = 'final';
		$scope.sortReverse = true;
		$scope.searchFilter = '';

		$scope.topSortType = 'approxEnd';
		$scope.topSortReverse = true;

		$scope.filtersError = false;
		$scope.temp = {
			desc: '',

			minTotalTrades: null,
			minFinalPercent: null,
			minWorstYear: null,
			minWorstMonth: null,
		};
		$scope.filters = {
			desc: '',

			minTotalTrades: 50,
			minFinalPercent: 150,
			minWorstYear: 70,
			minWorstMonth: 70,
		};

		if (NUM_PAIRS === undefined)
			throw Error('NUM_PAIRS is undefined');

		$scope.getFullDesc = function(line) {
			var combined = '';
			for (var i = 0; i < line.desc.length; i++) {
				combined += line.desc[i] + ',';
			}
			combined += 'SL' + line.sl + 'TP' + line.tp + 'CD' + line.cd;
			return combined;
		};

		$scope.getApproxEnd = function(strategy) {
			var monthResult = 1;
			for (var pair = 0; pair < strategy.months.length; pair++) {
				for (var month = 0; month < strategy.months[pair].length; month++) {
					monthResult *= (strategy.months[pair][month] / 100);
				}
			}
			monthResult *= 100;

			return monthResult.toFixed(2);
		};

		$scope.getEquityPoints = function(strategy) {
			var points = [];
			var equity = STARTING_EQUITY;
			for (var m = 0; m < strategy.months[0].length; m++) {
				for (var c = 0; c < NUM_PAIRS; c++) {
					equity *= (strategy.months[c][m] / 100);
				}
				points.push(equity);
			}
			return points;
		}

		$scope.getLargestDrawdown = function(points) {
			var topEquity = 0;
			var largestDrawdown = 0;
			for (var i = 0; i < points.length; i++) {
				var curEquity = points[i];
				if (curEquity > topEquity) {
					topEquity = curEquity;
				} else {
					var diff = topEquity - curEquity;
					var diffPercent = diff / topEquity * 100;
					if (diffPercent > largestDrawdown)
						largestDrawdown = diffPercent;
				}
			}
			return largestDrawdown;
		};

		$scope.expand = function(strategy) {
			strategy.expanded = !strategy.expanded;
		};

		for (var i = 0; i < DATA.length; i++) {
			var line = DATA[i];
			// calculate values
			line.winPercent = line.winners / (line.winners + line.losers) * 100;
			line.expected = line.sl / (line.tp + line.sl) * 100;
			line.diff = line.winPercent - line.expected;
		}

		// calculate best strategies
		// sort into order by name
		DATA.sort(function(a, b) {
			var descA = $scope.getFullDesc(a);
			var descB = $scope.getFullDesc(b);
			if (descA === descB)
				return 0;
			if (descA < descB)
				return -1;
			else
				return 1;
		});
		$scope.strategies = [];
		var index = 0;
		while (index < DATA.length) {
			var strategy = {
				name: $scope.getFullDesc(DATA[index]),
				bestFinal: 0,
				bestFinalPair: '',
				worstFinal: 10000,
				worstFinalPair: '',
				finals: [],
				bestMonth: 0,
				worstMonth: 10000,
				totalTrades: 0,
				totalWinTrades: 0,
				tradesPerYear: [],
				winTradesPerYear: [],
				months: [],
				years: [],
				currencies: [],
				finals: [],

				expanded: false,
			};

			var stratIter = 0;
			while (stratIter < NUM_PAIRS) {
				var line = DATA[index];
				var fullDesc = $scope.getFullDesc(DATA[index]);
				if (fullDesc !== strategy.name)
					throw Error('fullDesc = ' + fullDesc + ', strategy.name = ' + strategy.name);

				if (strategy.bestFinal < line.final) {
					strategy.bestFinal = line.final;
					strategy.bestFinalPair = line.pair;
				}
				if (strategy.worstFinal > line.final) {
					strategy.worstFinal = line.final;
					strategy.worstFinalPair = line.pair;
				}

				if (strategy.bestMonth < line.month_best)
					strategy.bestMonth = line.month_best;
				if (strategy.worstMonth > line.month_worst)
					strategy.worstMonth = line.month_worst;

				strategy.totalTrades += line.trades;
				strategy.totalWinTrades += line.winners;

				strategy.tradesPerYear.push(line.trades);
				strategy.winTradesPerYear.push(line.winners);
				strategy.months.push(line.months);
				strategy.years.push(line.years);
				strategy.currencies.push(line.pair);
				strategy.finals.push(line.final);

				stratIter++;
				index++;
			}

			strategy.largestDrawdown = $scope.getLargestDrawdown($scope.getEquityPoints(strategy));

			strategy.approxEnd = Number($scope.getApproxEnd(strategy));
			$scope.strategies.push(strategy);
		}

		$scope.setSort = function(type) {
			$scope.sortType = type;
		};

		$scope.greaterThan = function(prop, val) {
			return function(item) {
				return item[prop] > val;
			};
		};

		$scope.strContains = function(prop, search) {
			return function(item) {
				if (search === '') return true;

				if (prop === 'desc') {
					var combined = $scope.getFullDesc(item);
					return combined.toLowerCase().indexOf(search.toLowerCase()) !== -1;
				}
				throw Error('prop = ' + prop);
			};
		};

		$scope.showFiltersModal = function() {
			$scope.temp.minTotalTrades = $scope.filters.minTotalTrades;
			$scope.temp.minFinalPercent = $scope.filters.minFinalPercent;
			$scope.temp.minWorstYear = $scope.filters.minWorstYear;
			$scope.temp.minWorstMonth = $scope.filters.minWorstMonth;
			$('#modal-filters').modal('show');
		};

		var drawChart = function(numMonths, points) {
			// labels
			var labels = [];
			for (var i = -1; i < numMonths; i++) {
				if (i % 12 === 0) {
					labels.push(STARTING_YEAR + (i / 12));
				} else {
					labels.push('');
				}
			}

			// construct chart config object
			var inputs = {
				labels: labels,
				series: [
					points
				],
			};
			var options = {
				width: 550,
				height: 412
			}
			// construct chart
			new Chartist.Line('.ct-chart', inputs, options);

			$('#modal-chart').modal('show');
		};

		$scope.getLineEquities = function(line) {
			var equity = STARTING_EQUITY;
			var points = [ equity ];
			for (var i = 0; i < line.months.length; i++) {
				equity *= (line.months[i] / 100);
				points.push(equity);
			}
			return points;
		};

		$scope.viewSingleChart = function(line) {
			var points = $scope.getLineEquities(line);

			$scope.modalChartMaxDrawdown = $scope.getLargestDrawdown(points);
			drawChart(line.months.length, points);
		};

		$scope.viewStrategyChart = function(strategy) {
			var points = $scope.getEquityPoints(strategy);
			points.splice(0, 0, STARTING_EQUITY); // very beginning = 100%

			$scope.modalChartMaxDrawdown = false;
			drawChart(strategy.months[0].length, points);
		};

		$scope.updateFilters = function() {
			var temp = $scope.temp;
			var arr = [ temp.minTotalTrades, temp.minFinalPercent, temp.minWorstYear, temp.minWorstMonth ];
			for (var i = 0; i < arr.length; i++) {
				var prop = arr[i];
				if (isNaN(prop)) {
					$scope.filtersError = true;
					return;
				}
				prop = Number(prop);
				if (prop < 0 || prop > 1000) {
					$scope.filtersError = true;
					return;
				}
			}
			// everything is ok
			$scope.filters.minTotalTrades = Number(temp.minTotalTrades);
			$scope.filters.minFinalPercent = Number(temp.minFinalPercent);
			$scope.filters.minWorstYear = Number(temp.minWorstYear);
			$scope.filters.minWorstMonth = Number(temp.minWorstMonth);
			$scope.filtersError = false;
			$('#modal-filters').modal('hide');
		};

		$scope.updateDescFilter = function() {
			$scope.filters.desc = $scope.temp.desc;
		};
	}
]);
