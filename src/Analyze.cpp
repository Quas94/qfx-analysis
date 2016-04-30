#include <cstdlib>
#include <cmath>

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdexcept>

#include <string>
#include <vector>
#include <iterator>

#include "ta_libc.h"
#include "AbstractIndicator.hpp"
#include "Stochastic.hpp"

#include "Constants.hpp"
#include "Timeframe.hpp"
#include "Parser.hpp"
#include "Strategy.hpp"

using namespace std;

int main(int argc, char** argv) {
	SimpleDate start(2015, 7, 1);
	SimpleDate end(2016, 3, 31);
	Parser* parser = new Parser(start, end, Hour);
	parser->parse();

	/*
	cout << "After parsing, we have written " << parser->get_num_candles() << " candles" << endl;
	cout << "First 100 OHLC:" << endl;
	for (int i = 0; i < parser->get_num_candles(); i++) {
		cout << parser->get_open_prices()[i] << ", " << parser->get_high_prices()[i] << ", " <<
			parser->get_low_prices()[i] << ", " << parser->get_close_prices()[i] << endl;
	}
	*/

	cout << "REMINDER: we are assuming the currency is always EUR_USD right now" << endl;
	cout << "REMINDER: we currently only look at closing price, without factoring in shadows" << endl;

	// technical analysis
	// start up TA_lib
	if (TA_Initialize() != TA_SUCCESS) {
		throw runtime_error("Could not initialise TA_lib");
	}
	cout << "Initialised TA_lib successfully" << endl;

	// initialise our strategy setups
	vector<pair<int, int> > sl_tp_pairs;
	sl_tp_pairs.push_back(make_pair(20, 20));
	sl_tp_pairs.push_back(make_pair(20, 40));
	sl_tp_pairs.push_back(make_pair(20, 50));
	sl_tp_pairs.push_back(make_pair(50, 50));
	sl_tp_pairs.push_back(make_pair(50, 75));
	sl_tp_pairs.push_back(make_pair(50, 100));
	sl_tp_pairs.push_back(make_pair(50, 125));
	sl_tp_pairs.push_back(make_pair(50, 150));
	sl_tp_pairs.push_back(make_pair(75, 75));
	sl_tp_pairs.push_back(make_pair(75, 150));
	sl_tp_pairs.push_back(make_pair(75, 225));

	for (auto it = sl_tp_pairs.begin(); it != sl_tp_pairs.end(); it++) {
		Strategy *strategy = new Strategy(parser, it->first, it->second, 12);
		strategy->run();
		delete strategy;
	}
	
	// shutdown TA_lib
	if (TA_Shutdown() != TA_SUCCESS) {
		throw runtime_error("Could not shutdown TA_lib");
	}

	delete parser;
	cout << "Finished" << endl;

	return EXIT_SUCCESS;
}
