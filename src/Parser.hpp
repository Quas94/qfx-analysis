#pragma once

#include <fstream>

#include "Timeframe.hpp"
#include "SimpleDate.hpp"

using namespace std;

// takes in a filestream and timeframe
class Parser {
private:
	ifstream ifs;

	const SimpleDate start_date;
	const SimpleDate end_date;
	SimpleDate current_date; // where we're up to in the file we're parsing through
	int current_file_year; // the year of the file we've currently opened

	const Timeframe timeframe;
	int max_candles;
	int num_candles;
	double *open_prices;
	double *high_prices;
	double *low_prices;
	double *close_prices;

	const int NONE = -1;

public:

	Parser(SimpleDate start_year, SimpleDate end_year, Timeframe timeframe);

	~Parser();

	void create_candle(double open, double high, double low, double close);

	int get_num_candles() const;
	int get_max_candles() const;

	const double * get_open_prices() const;
	const double * get_high_prices() const;
	const double * get_low_prices() const;
	const double * get_close_prices() const;

	/**
	* Checks if the next file should be opened, and if so, opens it, and returns true.
	* If the next file should not be opened (ie. we're past the end_date), this function returns false.
	*/
	bool next_year();

	void parse();
};
