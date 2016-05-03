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
	if (argc != 7) {
		cout << "Usage: 'analyze.exe filename.csv currency_pair startyear startmonth endyear endmonth'" << endl;
		return EXIT_FAILURE;
	}

	string output_file = argv[1];
	string currency_pair = argv[2];
	int start_year = stoi(argv[3]);
	int start_month = stoi(argv[4]);
	int end_year = stoi(argv[5]);
	int end_month = stoi(argv[6]);
	if (!valid_year(start_year) || !valid_year(end_year) || !valid_month(start_month) || !valid_month(end_month)) {
		cout << "Years must be between 2011-2016, and months must be between 1-12" << endl;
		return EXIT_FAILURE;
	}

	cout << "Specified output file: " << output_file << " - will be saved to ../results/" << endl;
	output_file = "../results/" + output_file;
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
	Parser* parser = new Parser(currency_pair, start, end, Hour);
	parser->parse();
	int num_years = end_year - start_year + 1; // for use with extra info

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
	vector<int> pip_sizes{ 25, 50, 75, 100, 125, 150, 200, 250, 300 };
	vector<pair<int, int> > sl_tp_pairs;
	for (unsigned int i = 0; i < pip_sizes.size(); i++) {
		for (unsigned int j = i; j < pip_sizes.size(); j++) {
			sl_tp_pairs.push_back(make_pair(pip_sizes[i], pip_sizes[j]));
		}
	}

	// vector<int> cooldowns{ 1, 4, 12, 24 };
	vector<int> cooldowns{ 4, 8 };

	// indicators
	vector<vector<AbstractIndicator*>*> indicator_groups;
	//indicator_groups.push_back(new vector<AbstractIndicator*>{ new Stochastic(parser, 10, 90) });
	indicator_groups.push_back(new vector<AbstractIndicator*>{ new Stochastic(parser, 10, 90) });
	indicator_groups.push_back(new vector<AbstractIndicator*>{ new Stochastic(parser, 15, 85) });
	// indicator_groups.push_back(new vector<AbstractIndicator*>{ new AlwaysBuy(parser) });
	// indicator_groups.push_back(new vector<AbstractIndicator*>{ new ReverseIndicator(new AlwaysBuy(parser)) });
	// indicators.push_back(new ReverseIndicator(new Stochastic(parser, 10, 90)));
	// @TODO add more

	// risk per trade
	const double risk_percent_per_trade = 2.0;

	// print first line of csv
	out << "Indicator list and descriptions,Cooldown,Risk per trade,Stop loss,Take profit,Winners,Losers,Total trades," <<
		"Win %,Pips gained";
	// print out years for extra info: pips gained per year
	for (int y = 0; y < num_years; y++) out << "," << (start_year + y);
	// print out years for extra info: account size at the end of each year
	for (int y = 0; y < num_years; y++) out << "," << (start_year + y);
	out << ",Average % per year,Best month,Worst month,Winning months,Losing months";
	out << endl;
	for (auto ig = indicator_groups.begin(); ig != indicator_groups.end(); ig++) {
		for (auto it = sl_tp_pairs.begin(); it != sl_tp_pairs.end(); it++) {
			for (auto i = cooldowns.begin(); i != cooldowns.end(); i++) {
				Strategy *strategy = new Strategy(parser, risk_percent_per_trade, it->first, it->second, *i, *ig);
				strategy->run(out);
				delete strategy;
			}
		}
	}

	// delete indicators
	for (auto ig = indicator_groups.begin(); ig != indicator_groups.end(); ig++) {
		vector<AbstractIndicator*> *group = *ig;
		for (auto it = group->begin(); it != group->end(); it++) {
			delete *it;
		}
		delete group;
	}
	
	// shutdown TA_lib
	if (TA_Shutdown() != TA_SUCCESS) {
		throw runtime_error("Could not shutdown TA_lib");
	}

	delete parser;
	cout << "Finished" << endl;

	return EXIT_SUCCESS;
}
