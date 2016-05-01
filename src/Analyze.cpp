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
	if (argc != 2) {
		cout << "Usage: 'analyze.exe filename.ext" << endl;
		return EXIT_FAILURE;
	}

	string output_file = argv[1];
	cout << "Specified input file: " << output_file << endl;
	// check if file exists and confirm overwrite
	ifstream check_exists;
	check_exists.open(output_file, ifstream::in);
	if (check_exists.is_open()) { // file exists
		string answer = "";
		while (answer != "Y" && answer != "N") {
			cout << "File already exists. Overwrite? (Y or N): ";
			getline(cin, answer);
			if (answer.size() == 1) {
				if (answer[0] == 'y') answer = "Y";
				else if (answer[0] == 'n') answer = "N";
			}
		}
		if (answer == "N") {
			cout << "App terminated" << endl;
			return EXIT_FAILURE;
		}
	}
	ofstream out;
	out.open(output_file, ofstream::out);
	if (out.fail()) {
		cout << "Error: could not open the specified file for writing" << endl;
		return EXIT_FAILURE;
	}

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
		Strategy *strategy = new Strategy(parser, it->first, it->second, 4);
		strategy->run(out);
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
