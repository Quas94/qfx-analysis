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

inline bool valid_year(int year) {
	return year >= 2011 && year <= 2016;
}

inline bool valid_month(int month) {
	return month >= 1 && month <= 12;
}

int main(int argc, char** argv) {
	if (argc != 6) {
		cout << "Usage: 'analyze.exe filename.csv startyear startmonth endyear endmonth" << endl;
		return EXIT_FAILURE;
	}

	string output_file = argv[1];
	int start_year = stoi(argv[2]);
	int start_month = stoi(argv[3]);
	int end_year = stoi(argv[4]);
	int end_month = stoi(argv[5]);
	if (!valid_year(start_year) || !valid_year(end_year) || !valid_month(start_month) || !valid_month(end_month)) {
		cout << "Years must be between 2011-2016, and months must be between 1-12" << endl;
		return EXIT_FAILURE;
	}

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

	SimpleDate start(start_year, start_month, 1);
	SimpleDate end(end_year, end_month, 31);
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
	sl_tp_pairs.push_back(make_pair(25, 25));
	sl_tp_pairs.push_back(make_pair(25, 50));
	sl_tp_pairs.push_back(make_pair(25, 75));
	sl_tp_pairs.push_back(make_pair(25, 100));
	sl_tp_pairs.push_back(make_pair(25, 125));
	sl_tp_pairs.push_back(make_pair(25, 150));
	sl_tp_pairs.push_back(make_pair(25, 200));
	sl_tp_pairs.push_back(make_pair(25, 250));
	sl_tp_pairs.push_back(make_pair(25, 300));
	sl_tp_pairs.push_back(make_pair(50, 25));
	sl_tp_pairs.push_back(make_pair(50, 50));
	sl_tp_pairs.push_back(make_pair(50, 75));
	sl_tp_pairs.push_back(make_pair(50, 100));
	sl_tp_pairs.push_back(make_pair(50, 125));
	sl_tp_pairs.push_back(make_pair(50, 150));
	sl_tp_pairs.push_back(make_pair(50, 200));
	sl_tp_pairs.push_back(make_pair(50, 250));
	sl_tp_pairs.push_back(make_pair(50, 300));
	sl_tp_pairs.push_back(make_pair(75, 75));
	sl_tp_pairs.push_back(make_pair(75, 150));
	sl_tp_pairs.push_back(make_pair(75, 225));
	sl_tp_pairs.push_back(make_pair(75, 300));
	sl_tp_pairs.push_back(make_pair(100, 25));
	sl_tp_pairs.push_back(make_pair(100, 50));
	sl_tp_pairs.push_back(make_pair(100, 75));
	sl_tp_pairs.push_back(make_pair(100, 100));
	sl_tp_pairs.push_back(make_pair(100, 200));
	sl_tp_pairs.push_back(make_pair(100, 300));

	// vector<int> cooldowns{ 1, 4, 12, 24 };
	vector<int> cooldowns{ 24 };

	// print first line of csv
	out << "Indicator list and descriptions,Cooldown,Stop loss,Take profit,Winners,Losers,Total trades," <<
		"Win %,Pips gained" << endl;
	for (auto it = sl_tp_pairs.begin(); it != sl_tp_pairs.end(); it++) {
		for (auto i = cooldowns.begin(); i != cooldowns.end(); i++) {
			Strategy *strategy = new Strategy(parser, it->first, it->second, *i);
			strategy->run(out);
			delete strategy;
		}
	}
	
	// shutdown TA_lib
	if (TA_Shutdown() != TA_SUCCESS) {
		throw runtime_error("Could not shutdown TA_lib");
	}

	delete parser;
	cout << "Finished" << endl;

	return EXIT_SUCCESS;
}
