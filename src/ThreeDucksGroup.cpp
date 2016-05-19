#include "ThreeDucks.hpp"
#include "ThreeDucksGroup.hpp"

ThreeDucksGroup::ThreeDucksGroup(Parser *parser) {
	// add threeducks instances with different crossover pip minimums and checking width combos
	vector<int> crossover_minimums = { 1, 3, 5, 10, 20, 50 };
	vector<int> checking_widths = { 3, 6, 9, 12, 15 };
	for (auto it = crossover_minimums.begin(); it != crossover_minimums.end(); it++) {
		for (auto cw = checking_widths.begin(); cw != checking_widths.end(); cw++) {
			ducks.push_back(new ThreeDucks(parser, false, *it, *cw));
		}
	}
	// add to indicator groups
	for (auto duck = ducks.begin(); duck != ducks.end(); duck++) {
		indicator_groups.push_back(new vector<AbstractIndicator*>{ *duck });
	}
}

ThreeDucksGroup::~ThreeDucksGroup() {
	for (auto duck = ducks.begin(); duck != ducks.end(); duck++)
		delete *duck;
}
