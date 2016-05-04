#pragma once

#include <fstream>

#include "Timeframe.hpp"
#include "SimpleDate.hpp"
#include "Util.hpp"

using namespace std;

// takes in a filestream and timeframe
class Parser {
private:
	ifstream ifs;
	const string currency_pair;
	const Timeframe timeframe;

	const SimpleDate start_date;
	const SimpleDate end_date;
	SimpleDate current_date; // where we're up to in the file we're parsing through

	int max_candles;
	int num_candles;
	double *open_prices;
	double *high_prices;
	double *low_prices;
	double *close_prices;
	SimpleDate *date_tracker;

	const int NONE = -1;

public:

	Parser(string currency_pair, SimpleDate start_year, SimpleDate end_year, Timeframe timeframe);

	~Parser();

	void create_candle(double open, double high, double low, double close, const SimpleDate &current_date);

	int get_num_candles() const;
	int get_max_candles() const;

	const double * get_open_prices() const;
	const double * get_high_prices() const;
	const double * get_low_prices() const;
	const double * get_close_prices() const;
	const SimpleDate * get_date_tracker() const;

	void parse();
};
