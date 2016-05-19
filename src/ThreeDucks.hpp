#pragma once

#include "ta_libc.h"
#include "AbstractIndicator.hpp"
#include "Parser.hpp"

/**
* An implementation of the three ducks system found on the Babypips forums.
* Link: http://forums.babypips.com/free-forex-trading-systems/6580-3-ducks-trading-system.html
*/
class ThreeDucks : public AbstractIndicator {
private:

	int a_period;
	int b_period;
	int c_period;

	int a_begin;
	int b_begin;
	int c_begin;
	int a_num;
	int b_num;
	int c_num;

	double *a_ma;
	double *b_ma;
	double *c_ma;

	bool simple_ma;

	// keeps track of price and ma crossovers
	int last_checked_index;

	// number of pips crossing strength on the smallest timeframe moving average
	// ie. first 33% of tested candles must be crossing_strength_pips above the smallest timeframe MA
	// and similarly for the last 33% below the smallest timeframe MA
	int crossing_strength_pips;

	// number of candles to check when determining crossover of price and smallest timeframe MA
	// must be a multiple of 3
	int checking_width;

public:
	// two implementations to choose from:
	// - default settings (5min, 1hr and 4hr charts) or
	// - custom (1hr, 4hr and daily charts)
	ThreeDucks(Parser *parser, bool default_settings, int crossing_strength_pips, int checking_width);

	~ThreeDucks();

	void process();

	Signal get_signal(int index);

	string get_desc();
};
