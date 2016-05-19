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
#include "ReverseIndicator.hpp"
#include "AlwaysBuy.hpp"
#include "MovingAverageTrend.hpp"

#include "Constants.hpp"
#include "Timeframe.hpp"
#include "Parser.hpp"
#include "Strategy.hpp"

#include "StochMATrendGroup.hpp"
#include "ThreeDucksGroup.hpp"

using namespace std;

inline bool valid_year(int year) {
	return year >= 2006 && year <= 2016;
}

inline bool valid_month(int month) {
	return month >= 1 && month <= 12;
}

int main(int argc, char** argv) {
	int start_year = 2011;
	int start_month = 1;
	int end_year = 2016;
	int end_month = 4;

	const string output_file = "../results/out.js";
	const vector<string> currencies = {
		"AUD_CAD",
		"AUD_CHF",
		"AUD_JPY",
		"AUD_NZD",
		"AUD_USD",

		"CAD_CHF",
		"CAD_JPY",

		"CHF_JPY",

		"EUR_AUD",
		"EUR_CAD",
		"EUR_CHF",
		"EUR_GBP",
		"EUR_JPY",
		"EUR_NZD",
		"EUR_USD",

		"GBP_AUD",
		"GBP_CAD",
		"GBP_CHF",
		"GBP_JPY",
		"GBP_NZD",
		"GBP_USD",

		"NZD_CAD",
		"NZD_CHF",
		"NZD_JPY",
		"NZD_USD",

		"USD_CAD",
		"USD_CHF",
		"USD_JPY",
	};

	// check if file exists and confirm overwrite
	ifstream check_exists;
	check_exists.open(output_file, ifstream::in);
	if (check_exists.is_open()) { // file exists
		string answer = "";
		while (answer != "Y" && answer != "N") {
			cout << "out.js already exists. Overwrite? (Y or N): ";
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

	// technical analysis
	// start up TA_lib
	if (TA_Initialize() != TA_SUCCESS) {
		throw runtime_error("Could not initialise TA_lib");
	}
	cout << "Initialised TA_lib successfully" << endl;

	// initialise our strategy setups
	vector<pair<int, int> > sl_tp_pairs;
	vector<int> pip_sizes{ 50, 75, 100, 150, 200, 250, 300 };
	for (unsigned int i = 0; i < pip_sizes.size(); i++) {
		for (unsigned int j = i; j < pip_sizes.size(); j++) {
			sl_tp_pairs.push_back(make_pair(pip_sizes[i], pip_sizes[j]));
		}
	}

	// vector<int> cooldowns{ 1, 4, 12, 24 };
	vector<int> cooldowns{ 4, 8 };

	// risk per trade and move stop
	const double risk_percent_per_trade = 2.0;
	const bool move_stop = false;

	SimpleDate start(start_year, start_month, 1);
	SimpleDate end(end_year, end_month, 31);
	int num_years = end_year - start_year + 1;

	out << "var NUM_PAIRS = " << currencies.size() << ";" << endl;
	out << "var DATA = [" << endl;

	for (auto it = currencies.begin(); it != currencies.end(); it++) {
		Parser *parser = new Parser(*it, start, end, Hour);
		parser->parse();

		//IndicatorGroup *indicator_group_wrapper = new StochMATrendGroup(parser);
		IndicatorGroup *indicator_group_wrapper = new ThreeDucksGroup(parser);
		vector<vector<AbstractIndicator*>*> indicator_groups = indicator_group_wrapper->get_groups();
		for (auto ig = indicator_groups.begin(); ig != indicator_groups.end(); ig++) {
			for (auto it = sl_tp_pairs.begin(); it != sl_tp_pairs.end(); it++) {
				for (auto i = cooldowns.begin(); i != cooldowns.end(); i++) {
					Strategy *strategy = new Strategy(parser, risk_percent_per_trade, it->first,
						it->second, *i, *ig, move_stop);
					strategy->run(out);
					delete strategy;
				}
			}
		}
		
		// cleanup
		delete indicator_group_wrapper;
		delete parser;
	}

	out << "];" << endl;
	
	// shutdown TA_lib
	if (TA_Shutdown() != TA_SUCCESS) {
		throw runtime_error("Could not shutdown TA_lib");
	}

	cout << "Finished" << endl;

	return EXIT_SUCCESS;
}
